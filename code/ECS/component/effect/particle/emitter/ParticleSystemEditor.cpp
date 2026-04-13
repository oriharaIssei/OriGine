#ifdef _DEBUG

#include "ParticleSystemEditor.h"
#include "EmitterEditor.h"
#include "ParticleSystem.h"

// engine
#include "editor/EditorController.h"
#include "Engine.h"
#define RESOURCE_DIRECTORY
#include "directX12/DxDevice.h"
#include "scene/Scene.h"
// asset
#include "asset/TextureAsset.h"
// component
#include "component/material/Material.h"
// module
#include "asset/AssetSystem.h"
#include "myFileSystem/MyFileSystem.h"
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"
// assets
#include "../Particle.h"
#include "EmitterShape.h"

using namespace OriGine;

void ParticleSystemEditor::Draw(ParticleSystem& _emitter, Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) {
    if (EmitterEditor::Draw(_emitter.emitter_, _parentLabel, _scene)) {
        _emitter.CreateResource();
    }

    ImGui::Spacing();

    {
        auto& materials            = _scene->GetComponents<Material>(_entity);
        int32_t entityMaterialSize = static_cast<int32_t>(materials.size());

        std::string label = "MaterialIndex##" + _parentLabel;
        InputGuiCommand(label, _emitter.materialIndex_);
        _emitter.materialIndex_ = std::clamp(_emitter.materialIndex_, -1, entityMaterialSize - 1);
        if (_emitter.materialIndex_ >= 0) {
            label = "Material##" + _parentLabel;
            if (ImGui::TreeNode(label.c_str())) {
                label = "Material" + _parentLabel;
                materials[_emitter.materialIndex_].Edit(_scene, _entity, label);
                ImGui::TreePop();
            }
        }

        ImGui::Text("Texture : %s", _emitter.textureFileName_.c_str());
        ImGui::SameLine();

        label = "Change Texture##" + _parentLabel;
        if (ImGui::Button(label.c_str())) {
            std::string directory, filename;
            if (MyFileSystem::SelectFileDialog(kApplicationResourceDirectory, directory, filename, {"png"})) {
                std::string filePath = kApplicationResourceDirectory + "/" + directory + "/" + filename;
                auto commandCombo    = std::make_unique<CommandCombo>();
                commandCombo->AddCommand(
                    std::make_shared<SetterCommand<std::string>>(&_emitter.textureFileName_, filePath));
                commandCombo->AddCommand(
                    std::make_shared<SetterCommand<size_t>>(&_emitter.textureIndex_, AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(filePath)));
                OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
            }
        }
    }

    //======================== Emitter の 編集 ========================//
    ImGui::Spacing();
    ImGui::SeparatorText("Emitter");
    ImGui::Spacing();

    EditEmitter(_emitter, _parentLabel);

    //======================== Particle の 編集 ========================//
    ImGui::Spacing();
    ImGui::SeparatorText("Particle");
    ImGui::Spacing();

    EditParticle(_emitter, _parentLabel);

    ImGui::Spacing();

    /*
D3D12 ERROR: GPU-BASED VALIDATION: Draw, SRV resource dimensions differs from that expected by shader: SRV Dimension Expected: D3D12_SRV_DIMENSION_TEXTURE2D, SRV Dimension In Descriptor: D3D12_SRV_DIMENSION_BUFFER, Descriptor Heap Index To DescriptorTableStart: [0], Descriptor Heap Index FromTableStart: [0], Index of Descriptor Range: 0, Shader Stage: PIXEL, Root Parameter Index: [1], Draw Index: [0], Shader Code: <couldn't find file location in debug info>, Asm Instruction Range: [0x1-0xffffffff], Asm Operand Index: [0], Command List: 0x000001BE7283B870:'main', Command List Type: D3D12_COMMAND_LIST_TYPE_DIRECT, SRV/UAV/CBV Descriptor Heap: 0x000001BE728C7560:'Unnamed ID3D12DescriptorHeap Object', Sampler Descriptor Heap: <not set>, Pipeline State: 0x000001BE7AE16090:'Unnamed ID3D12PipelineState Object',  [ EXECUTION ERROR #940: GPU_BASED_VALIDATION_SRV_RESOURCE_DIMENSION_MISMATCH]
D3D12: **BREAK** enabled for the previous message, which was: [ ERROR EXECUTION #940: GPU_BASED_VALIDATION_SRV_RESOURCE_DIMENSION_MISMATCH ]
0x00007FFDF51B73FA (KernelBase.dll) で例外がスローされました (OriGineApp.exe 内): 0x0000087A (パラメーター: 0x0000000000000001, 0x000000BA14AFD728, 0x000001BE7A10EED0)
ハンドルされない例外が 0x00007FFDF51B73FA (KernelBase.dll) で発生しました(OriGineApp.exe 内): 0x0000087A (パラメーター: 0x0000000000000001, 0x000000BA14AFD728, 0x000001BE7A10EED0)。
 */
    if (ImGui::Button("Calculate Particle MaxSize")) {
        _emitter.CalculateMaxSize();
        _emitter.pendingResize_ = true;
    }
}

void ParticleSystemEditor::EditEmitter(ParticleSystem& _emitter, const std::string& _parentLabel) {
    std::string label = "BlendMode##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), kBlendModeStr[int(_emitter.blendMode_)].c_str())) {
        for (int32_t i = 0; i < kBlendNum; i++) {
            bool isSelected = (_emitter.blendMode_ == BlendMode(i));
            if (ImGui::Selectable(kBlendModeStr[i].c_str(), isSelected)) {
                auto command = std::make_unique<SetterCommand<BlendMode>>(&_emitter.blendMode_, (BlendMode)i);
                OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                break;
            }
        }
        ImGui::EndCombo();
    }

    label = "Particle Is BillBoard##" + _parentLabel;
    CheckBoxCommand(label, _emitter.particleIsBillBoard_);
}

void ParticleSystemEditor::EditParticle(ParticleSystem& _emitter, const std::string& _parentLabel) {
    int32_t newFlag               = _emitter.updateSettings_;
    auto commandComboByChangeFlag = std::make_unique<CommandCombo>();

    ImGui::Text("ParticleLifeTime");
    std::string label = "##ParticleLifeTime" + _parentLabel;
    DragGuiCommand<float>(label, _emitter.particleLifeTime_, 0.1f, 0.f);

    ImGui::Separator();

    // Color
    EditColor(_emitter, _parentLabel, newFlag);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Velocity
    EditVelocity(_emitter, _parentLabel, newFlag);

    // Scale
    EditScale(_emitter, _parentLabel, newFlag);

    // Rotate
    EditRotate(_emitter, _parentLabel, newFlag);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // UV
    EditUV(_emitter, _parentLabel, newFlag);

    if (newFlag != _emitter.updateSettings_) {
        auto command = std::make_unique<SetterCommand<int32_t>>(&_emitter.updateSettings_, newFlag);
        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
        OriGine::EditorController::GetInstance()->PushCommand(std::move(commandComboByChangeFlag));
    }
}

void ParticleSystemEditor::EditColor(ParticleSystem& _emitter, const std::string& _parentLabel, int32_t& _newFlag) {
    std::string label        = "ColorInterpolationType##" + _parentLabel;
    int newInterpolationType = static_cast<int>(_emitter.colorInterpolationType_);
    if (ImGui::Combo(label.c_str(), &newInterpolationType, "LINEAR\0STEP\0\0", static_cast<int>(InterpolationType::COUNT))) {
        auto command = std::make_unique<SetterCommand<InterpolationType>>(&_emitter.colorInterpolationType_, (InterpolationType)newInterpolationType);
        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
    }

    label = "Particle Color##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        ColorEditGuiCommand<4>(
            label,
            _emitter.particleColor_,
            0,
            [&_emitter](Vec<4, float>* _newColor) {
                if (!_emitter.particleKeyFrames_->colorCurve.empty()) {
                    _emitter.particleKeyFrames_->colorCurve[0].value = *_newColor;
                }
            });

        bool updatePerLifeTime = (_newFlag & static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime)) != 0;
        label                  = "UpdateColorPerLifeTime##" + _parentLabel;

        if (ImGui::Checkbox(label.c_str(), &updatePerLifeTime)) {
            if (updatePerLifeTime) {
                _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime));
            } else {
                if (_emitter.particleKeyFrames_->colorCurve.empty()) {
                    _emitter.particleKeyFrames_->colorCurve.emplace_back(0.f, _emitter.particleColor_);
                } else {
                    _emitter.particleKeyFrames_->colorCurve[0].value = _emitter.particleColor_;
                }
                _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime));
            }
        }

        if (updatePerLifeTime) {
            label = "ColorLine##" + _parentLabel;
            ImGui::EditColorKeyFrame(label, _emitter.particleKeyFrames_->colorCurve, _emitter.particleLifeTime_);
        }

        ImGui::TreePop();
    }
}

void ParticleSystemEditor::EditVelocity(ParticleSystem& _emitter, const std::string& _parentLabel, int32_t& _newFlag) {
    std::string label        = "TransformInterpolationType##" + _parentLabel;
    int newInterpolationType = static_cast<int>(_emitter.transformInterpolationType_);
    if (ImGui::Combo(label.c_str(), &newInterpolationType, "LINEAR\0STEP\0\0", static_cast<int>(InterpolationType::COUNT))) {
        auto command = std::make_unique<SetterCommand<InterpolationType>>(&_emitter.transformInterpolationType_, (InterpolationType)newInterpolationType);
        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
    }

    label = "Particle Velocity##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        ImGui::Text("Min");
        label = "##ParticleVelocityMin" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            _emitter.startParticleVelocityMin_,
            0.1f,
            {}, {},
            "%.3f",
            [&_emitter](Vec<3, float>* _newVec) {
                *_newVec = MinElement(*_newVec, _emitter.startParticleVelocityMax_);
                if (!_emitter.particleKeyFrames_->velocityCurve.empty()) {
                    _emitter.particleKeyFrames_->velocityCurve[0].value = *_newVec;
                }
            });

        _emitter.startParticleVelocityMin_ = MinElement(_emitter.startParticleVelocityMin_, _emitter.startParticleVelocityMax_);

        ImGui::Text("Max");
        label = "##ParticleVelocityMax" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            _emitter.startParticleVelocityMax_,
            0.1f,
            {}, {},
            "%.3f",
            [&_emitter](Vec<3, float>* _newVec) {
                *_newVec = MaxElement(_emitter.startParticleVelocityMin_, *(_newVec));
            });
        _emitter.startParticleVelocityMax_ = MaxElement(_emitter.startParticleVelocityMin_, _emitter.startParticleVelocityMax_);

        int randomOrPerLifeTime    = (_newFlag & static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime)) ? 2 : ((_newFlag & static_cast<int32_t>(ParticleUpdateType::VelocityRandom)) ? 1 : 0);
        int preRandomOrPerLifeTime = randomOrPerLifeTime;

        label = "Update Velocity None##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 0);
        label = "Update Velocity Random##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 1);
        label = "Update Velocity PerLifeTime##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 2);

        bool isUsingVelocityRotate = (_newFlag & static_cast<int32_t>(ParticleUpdateType::VelocityRotateForward)) != 0;
        label                      = "VelocityRotateForward##" + _parentLabel;
        if (ImGui::Checkbox(label.c_str(), &isUsingVelocityRotate)) {
            if (isUsingVelocityRotate) {
                _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::VelocityRotateForward));
            } else {
                _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::VelocityRotateForward));
            }
        }

        bool isUsingGravity = (_newFlag & static_cast<int32_t>(ParticleUpdateType::UsingGravity));
        label               = "UsingGravity##" + _parentLabel;
        if (ImGui::Checkbox(label.c_str(), &isUsingGravity)) {
            _newFlag = isUsingGravity
                           ? _newFlag | static_cast<int32_t>(ParticleUpdateType::UsingGravity)
                           : _newFlag & ~static_cast<int32_t>(ParticleUpdateType::UsingGravity);
        }

        if (isUsingGravity) {
            _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::UsingGravity));

            label = "Min Mass##" + _parentLabel;
            DragGuiCommand<float>(label, _emitter.randMass_[X], 0.1f, {}, {}, "%.3f", [&_emitter](float* _newVal) {
                *_newVal = (std::min)(*_newVal, _emitter.randMass_[Y]);
            });
            label = "Max Mass##" + _parentLabel;
            DragGuiCommand<float>(label, _emitter.randMass_[Y], 0.1f, {}, {}, "%.3f", [&_emitter](float* _newVal) {
                *_newVal = (std::max)(_emitter.randMass_[X], *_newVal);
            });
            _emitter.randMass_[X] = (std::min)(_emitter.randMass_[X], _emitter.randMass_[Y]);
            _emitter.randMass_[Y] = (std::max)(_emitter.randMass_[X], _emitter.randMass_[Y]);
        }

        if (randomOrPerLifeTime == 2) {
            _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime));
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::VelocityRandom));

            if (_emitter.particleKeyFrames_->velocityCurve.empty()) {
                _emitter.particleKeyFrames_->velocityCurve.emplace_back(0.f, _emitter.startParticleVelocityMin_);
            } else {
                _emitter.particleKeyFrames_->velocityCurve[0].value = _emitter.startParticleVelocityMin_;
            }
            label = "VelocityLine##" + _parentLabel;
            ImGui::EditKeyFrame(label.c_str(), _emitter.particleKeyFrames_->velocityCurve, _emitter.particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            ImGui::Text("UpdateMin");
            label = "##UpdateParticleVelocityMin" + _parentLabel;
            DragGuiVectorCommand<3, float>(
                label,
                _emitter.updateParticleVelocityMin_,
                0.1f,
                {}, {},
                "%.3f",
                [&_emitter](Vec<3, float>* _newMin) {
                    *_newMin = MinElement(*_newMin, _emitter.updateParticleVelocityMax_);
                });

            _emitter.updateParticleVelocityMin_ = MinElement(_emitter.updateParticleVelocityMin_, _emitter.updateParticleVelocityMax_);
            ImGui::Text("UpdateMax");
            label = "##UpdateParticleVelocityMax" + _parentLabel;
            DragGuiVectorCommand<3, float>(
                label,
                _emitter.updateParticleVelocityMax_,
                0.1f, {}, {},
                "%.3f",
                [&_emitter](Vec<3, float>* _newMax) {
                    *_newMax = MaxElement(_emitter.updateParticleVelocityMin_, *(_newMax));
                });
            _emitter.updateParticleVelocityMax_ = MaxElement(_emitter.updateParticleVelocityMin_, _emitter.updateParticleVelocityMax_);

            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime));
            _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::VelocityRandom));
        } else if (preRandomOrPerLifeTime != 0 && randomOrPerLifeTime == 0) {
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime));
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::VelocityRandom));
        }
        ImGui::TreePop();
    }
}

void ParticleSystemEditor::EditScale(ParticleSystem& _emitter, const std::string& _parentLabel, int32_t& _newFlag) {
    std::string label = "Particle Scale##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        ImGui::Text("Min");
        label = "##ParticleScaleMin" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            _emitter.startParticleScaleMin_,
            0.1f,
            {}, {},
            "%.3f",
            [&_emitter](Vec<3, float>* _newMin) {
                *_newMin = MinElement(*_newMin, _emitter.startParticleScaleMax_);
                if (!_emitter.particleKeyFrames_->scaleCurve.empty()) {
                    _emitter.particleKeyFrames_->scaleCurve[0].value = *_newMin;
                }
            });
        _emitter.startParticleScaleMin_ = MinElement(_emitter.startParticleScaleMin_, _emitter.startParticleScaleMax_);

        ImGui::Text("Max");
        label = "##ParticleScaleMax" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            _emitter.startParticleScaleMax_,
            0.1f,
            {}, {},
            "%.3f",
            [&_emitter](Vec<3, float>* _newMax) {
                *_newMax = MaxElement(_emitter.startParticleScaleMin_, *(_newMax));
            });

        _emitter.startParticleScaleMax_ = MaxElement(_emitter.startParticleScaleMin_, _emitter.startParticleScaleMax_);

        int randomOrPerLifeTime    = (_newFlag & static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime)) ? 2 : ((_newFlag & static_cast<int32_t>(ParticleUpdateType::ScaleRandom)) ? 1 : 0);
        int preRandomOrPerLifeTime = randomOrPerLifeTime;

        bool uniformScaleRandom = (_newFlag & static_cast<int32_t>(ParticleUpdateType::UniformScaleRandom)) != 0;
        label                   = "UniformScaleRandom##" + _parentLabel;
        if (ImGui::Checkbox(label.c_str(), &uniformScaleRandom)) {
            if (uniformScaleRandom) {
                _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::UniformScaleRandom));
            } else {
                _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::UniformScaleRandom));
            }
        }

        label = "Update Scale None##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 0);
        label = "Update Scale Random##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 1);
        label = "Update Scale PerLifeTime##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 2);

        if (randomOrPerLifeTime == 2) {
            _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime));
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::ScaleRandom));

            if (_emitter.particleKeyFrames_->scaleCurve.empty()) {
                _emitter.particleKeyFrames_->scaleCurve.emplace_back(0.f, _emitter.startParticleVelocityMin_);
            } else {
                _emitter.particleKeyFrames_->scaleCurve[0].value = _emitter.startParticleVelocityMin_;
            }

            label = "ScaleLine##" + _parentLabel;
            ImGui::EditKeyFrame(label, _emitter.particleKeyFrames_->scaleCurve, _emitter.particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            ImGui::Text("UpdateMin");
            label = "##UpdateParticleScaleMin" + _parentLabel;
            DragGuiVectorCommand<3, float>(
                label,
                _emitter.updateParticleScaleMin_,
                0.1f,
                {}, {},
                "%.3f",
                [&_emitter](Vec<3, float>* _newMin) {
                    *_newMin = MinElement(*_newMin, _emitter.updateParticleScaleMax_);
                });
            _emitter.updateParticleScaleMin_ = MinElement(_emitter.updateParticleScaleMin_, _emitter.updateParticleScaleMax_);

            ImGui::Text("UpdateMax");
            label = "##UpdateParticleScaleMax" + _parentLabel;
            DragGuiVectorCommand<3, float>(
                label,
                _emitter.updateParticleScaleMax_,
                0.1f,
                {}, {},
                "%.3f",
                [&_emitter](Vec<3, float>* _newMax) {
                    *_newMax = MaxElement(_emitter.updateParticleScaleMin_, *(_newMax));
                });
            _emitter.updateParticleScaleMax_ = MaxElement(_emitter.updateParticleScaleMin_, _emitter.updateParticleScaleMax_);

            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime));
            _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::ScaleRandom));
        } else if (preRandomOrPerLifeTime != 0 && randomOrPerLifeTime == 0) {
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime));
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::ScaleRandom));
        }
        ImGui::TreePop();
    }
}

void ParticleSystemEditor::EditRotate(ParticleSystem& _emitter, const std::string& _parentLabel, int32_t& _newFlag) {
    if (ImGui::TreeNode("Particle Rotate")) {
        std::string label = "Min";
        label             = "##ParticleRotateMin" + _parentLabel;
        ImGui::Text("Min");
        DragGuiVectorCommand<3, float>(
            label,
            _emitter.startParticleRotateMin_,
            0.1f,
            {}, {},
            "%.3f",
            [&_emitter](Vec<3, float>* _newMin) {
                *_newMin = MinElement(*_newMin, _emitter.startParticleRotateMax_);
                if (!_emitter.particleKeyFrames_->rotateCurve.empty()) {
                    _emitter.particleKeyFrames_->rotateCurve[0].value = *_newMin;
                }
            });
        _emitter.startParticleRotateMin_ = MinElement(_emitter.startParticleRotateMin_, _emitter.startParticleRotateMax_);

        ImGui::Text("Max");
        label = "##ParticleRotateMax" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            _emitter.startParticleRotateMax_,
            0.1f,
            {}, {},
            "%.3f",
            [&_emitter](Vec<3, float>* _newMax) {
                *_newMax = MaxElement(_emitter.startParticleRotateMin_, *(_newMax));
            });
        _emitter.startParticleRotateMax_ = MaxElement(_emitter.startParticleRotateMin_, _emitter.startParticleRotateMax_);

        int randomOrPerLifeTime    = (_newFlag & static_cast<int32_t>(ParticleUpdateType::RotateForward))
                                         ? 3
                                     : (_newFlag & static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime))
                                         ? 2
                                         : ((_newFlag & static_cast<int32_t>(ParticleUpdateType::RotateRandom))
                                                   ? 1
                                                   : 0);
        int preRandomOrPerLifeTime = randomOrPerLifeTime;

        label = "Update Rotate None##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 0);
        label = "Update Rotate Random##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 1);
        label = "Update Rotate PerLifeTime##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 2);
        label = "Update Rotate Forward##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 3);

        if (randomOrPerLifeTime == 3) {
            _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::RotateForward));
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateRandom));
        } else if (randomOrPerLifeTime == 2) {
            _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateForward));
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateRandom));

            if (_emitter.particleKeyFrames_->rotateCurve.empty()) {
                _emitter.particleKeyFrames_->rotateCurve.emplace_back(0.f, _emitter.startParticleRotateMin_);
            } else {
                _emitter.particleKeyFrames_->rotateCurve[0].value = _emitter.startParticleRotateMin_;
            }

            label = "RotateLine##" + _parentLabel;
            ImGui::EditKeyFrame(label, _emitter.particleKeyFrames_->rotateCurve, _emitter.particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            ImGui::Text("UpdateMin");
            label = "##UpdateParticleRotateMin" + _parentLabel;
            DragGuiVectorCommand<3, float>(
                label,
                _emitter.updateParticleRotateMin_,
                0.1f,
                {}, {},
                "%.3f",
                [&_emitter](Vec<3, float>* _newMin) {
                    *_newMin = MinElement(*_newMin, _emitter.updateParticleRotateMax_);
                });
            _emitter.updateParticleRotateMin_ = MinElement(_emitter.updateParticleRotateMin_, _emitter.updateParticleRotateMax_);

            ImGui::Text("UpdateMax");
            label = "##UpdateParticleRotateMax" + _parentLabel;
            DragGuiVectorCommand<3, float>(
                label,
                _emitter.updateParticleRotateMax_,
                0.1f,
                {}, {},
                "%.3f",
                [&_emitter](Vec<3, float>* _newMax) {
                    *_newMax = MaxElement(_emitter.updateParticleRotateMin_, *(_newMax));
                });
            _emitter.updateParticleRotateMax_ = MaxElement(_emitter.updateParticleRotateMin_, _emitter.updateParticleRotateMax_);

            _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::RotateRandom));
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateForward));
        } else if (preRandomOrPerLifeTime != 0 && randomOrPerLifeTime == 0) {
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateForward));
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateRandom));
        }
        ImGui::TreePop();
    }
}

void ParticleSystemEditor::EditUV(ParticleSystem& _emitter, const std::string& _parentLabel, int32_t& _newFlag) {
    std::string label = "UvCurveGenerator Form TextureAnimation##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        label = "TileSize##" + _parentLabel;
        DragGuiVectorCommand(label, _emitter.tileSize_, 0.1f);
        label = "TextureSize##" + _parentLabel;
        DragGuiVectorCommand(label, _emitter.textureSize_, 0.1f);
        label = "TilePerTime##" + _parentLabel;
        DragGuiCommand(label, _emitter.tilePerTime_);
        label = "StartAnimationTime##" + _parentLabel;
        DragGuiCommand<float>(label, _emitter.startAnimationTime_, 0.1f, 0.f);
        label = "AnimationTimeLength##" + _parentLabel;
        DragGuiCommand<float>(label, _emitter.animationTimeLength_, 0.1f, 0.f);

        label = "Generate Curve##" + _parentLabel;
        if (ImGui::Button(label.c_str())) {
            if (_emitter.particleKeyFrames_) {
                _emitter.particleLifeTime_ = _emitter.animationTimeLength_;

                auto commandCombo = std::make_unique<CommandCombo>();

                commandCombo->AddCommand(
                    std::make_shared<ClearCommand<AnimationCurve<Vec3f>>>(&_emitter.particleKeyFrames_->uvScaleCurve));
                commandCombo->AddCommand(
                    std::make_shared<ClearCommand<AnimationCurve<Vec3f>>>(&_emitter.particleKeyFrames_->uvTranslateCurve));

                _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));
                commandCombo->AddCommand(
                    std::make_shared<SetterCommand<Vector3f>>(&_emitter.particleUvScale_, Vector3f(_emitter.tileSize_[X] / _emitter.textureSize_[X], _emitter.tileSize_[Y] / _emitter.textureSize_[Y], 0.f)));

                _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));
                commandCombo->AddCommand(
                    std::make_shared<SetterCommand<InterpolationType>>(&_emitter.uvInterpolationType_, InterpolationType::STEP));

                int32_t maxTilesX = int32_t(_emitter.textureSize_[X] / _emitter.tileSize_[X]);
                int32_t maxTilesY = int32_t(_emitter.textureSize_[Y] / _emitter.tileSize_[Y]);
                int32_t maxTiles  = maxTilesX * maxTilesY;
                float maxTime     = maxTiles * _emitter.tilePerTime_;

                commandCombo->AddCommand(
                    std::make_shared<SetterCommand<float>>(&_emitter.startAnimationTime_, fmod(_emitter.startAnimationTime_, maxTime)));

                int32_t startTileIndex = int32_t(_emitter.startAnimationTime_ / _emitter.tilePerTime_);
                float col              = float(startTileIndex % maxTilesX);
                float row              = float(startTileIndex / maxTilesX);

                float x = col * (_emitter.tileSize_[X] / _emitter.textureSize_[X]);
                float y = row * (_emitter.tileSize_[Y] / _emitter.textureSize_[Y]);

                commandCombo->AddCommand(
                    std::make_shared<SetterCommand<Vector3f>>(&_emitter.particleUvTranslate_, Vector3f(x, y, 0.f)));

                AnimationCurve<Vec3f> uvTranslateCurve;
                int32_t tileNum = int32_t(_emitter.animationTimeLength_ / _emitter.tilePerTime_);
                for (int32_t i = 0; i < tileNum; i++) {
                    float time = (_emitter.tilePerTime_ * i);

                    col += 1.f;
                    if (col >= maxTilesX) {
                        col = 0.f;
                        row += 1.f;
                    }

                    x = col * (_emitter.tileSize_[X] / _emitter.textureSize_[X]);
                    y = row * (_emitter.tileSize_[Y] / _emitter.textureSize_[Y]);
                    uvTranslateCurve.emplace_back(time, Vector3f(x, y, 0.f));
                }
                commandCombo->AddCommand(
                    std::make_shared<SetterCommand<AnimationCurve<Vec3f>>>(&_emitter.particleKeyFrames_->uvTranslateCurve, uvTranslateCurve));

                OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
            }
        }

        ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    int newInterpolationType = static_cast<int>(_emitter.uvInterpolationType_);
    label                    = "UvInterpolationType##" + _parentLabel;
    if (ImGui::Combo(label.c_str(), &newInterpolationType, "LINEAR\0STEP\0\0", static_cast<int>(InterpolationType::COUNT))) {
        auto command = std::make_unique<SetterCommand<InterpolationType>>(&_emitter.uvInterpolationType_, (InterpolationType)newInterpolationType);
        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // UV Scale
    label = "Particle UV Scale##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        ImGui::Text("UVScale");
        label = "##ParticleUvScale" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            _emitter.particleUvScale_,
            0.1f,
            {}, {},
            "%.3f",
            [&_emitter](Vec<3, float>* _newScale) {
                if (!_emitter.particleKeyFrames_->uvScaleCurve.empty()) {
                    _emitter.particleKeyFrames_->uvScaleCurve[0].value = *_newScale;
                }
            });

        bool updatePerLifeTime    = (_newFlag & static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;
        label                     = "Update UvScale PerLifeTime##" + _parentLabel;
        ImGui::Checkbox(label.c_str(), &updatePerLifeTime);
        if (updatePerLifeTime) {
            _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));

            if (_emitter.particleKeyFrames_->uvScaleCurve.empty()) {
                _emitter.particleKeyFrames_->uvScaleCurve.emplace_back(0.f, _emitter.particleUvScale_);
            } else {
                _emitter.particleKeyFrames_->uvScaleCurve[0].value = _emitter.particleUvScale_;
            }
            ImGui::EditKeyFrame("UvScaleLine", _emitter.particleKeyFrames_->uvScaleCurve, _emitter.particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));
        }
        ImGui::TreePop();
    }

    // UV Rotate
    if (ImGui::TreeNode("Particle UV Rotate")) {
        ImGui::Text("UVRotate");
        label = "##ParticleUvRotate" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            _emitter.particleUvRotate_,
            0.1f,
            {}, {},
            "%.3f",
            [&_emitter](Vec<3, float>* _newUvRotate) {
                if (!_emitter.particleKeyFrames_->uvRotateCurve.empty()) {
                    _emitter.particleKeyFrames_->uvRotateCurve[0].value = *_newUvRotate;
                }
            });

        bool updatePerLifeTime    = (_newFlag & static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;

        label = "Update UvRotate PerLifeTime##" + _parentLabel;
        ImGui::Checkbox(label.c_str(), &updatePerLifeTime);
        if (updatePerLifeTime) {
            _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime));

            if (_emitter.particleKeyFrames_->uvRotateCurve.empty()) {
                _emitter.particleKeyFrames_->uvRotateCurve.emplace_back(0.f, _emitter.particleUvRotate_);
            } else {
                _emitter.particleKeyFrames_->uvRotateCurve[0].value = _emitter.particleUvRotate_;
            }

            label = "UvRotateLine##" + _parentLabel;
            ImGui::EditKeyFrame(label, _emitter.particleKeyFrames_->uvRotateCurve, _emitter.particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime));
        }
        ImGui::TreePop();
    }

    // UV Translate
    label = "Particle UV Translate##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        ImGui::Text("UVTranslate");
        label = "##ParticleUvTranslate" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            _emitter.particleUvTranslate_,
            0.1f,
            {}, {},
            "%.3f",
            [&_emitter](Vec<3, float>* _newUVTranslate) {
                if (!_emitter.particleKeyFrames_->uvTranslateCurve.empty()) {
                    _emitter.particleKeyFrames_->uvTranslateCurve[0].value = *_newUVTranslate;
                }
            });

        bool updatePerLifeTime    = (_newFlag & static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;
        label                     = "Update UvTranslate PerLifeTime##" + _parentLabel;
        ImGui::Checkbox(label.c_str(), &updatePerLifeTime);
        if (updatePerLifeTime) {
            _newFlag = (_newFlag | static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));

            if (_emitter.particleKeyFrames_->uvTranslateCurve.empty()) {
                _emitter.particleKeyFrames_->uvTranslateCurve.emplace_back(0.f, _emitter.particleUvTranslate_);
            } else {
                _emitter.particleKeyFrames_->uvTranslateCurve[0].value = _emitter.particleUvTranslate_;
            }

            label = "UvTranslateLine##" + _parentLabel;
            ImGui::EditKeyFrame(label, _emitter.particleKeyFrames_->uvTranslateCurve, _emitter.particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            _newFlag = (_newFlag & ~static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));
        }
        ImGui::TreePop();
    }
}

#endif // _DEBUG
