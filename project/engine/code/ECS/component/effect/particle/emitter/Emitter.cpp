#include "Emitter.h"

// stl
// container
#include <array>

/// engine
#include "Engine.h"
#define RESOURCE_DIRECTORY
#include "directX12/DxDevice.h"
#include "EngineInclude.h"
#include "scene/Scene.h"
// component
#include "component/material/Material.h"
#include "component/renderer/primitive/shape/Plane.h"
// module
#include "camera/CameraManager.h"
#include "directX12/ShaderManager.h"
#include "globalVariables/GlobalVariables.h"
#include "model/ModelManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "myRandom/MyRandom.h"
#include "texture/TextureManager.h"
// asSets
#include "EmitterShape.h"
#include "model/Model.h"

#include "logger/Logger.h"

// math
#include "math/Matrix4x4.h"
#include <cmath>

#ifdef _DEBUG
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"
#endif // _DEBUG

Emitter::Emitter() : IComponent(), currentCoolTime_(0.f), leftActiveTime_(0.f) {
    isActive_       = false;
    leftActiveTime_ = 0.0f;
}

Emitter::~Emitter() {}

void Emitter::Initialize(Entity* /*_entity*/) {
    { // Initialize DrawingData Size
        CalculateMaxSize();
        particles_.reserve(particleMaxSize_);
    }

    { // Initialize Active State
        leftActiveTime_  = activeTime_;
        currentCoolTime_ = 0.f;
    }

    // resource
    if (isActive_) {
        CreateResource();
    }

    if (!textureFileName_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureFileName_);
    }

    if (!particleKeyFrames_) {
        particleKeyFrames_ = std::make_shared<ParticleKeyFrames>();
    }
}

void Emitter::Finalize() {
    if (!particles_.empty()) {
        particles_.clear();
    }
    structuredTransform_.Finalize();
}

void Emitter::Update(float deltaTime) {
    { // Update Active
        if (!isActive_) {
            return;
        }

        // Loop するなら スキップ
        if (!isLoop_) {
            leftActiveTime_ -= deltaTime;
            // leftActiveTime が 0 以下で Particle が 全て消えたら
            if (leftActiveTime_ <= 0.0f && particles_.empty()) {
                isActive_ = false;
                return;
            }
        }
    }

    UpdateParticle(deltaTime);
}

void Emitter::UpdateParticle(float _deltaTime) {
    { // Particles Update
        for (auto& particle : particles_) {
            particle->Update(_deltaTime);
        }
        // IsAliveでないもの は 消す
        std::erase_if(particles_, [](std::shared_ptr<Particle>& particle) {
            return !particle->GetIsAlive();
        });
    }

    { // Update Spawn
        currentCoolTime_ -= _deltaTime;
        // leftActiveTimeが 0以上のときだけ
        if (leftActiveTime_ > 0.f) {
            if (currentCoolTime_ <= 0.0f) {
                currentCoolTime_ = spawnCoolTime_ / static_cast<float>(spawnParticleVal_);
                SpawnParticle((std::max)(1, static_cast<int32_t>(_deltaTime / currentCoolTime_)));
            }
        }
    }

    { // push Drawing InstanceData
        structuredTransform_.openData_.clear();
        for (auto& particle : particles_) {
            structuredTransform_.openData_.push_back(particle->GetTransform());
        }
    }
}

void Emitter::CreateResource() {
    if (!mesh_.GetVertexBuffer().GetResource()) {
        Primitive::Plane planeGenerator;
        planeGenerator.CreateMesh(&mesh_);
    }
    if (!structuredTransform_.GetResource().GetResource().Get()) {
        structuredTransform_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_, particleMaxSize_);
    }
    if (!materialBuffer_.GetResource().GetResource().Get()) {
        materialBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    }
}

void Emitter::CalculateMaxSize() {
    // 1秒あたりの生成回数
    float spawnRatePersecond = spawnParticleVal_ / spawnCoolTime_;

    // 最大個数
    particleMaxSize_ = (std::max<uint32_t>)((std::max<uint32_t>)(static_cast<uint32_t>(std::ceil(spawnRatePersecond * particleLifeTime_)), spawnParticleVal_), particleMaxSize_);
}

void Emitter::SpawnParticle(int32_t _spawnVal) {
    // スポーンして良い数
    int32_t canSpawnParticleValue_ = (std::min<int32_t>)(_spawnVal, static_cast<int32_t>(particleMaxSize_ - particles_.size()));

    preWorldOriginPos_ = worldOriginPos_;
    worldOriginPos_    = originPos_;
    if (worldOriginPos_ != preWorldOriginPos_) {
        LOG_DEBUG("Emitter::SpawnParticle: worldOriginPos_ changed");
    }

    bool uniformScaleRandom = (updateSettings_ & int(ParticleUpdateType::UniformScaleRandom)) != 0;

    for (int32_t i = 0; i < canSpawnParticleValue_; i++) {
        Vec3f spawnPos = Lerp(preWorldOriginPos_, worldOriginPos_, float(i) / float(canSpawnParticleValue_));
        spawnPos += emitterSpawnShape_->GetSpawnPos();

        // 割りたてる Transform の 初期化
        structuredTransform_.openData_.push_back({});
        auto& transform = structuredTransform_.openData_.back();

        transform.color = particleColor_;

        MyRandom::Float randX;
        MyRandom::Float randY;
        MyRandom::Float randZ;

        randX.SetRange(startParticleVelocityMin_.v[X], startParticleVelocityMax_.v[X]);
        randY.SetRange(startParticleVelocityMin_.v[Y], startParticleVelocityMax_.v[Y]);
        randZ.SetRange(startParticleVelocityMin_.v[Z], startParticleVelocityMax_.v[Z]);
        Vec3f velocity = {randX.Get(), randY.Get(), randZ.Get()};

        if (uniformScaleRandom) {
            Vec3f scaleBase    = startParticleScaleMin_.normalize();
            float maxScaleRate = startParticleScaleMax_.length();
            float minScaleRate = startParticleScaleMin_.length();
            randX.SetRange(minScaleRate, maxScaleRate);

            transform.scale = scaleBase * randX.Get();
        } else {
            randX.SetRange(startParticleScaleMin_.v[X], startParticleScaleMax_.v[X]);
            randY.SetRange(startParticleScaleMin_.v[Y], startParticleScaleMax_.v[Y]);
            randZ.SetRange(startParticleScaleMin_.v[Z], startParticleScaleMax_.v[Z]);
            transform.scale = {randX.Get(), randY.Get(), randZ.Get()};
        }

        randX.SetRange(startParticleRotateMin_.v[X], startParticleRotateMax_.v[X]);
        randY.SetRange(startParticleRotateMin_.v[Y], startParticleRotateMax_.v[Y]);
        randZ.SetRange(startParticleRotateMin_.v[Z], startParticleRotateMax_.v[Z]);
        transform.rotate    = {randX.Get(), randY.Get(), randZ.Get()};
        transform.translate = spawnPos;

        transform.uvScale     = particleUvScale_;
        transform.uvRotate    = particleUvRotate_;
        transform.uvTranslate = particleUvTranslate_;

        // Particle 初期化
        std::shared_ptr<Particle>& spawnedParticle = particles_.emplace_back<std::shared_ptr<Particle>>(std::make_unique<Particle>());
        spawnedParticle->Initialize(
            transform,
            startParticleVelocityMin_,
            startParticleVelocityMax_,
            startParticleScaleMin_,
            startParticleScaleMax_,
            startParticleRotateMin_,
            startParticleRotateMax_,
            particleLifeTime_,
            Vec3f(transform.translate - originPos_).normalize(),
            velocity,
            transformInterpolationType_,
            colorInterpolationType_,
            uvInterpolationType_);

        if (updateSettings_ & int(ParticleUpdateType::VelocityRandom)) {

            randX.SetRange(updateParticleVelocityMin_.v[X], updateParticleVelocityMax_.v[X]);
            randY.SetRange(updateParticleVelocityMin_.v[Y], updateParticleVelocityMax_.v[Y]);
            randZ.SetRange(updateParticleVelocityMin_.v[Z], updateParticleVelocityMax_.v[Z]);
            spawnedParticle->SetUpdateVelocity(Vec3f(randX.Get(), randY.Get(), randZ.Get()));
        }
        if (updateSettings_ & int(ParticleUpdateType::UsingGravity)) {
            randX.SetRange(randMass_[X], randMass_[Y]);
            spawnedParticle->SetMass(randX.Get());
        }
        if (updateSettings_ & int(ParticleUpdateType::ScaleRandom)) {
            randX.SetRange(updateParticleScaleMin_.v[X], updateParticleScaleMax_.v[X]);
            randY.SetRange(updateParticleScaleMin_.v[Y], updateParticleScaleMax_.v[Y]);
            randZ.SetRange(updateParticleScaleMin_.v[Z], updateParticleScaleMax_.v[Z]);
            spawnedParticle->SetUpdateScale(Vec3f(randX.Get(), randY.Get(), randZ.Get()));
        }
        if (updateSettings_ & int(ParticleUpdateType::RotateRandom)) {
            randX.SetRange(updateParticleRotateMin_.v[X], updateParticleRotateMax_.v[X]);
            randY.SetRange(updateParticleRotateMin_.v[Y], updateParticleRotateMax_.v[Y]);
            randZ.SetRange(updateParticleRotateMin_.v[Z], updateParticleRotateMax_.v[Z]);
            spawnedParticle->SetUpdateRotate(Vec3f(randX.Get(), randY.Get(), randZ.Get()));
        }

        spawnedParticle->SetKeyFrames(updateSettings_, particleKeyFrames_.get());
    }
}

void Emitter::PlayStart() {
    isActive_        = true;
    leftActiveTime_  = activeTime_;
    currentCoolTime_ = 0.f;

    worldOriginPos_    = originPos_;
    preWorldOriginPos_ = worldOriginPos_;

    CreateResource();
}

void Emitter::PlayContinue() {
    isActive_ = true;
}

void Emitter::PlayStop() {
    isActive_       = false;
    leftActiveTime_ = 0.f;
}

void Emitter::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    if (CheckBoxCommand("isActive##" + _parentLabel, isActive_)) {
        CreateResource();
    }
    CheckBoxCommand("isLoop##" + _parentLabel, isLoop_);

    std::string label = "Play##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        leftActiveTime_ = activeTime_;
        CreateResource();
    }
    ImGui::SameLine();
    label = "Stop##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        leftActiveTime_ = -1.f;
    }

    ImGui::Spacing();

    {
        auto materials             = _scene->GetComponents<Material>(_entity);
        int32_t entityMaterialSize = materials != nullptr ? static_cast<int32_t>(materials->size()) : 0;

        InputGuiCommand(label, materialIndex_);
        materialIndex_ = std::clamp(materialIndex_, -1, entityMaterialSize - 1);
        if (materialIndex_ >= 0) {
            label = "Material##" + _parentLabel;
            if (ImGui::TreeNode(label.c_str())) {
                label = "Material" + _parentLabel;
                materials->operator[](materialIndex_).Edit(_scene, _entity, label);
                ImGui::TreePop();
            }
        }

        ImGui::Text("Texture : %s", textureFileName_.c_str());
        ImGui::SameLine();

        label = "Change Texture##" + _parentLabel;
        if (ImGui::Button(label.c_str())) {
            std::string directory, filename;
            if (MyFileSystem::selectFileDialog(kApplicationResourceDirectory, directory, filename, {"png"})) {
                std::string filePath = kApplicationResourceDirectory + "/" + directory + "/" + filename;
                auto commandCombo    = std::make_unique<CommandCombo>();
                commandCombo->AddCommand(
                    std::make_shared<SetterCommand<std::string>>(&textureFileName_, filePath));
                commandCombo->AddCommand(
                    std::make_shared<SetterCommand<int32_t>>(&textureIndex_, TextureManager::LoadTexture(filePath)));
                EditorController::GetInstance()->PushCommand(std::move(commandCombo));
            }
        }
    }

    //======================== ShapeType の 編集 ========================//
    ImGui::Spacing();
    ImGui::SeparatorText("ShapeType");
    ImGui::Spacing();

    EditShapeType(_parentLabel);

    //======================== Emitter の 編集 ========================//
    ImGui::Spacing();
    ImGui::SeparatorText("Emitter");
    ImGui::Spacing();

    EditEmitter(_parentLabel);

    //======================== Particle の 編集 ========================//
    ImGui::Spacing();
    ImGui::SeparatorText("Particle");
    ImGui::Spacing();

    EditParticle(_parentLabel);

    ImGui::Spacing();

    if (ImGui::Button("Calculate Particle MaxSize")) {
        CalculateMaxSize();
        if (structuredTransform_.Capacity() <= particleMaxSize_) {
            structuredTransform_.Resize(Engine::GetInstance()->GetDxDevice()->device_, particleMaxSize_ * 2);
        }
    }

#endif // _DEBUG
}

#ifdef _DEBUG
void Emitter::EditEmitter([[maybe_unused]] const std::string& _parentLabel) {
    //======================== Emitter の 編集 ========================//
    std::string label = "BlendMode##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), blendModeStr[int(blendMode_)].c_str())) {
        for (int32_t i = 0; i < kBlendNum; i++) {
            bool isSelected = (blendMode_ == BlendMode(i)); // 現在選択中かどうか
            if (ImGui::Selectable(blendModeStr[i].c_str(), isSelected)) {
                auto command = std::make_unique<SetterCommand<BlendMode>>(&blendMode_, (BlendMode)i);
                EditorController::GetInstance()->PushCommand(std::move(command));
                break;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Text("EmitterOriginPos");
    label = "##EmitterOriginPos" + _parentLabel;
    DragGuiVectorCommand<3, float>(label, originPos_, 0.01f);

    ImGui::Text("EmitterActiveTime");
    label = "##EmitterActiveTime" + _parentLabel;
    DragGuiCommand(label, activeTime_, 0.1f);
    ImGui::Text("SpawnParticleVal");
    label = "##SpawnParticleVal" + _parentLabel;
    DragGuiCommand(label, spawnParticleVal_, 1, 0, 0, "%d");

    label = "Particle Is BillBoard##" + _parentLabel;
    CheckBoxCommand(label, particleIsBillBoard_);

    ImGui::Text("SpawnCoolTime");
    label = "##SpawnCoolTime" + _parentLabel;
    DragGuiCommand(label, spawnCoolTime_, 0.1f, 0.f);
}

void Emitter::EditShapeType([[maybe_unused]] const std::string& _parentLabel) {
    //======================== ShapeType の 切り替え ========================//
    std::string label = "EmitterShapeType##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), emitterShapeTypeWord_[static_cast<int32_t>(shapeType_)].c_str())) {
        for (int32_t i = 0; i < shapeTypeCount; i++) {
            bool isSelected = (shapeType_ == EmitterShapeType(i)); // 現在選択中かどうか

            if (ImGui::Selectable(emitterShapeTypeWord_[i].c_str(), isSelected)) {

                auto command = std::make_unique<SetterCommand<EmitterShapeType>>(
                    &shapeType_,
                    EmitterShapeType(i),
                    [this](EmitterShapeType* _newType) {
                        switch (*_newType) {
                        case EmitterShapeType::SPHERE:
                            emitterSpawnShape_ = std::make_shared<EmitterSphere>();
                            break;
                        case EmitterShapeType::BOX:
                            emitterSpawnShape_ = std::make_shared<EmitterBox>();
                            break;
                        case EmitterShapeType::CAPSULE:
                            emitterSpawnShape_ = std::make_shared<EmitterCapsule>();
                            break;
                        case EmitterShapeType::CONE:
                            emitterSpawnShape_ = std::make_shared<EmitterCone>();
                            break;
                        default:
                            break;
                        }
                    });

                EditorController::GetInstance()->PushCommand(std::move(command));
            }

            // 現在選択中の項目をハイライトする
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    //======================== ShapeType の 編集 ========================//

    if (emitterSpawnShape_) {
        emitterSpawnShape_->Debug(_parentLabel);
    }
}

void Emitter::EditParticle([[maybe_unused]] const std::string& _parentLabel) {
    int32_t newFlag               = updateSettings_;
    auto commandComboByChangeFlag = std::make_unique<CommandCombo>();
    //======================== Particle の 編集 ========================//

    ImGui::Text("ParticleLifeTime");
    std::string label = "##ParticleLifeTime" + _parentLabel;
    DragGuiCommand<float>(
        label,
        particleLifeTime_,
        0.1f, 0.f);

    ImGui::Separator();

    {
        label                    = "ColorInterpolationType##" + _parentLabel;
        int newInterpolationType = static_cast<int>(colorInterpolationType_);
        if (ImGui::Combo(label.c_str(), &newInterpolationType, "LINEAR\0STEP\0\0", static_cast<int>(InterpolationType::COUNT))) {
            auto command = std::make_unique<SetterCommand<InterpolationType>>(&colorInterpolationType_, (InterpolationType)newInterpolationType);
            EditorController::GetInstance()->PushCommand(std::move(command));
        }
    }

    label = "Particle Color##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        ColorEditGuiCommand<4>(
            label,
            particleColor_,
            0,
            [this](Vec<4, float>* _newColor) {
                if (!particleKeyFrames_->colorCurve.empty()) {
                    particleKeyFrames_->colorCurve[0].value = *_newColor;
                }
            });

        // curveで変更するかどうか
        bool updatePerLifeTime = (newFlag & static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime)) != 0;

        label = "UpdateColorPerLifeTime##" + _parentLabel;

        if (ImGui::Checkbox(label.c_str(), &updatePerLifeTime)) {
            if (updatePerLifeTime) {
                newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime));
            } else {
                if (particleKeyFrames_->colorCurve.empty()) {
                    particleKeyFrames_->colorCurve.emplace_back(0.f, particleColor_);
                } else {
                    particleKeyFrames_->colorCurve[0].value = particleColor_;
                }
                newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime));
            }
        }

        if (updatePerLifeTime) {
            label = "ColorLine##" + _parentLabel;
            ImGui::EditColorKeyFrame(label, particleKeyFrames_->colorCurve, particleLifeTime_);
        }

        ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    {
        int newInterpolationType = static_cast<int>(transformInterpolationType_);
        label                    = "ColorInterpolationType##" + _parentLabel;
        if (ImGui::Combo(label.c_str(), &newInterpolationType, "LINEAR\0STEP\0\0", static_cast<int>(InterpolationType::COUNT))) {
            auto command = std::make_unique<SetterCommand<InterpolationType>>(&transformInterpolationType_, (InterpolationType)newInterpolationType);
            EditorController::GetInstance()->PushCommand(std::move(command));
        }
    }

    label = "Particle Velocity##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        ImGui::Text("Min");
        // 最小速度の設定
        label = "##ParticleVelocityMin" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            startParticleVelocityMin_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newVec) {
                *_newVec = MinElement(*_newVec, startParticleVelocityMax_);
                if (!particleKeyFrames_->velocityCurve.empty()) {
                    particleKeyFrames_->velocityCurve[0].value = *_newVec;
                }
            });

        startParticleVelocityMin_ = MinElement(startParticleVelocityMin_, startParticleVelocityMax_);

        ImGui::Text("Max");
        // 最大速度の設定
        label = "##ParticleVelocityMax" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            startParticleVelocityMax_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newVec) {
                *_newVec = MaxElement(startParticleVelocityMin_, *(_newVec));
            });
        startParticleVelocityMax_ = MaxElement(startParticleVelocityMin_, startParticleVelocityMax_);

        int randomOrPerLifeTime    = (newFlag & static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime)) ? 2 : ((newFlag & static_cast<int32_t>(ParticleUpdateType::VelocityRandom)) ? 1 : 0);
        int preRandomOrPerLifeTime = randomOrPerLifeTime;

        label = "Update Velocity None##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 0);
        label = "Update Velocity Random##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 1);
        label = "Update Velocity PerLifeTime##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 2);

        bool isUsingVelocityRotate = (newFlag & static_cast<int32_t>(ParticleUpdateType::VelocityRotateForward)) != 0;

        label = "VelocityRotateForward##" + _parentLabel;
        if (ImGui::Checkbox(label.c_str(), &isUsingVelocityRotate)) {
            if (isUsingVelocityRotate) {
                newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::VelocityRotateForward));
            } else {
                newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::VelocityRotateForward));
            }
        }

        bool isUsingGravity = (newFlag & static_cast<int32_t>(ParticleUpdateType::UsingGravity));
        label               = "UsingGravity##" + _parentLabel;
        if (ImGui::Checkbox(label.c_str(), &isUsingGravity)) {
            newFlag = isUsingGravity
                          ? newFlag | static_cast<int32_t>(ParticleUpdateType::UsingGravity)
                          : newFlag & ~static_cast<int32_t>(ParticleUpdateType::UsingGravity);
        }

        if (isUsingGravity) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::UsingGravity));

            label = "Min Mass##" + _parentLabel;
            DragGuiCommand<float>(label, randMass_[X], 0.1f, {}, {}, "%.3f", [this](float* _newVal) {
                *_newVal = (std::min)(*_newVal, randMass_[Y]);
            });
            label = "Max Mass##" + _parentLabel;
            DragGuiCommand<float>(label, randMass_[Y], 0.1f, {}, {}, "%.3f", [this](float* _newVal) {
                *_newVal = (std::max)(randMass_[X], *_newVal);
            });
            randMass_[X] = (std::min)(randMass_[X], randMass_[Y]);
            randMass_[Y] = (std::max)(randMass_[X], randMass_[Y]);
        }

        if (randomOrPerLifeTime == 2) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::VelocityRandom));

            if (particleKeyFrames_->velocityCurve.empty()) {
                particleKeyFrames_->velocityCurve.emplace_back(0.f, startParticleVelocityMin_);
            } else {
                particleKeyFrames_->velocityCurve[0].value = startParticleVelocityMin_;
            }
            label = "VelocityLine##" + _parentLabel;
            ImGui::EditKeyFrame(label.c_str(), particleKeyFrames_->velocityCurve, particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            // ランダムな速度を設定
            ImGui::Text("UpdateMin");
            label = "##UpdateParticleVelocityMin" + _parentLabel;
            DragGuiVectorCommand<3, float>(
                label,
                updateParticleVelocityMin_,
                0.1f,
                {}, {},
                "%.3f",
                [this](Vec<3, float>* _newMin) {
                    *_newMin = MinElement(*_newMin, updateParticleVelocityMax_);
                });

            updateParticleVelocityMin_ = MinElement(updateParticleVelocityMin_, updateParticleVelocityMax_);
            ImGui::Text("UpdateMax");
            label = "##UpdateParticleVelocityMax" + _parentLabel;
            DragGuiVectorCommand<3, float>(
                label,
                updateParticleVelocityMax_,
                0.1f, {}, {},
                "%.3f",
                [this](Vec<3, float>* _newMax) {
                    *_newMax = MaxElement(updateParticleVelocityMin_, *(_newMax));
                });
            updateParticleVelocityMax_ = MaxElement(updateParticleVelocityMin_, updateParticleVelocityMax_);

            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime));
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::VelocityRandom));
        } else if (preRandomOrPerLifeTime != 0 && randomOrPerLifeTime == 0) {
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::VelocityRandom));
        }
        ImGui::TreePop();
    }

    label = "Particle Scale##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        ImGui::Text("Min");
        label = "##ParticleScaleMin" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            startParticleScaleMin_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newMin) {
                *_newMin = MinElement(*_newMin, startParticleScaleMax_);
                if (!particleKeyFrames_->scaleCurve.empty()) {
                    particleKeyFrames_->scaleCurve[0].value = *_newMin;
                }
            });
        startParticleScaleMin_ = MinElement(startParticleScaleMin_, startParticleScaleMax_);

        ImGui::Text("Max");
        label = "##ParticleScaleMax" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            startParticleScaleMax_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newMax) {
                *_newMax = MaxElement(startParticleScaleMin_, *(_newMax));
            });

        startParticleScaleMax_ = MaxElement(startParticleScaleMin_, startParticleScaleMax_);

        // curveかrandom か
        int randomOrPerLifeTime    = (newFlag & static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime)) ? 2 : ((newFlag & static_cast<int32_t>(ParticleUpdateType::ScaleRandom)) ? 1 : 0);
        int preRandomOrPerLifeTime = randomOrPerLifeTime;

        bool uniformScaleRandom = (newFlag & static_cast<int32_t>(ParticleUpdateType::UniformScaleRandom)) != 0;
        label                   = "UniformScaleRandom##" + _parentLabel;
        if (ImGui::Checkbox(label.c_str(), &uniformScaleRandom)) {
            if (uniformScaleRandom) {
                newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::UniformScaleRandom));
            } else {
                newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::UniformScaleRandom));
            }
        }

        label = "Update Scale None##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 0);
        label = "Update Scale Random##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 1);
        label = "Update Scale PerLifeTime##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &randomOrPerLifeTime, 2);

        if (randomOrPerLifeTime == 2) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::ScaleRandom));

            if (particleKeyFrames_->scaleCurve.empty()) {
                particleKeyFrames_->scaleCurve.emplace_back(0.f, startParticleVelocityMin_);
            } else {
                particleKeyFrames_->scaleCurve[0].value = startParticleVelocityMin_;
            }

            label = "ScaleLine##" + _parentLabel;
            ImGui::EditKeyFrame(label, particleKeyFrames_->scaleCurve, particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            // ランダムなScaleを設定
            ImGui::Text("UpdateMin");
            label = "##UpdateParticleScaleMin" + _parentLabel;
            DragGuiVectorCommand<3, float>(
                label,
                updateParticleScaleMin_,
                0.1f,
                {}, {},
                "%.3f",
                [this](Vec<3, float>* _newMin) {
                    *_newMin = MinElement(*_newMin, updateParticleScaleMax_);
                });
            updateParticleScaleMin_ = MinElement(updateParticleScaleMin_, updateParticleScaleMax_);

            ImGui::Text("UpdateMax");
            label = "##UpdateParticleScaleMax" + _parentLabel;
            DragGuiVectorCommand<3, float>(
                label,
                updateParticleScaleMax_,
                0.1f,
                {}, {},
                "%.3f",
                [this](Vec<3, float>* _newMax) {
                    *_newMax = MaxElement(updateParticleScaleMin_, *(_newMax));
                });
            updateParticleScaleMax_ = MaxElement(updateParticleScaleMin_, updateParticleScaleMax_);

            // ランダムなスケールを設定
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime));
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::ScaleRandom));
        } else if (preRandomOrPerLifeTime != 0 && randomOrPerLifeTime == 0) {
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::ScaleRandom));
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Particle Rotate")) {
        ImGui::Text("Min");
        label = "##ParticleRotateMin" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            startParticleRotateMin_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newMin) {
                *_newMin = MinElement(*_newMin, startParticleRotateMax_);

                if (!particleKeyFrames_->rotateCurve.empty()) {
                    particleKeyFrames_->rotateCurve[0].value = *_newMin;
                }
            });
        startParticleRotateMin_ = MinElement(startParticleRotateMin_, startParticleRotateMax_);

        ImGui::Text("Max");
        label = "##ParticleRotateMax" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            startParticleRotateMax_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newMax) {
                *_newMax = MaxElement(startParticleRotateMin_, *(_newMax));
            });
        startParticleRotateMax_ = MaxElement(startParticleRotateMin_, startParticleRotateMax_);

        int randomOrPerLifeTime    = (newFlag & static_cast<int32_t>(ParticleUpdateType::RotateForward))
                                         ? 3
                                     : (newFlag & static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime))
                                         ? 2
                                         : ((newFlag & static_cast<int32_t>(ParticleUpdateType::RotateRandom))
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
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::RotateForward));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateRandom));
        } else if (randomOrPerLifeTime == 2) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateForward));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateRandom));

            if (particleKeyFrames_->rotateCurve.empty()) {
                particleKeyFrames_->rotateCurve.emplace_back(0.f, startParticleRotateMin_);
            } else {
                particleKeyFrames_->rotateCurve[0].value = startParticleRotateMin_;
            }

            label = "RotateLine##" + _parentLabel;
            ImGui::EditKeyFrame(label, particleKeyFrames_->rotateCurve, particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            // ランダムな回転を設定
            ImGui::Text("UpdateMin");
            label = "##UpdateParticleRotateMin" + _parentLabel;
            DragGuiVectorCommand<3, float>(
                label,
                updateParticleRotateMin_,
                0.1f,
                {}, {},
                "%.3f",
                [this](Vec<3, float>* _newMin) {
                    *_newMin = MinElement(*_newMin, updateParticleRotateMax_);
                });
            updateParticleRotateMin_ = MinElement(updateParticleRotateMin_, updateParticleRotateMax_);

            ImGui::Text("UpdateMax");
            label = "##UpdateParticleRotateMax" + _parentLabel;
            DragGuiVectorCommand<3, float>(
                label,
                updateParticleRotateMax_,
                0.1f,
                {}, {},
                "%.3f",
                [this](Vec<3, float>* _newMax) {
                    *_newMax = MaxElement(updateParticleRotateMin_, *(_newMax));
                });
            updateParticleRotateMax_ = MaxElement(updateParticleRotateMin_, updateParticleRotateMax_);

            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::RotateRandom));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateForward));
        } else if (preRandomOrPerLifeTime != 0 && randomOrPerLifeTime == 0) {
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateForward));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateRandom));
        }
        ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    label = "UvCurveGenerator Form TextureAnimation##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        label = "TileSize##" + _parentLabel;
        DragGuiVectorCommand(label, tileSize_, 0.1f);
        label = "TextureSize##" + _parentLabel;
        DragGuiVectorCommand(label, textureSize_, 0.1f);
        label = "TilePerTime##" + _parentLabel;
        DragGuiCommand(label, tilePerTime_);
        label = "StartAnimationTime##" + _parentLabel;
        DragGuiCommand<float>(label, startAnimationTime_, 0.1f, 0.f);
        label = "AnimationTimeLength##" + _parentLabel;
        DragGuiCommand<float>(label, animationTimeLength_, 0.1f, 0.f);

        label = "Generate Curve##" + _parentLabel;
        if (ImGui::Button(label.c_str())) {
            if (particleKeyFrames_) {
                particleLifeTime_ = animationTimeLength_;

                // すでにあるカーブを消す UV /scale,translate
                commandComboByChangeFlag->AddCommand(
                    std::make_shared<ClearCommand<AnimationCurve<Vec3f>>>(&particleKeyFrames_->uvScaleCurve));
                commandComboByChangeFlag->AddCommand(
                    std::make_shared<ClearCommand<AnimationCurve<Vec3f>>>(&particleKeyFrames_->uvTranslateCurve));

                // uvScale は Animation しない
                newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));
                commandComboByChangeFlag->AddCommand(
                    std::make_shared<SetterCommand<Vector3f>>(&particleUvScale_, Vector3f(tileSize_[X] / textureSize_[X], tileSize_[Y] / textureSize_[Y], 0.f)));

                // uv Translate は Animation する
                newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));
                commandComboByChangeFlag->AddCommand(
                    std::make_shared<SetterCommand<InterpolationType>>(&uvInterpolationType_, InterpolationType::STEP));

                // 最大タイル数と最大時間を計算
                int32_t maxTilesX = int32_t(textureSize_[X] / tileSize_[X]);
                int32_t maxTilesY = int32_t(textureSize_[Y] / tileSize_[Y]);
                int32_t maxTiles  = maxTilesX * maxTilesY;
                float maxTime     = maxTiles * tilePerTime_;

                // startAnimationTime_ を最大時間内に収める
                commandComboByChangeFlag->AddCommand(
                    std::make_shared<SetterCommand<float>>(&startAnimationTime_, fmod(startAnimationTime_, maxTime)));

                // 初期の col と row を計算
                int32_t startTileIndex = int32_t(startAnimationTime_ / tilePerTime_);
                float col              = float(startTileIndex % maxTilesX);
                float row              = float(startTileIndex / maxTilesX);

                // UV座標を計算
                float x = col * (tileSize_[X] / textureSize_[X]);
                float y = row * (tileSize_[Y] / textureSize_[Y]);

                commandComboByChangeFlag->AddCommand(
                    std::make_shared<SetterCommand<Vector3f>>(&particleUvTranslate_, Vector3f(x, y, 0.f)));

                AnimationCurve<Vec3f> uvTranslateCurve;
                int32_t tileNum = int32_t(animationTimeLength_ / tilePerTime_);
                for (int32_t i = 0; i < tileNum; i++) {
                    float time = (tilePerTime_ * i);

                    col += 1.f;
                    if (col >= maxTilesX) {
                        col = 0.f;
                        row += 1.f;
                    }

                    // UV座標を計算
                    x = col * (tileSize_[X] / textureSize_[X]);
                    y = row * (tileSize_[Y] / textureSize_[Y]);
                    uvTranslateCurve.emplace_back(time, Vector3f(x, y, 0.f));
                }
                commandComboByChangeFlag->AddCommand(
                    std::make_shared<SetterCommand<AnimationCurve<Vec3f>>>(&particleKeyFrames_->uvTranslateCurve, uvTranslateCurve));
            }
        }

        ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    {
        int newInterpolationType = static_cast<int>(uvInterpolationType_);
        label                    = "UvInterpolationType##" + _parentLabel;
        if (ImGui::Combo(label.c_str(), &newInterpolationType, "LINEAR\0STEP\0\0", static_cast<int>(InterpolationType::COUNT))) {
            auto command = std::make_unique<SetterCommand<InterpolationType>>(&uvInterpolationType_, (InterpolationType)newInterpolationType);
            EditorController::GetInstance()->PushCommand(std::move(command));
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    label = "Particle UV Scale##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        ImGui::Text("UVScale");
        label = "##ParticleUvScale" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            particleUvScale_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newScale) {
                if (!particleKeyFrames_->uvScaleCurve.empty()) {
                    particleKeyFrames_->uvScaleCurve[0].value = *_newScale;
                }
            });

        bool updatePerLifeTime    = (newFlag & static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;
        label                     = "Update UvScale PerLifeTime##" + _parentLabel;
        ImGui::Checkbox(label.c_str(), &updatePerLifeTime);
        if (updatePerLifeTime) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));

            if (particleKeyFrames_->uvScaleCurve.empty()) {
                particleKeyFrames_->uvScaleCurve.emplace_back(0.f, particleUvScale_);
            } else {
                particleKeyFrames_->uvScaleCurve[0].value = particleUvScale_;
            }
            ImGui::EditKeyFrame("UvScaleLine", particleKeyFrames_->uvScaleCurve, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {

            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Particle UV Rotate")) {
        ImGui::Text("UVRotate");
        label = "##ParticleUvRotate" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            particleUvRotate_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newUvRotate) {
                if (!particleKeyFrames_->uvRotateCurve.empty()) {
                    particleKeyFrames_->uvRotateCurve[0].value = *_newUvRotate;
                }
            });

        bool updatePerLifeTime    = (newFlag & static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;

        label = "Update UvRotate PerLifeTime##" + _parentLabel;
        ImGui::Checkbox(label.c_str(), &updatePerLifeTime);
        if (updatePerLifeTime) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime));

            if (particleKeyFrames_->uvRotateCurve.empty()) {
                particleKeyFrames_->uvRotateCurve.emplace_back(0.f, particleUvRotate_);
            } else {
                particleKeyFrames_->uvRotateCurve[0].value = particleUvRotate_;
            }

            label = "UvRotateLine##" + _parentLabel;
            ImGui::EditKeyFrame(label, particleKeyFrames_->uvRotateCurve, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {

            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime));
        }
        ImGui::TreePop();
    }

    label = "Particle UV Translate##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        ImGui::Text("UVTranslate");
        label = "##ParticleUvTranslate" + _parentLabel;
        DragGuiVectorCommand<3, float>(
            label,
            particleUvTranslate_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newUVTranslate) {
                if (!particleKeyFrames_->uvTranslateCurve.empty()) {
                    particleKeyFrames_->uvTranslateCurve[0].value = *_newUVTranslate;
                }
            });

        bool updatePerLifeTime    = (newFlag & static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;
        label                     = "Update UvTranslate PerLifeTime##" + _parentLabel;
        ImGui::Checkbox(label.c_str(), &updatePerLifeTime);
        if (updatePerLifeTime) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));

            if (particleKeyFrames_->uvTranslateCurve.empty()) {
                particleKeyFrames_->uvTranslateCurve.emplace_back(0.f, particleUvTranslate_);
            } else {
                particleKeyFrames_->uvTranslateCurve[0].value = particleUvTranslate_;
            }

            label = "UvTranslateLine##" + _parentLabel;
            ImGui::EditKeyFrame(label, particleKeyFrames_->uvTranslateCurve, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));
        }
        ImGui::TreePop();
    }

    if (newFlag != updateSettings_) {
        auto command = std::make_unique<SetterCommand<int32_t>>(&updateSettings_, newFlag);
        EditorController::GetInstance()->PushCommand(std::move(command));
        EditorController::GetInstance()->PushCommand(std::move(commandComboByChangeFlag));
    }
}
#endif // _DEBUG

void Emitter::Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList) {
    const Matrix4x4& viewMat = CameraManager::GetInstance()->GetTransform().viewMat;

    Matrix4x4 billboardMat = {};
    // パーティクルのスケール行列を事前計算
    Matrix4x4 scaleMat     = MakeMatrix4x4::Identity();
    Matrix4x4 rotateMat    = MakeMatrix4x4::Identity();
    Matrix4x4 translateMat = MakeMatrix4x4::Identity();
    if (particles_.empty()) {
        return;
    }

    if (particleIsBillBoard_) { // Bill Board
        // カメラの回転行列を取得し、平行移動成分をゼロにする
        Matrix4x4 cameraRotation = viewMat;
        cameraRotation[3][0]     = 0.0f;
        cameraRotation[3][1]     = 0.0f;
        cameraRotation[3][2]     = 0.0f;
        cameraRotation[3][3]     = 1.0f;

        // カメラの回転行列を反転してワールド空間への変換行列を作成
        billboardMat = cameraRotation.inverse();

        // 各パーティクルのワールド行列を計算
        for (size_t i = 0; i < particles_.size(); i++) {
            scaleMat     = MakeMatrix4x4::Scale(structuredTransform_.openData_[i].scale);
            rotateMat    = MakeMatrix4x4::RotateXYZ(structuredTransform_.openData_[i].rotate);
            translateMat = MakeMatrix4x4::Translate(structuredTransform_.openData_[i].translate);

            // ワールド行列を構築
            structuredTransform_.openData_[i].worldMat = scaleMat * billboardMat * translateMat;

            structuredTransform_.openData_[i].uvMat = particles_[i]->GetTransform().uvMat;
            structuredTransform_.openData_[i].color = particles_[i]->GetTransform().color;
        }
    } else {
        // 各パーティクルのワールド行列を計算
        for (size_t i = 0; i < particles_.size(); i++) {
            scaleMat     = MakeMatrix4x4::Scale(structuredTransform_.openData_[i].scale);
            rotateMat    = MakeMatrix4x4::RotateXYZ(structuredTransform_.openData_[i].rotate);
            translateMat = MakeMatrix4x4::Translate(structuredTransform_.openData_[i].translate);

            // ワールド行列を構築
            structuredTransform_.openData_[i].worldMat = scaleMat * rotateMat * translateMat;

            structuredTransform_.openData_[i].uvMat = particles_[i]->GetTransform().uvMat;
            structuredTransform_.openData_[i].color = particles_[i]->GetTransform().color;
        }
    }

    if (parent_) {
        for (size_t i = 0; i < particles_.size(); i++) {
            structuredTransform_.openData_[i].worldMat *= parent_->worldMat;
        }
    }

    structuredTransform_.ConvertToBuffer();
    structuredTransform_.SetForRootParameter(_commandList, 0);

    materialBuffer_.SetForRootParameter(_commandList, 2);
    _commandList->SetGraphicsRootDescriptorTable(
        3,
        TextureManager::GetDescriptorGpuHandle(textureIndex_));

    // 頂点バッファの設定
    _commandList->IASetVertexBuffers(0, 1, &mesh_.GetVBView());
    _commandList->IASetIndexBuffer(&mesh_.GetIBView());

    // 描画!!!
    _commandList->DrawIndexedInstanced(UINT(mesh_.GetIndexSize()), static_cast<UINT>(structuredTransform_.openData_.size()), 0, 0, 0);
}

void from_json(const nlohmann::json& j, Emitter& e) {
    j.at("blendMode").get_to(e.blendMode_);

    j.at("isActive").get_to(e.isActive_);
    j.at("isLoop").get_to(e.isLoop_);

    j.at("originPos").get_to(e.originPos_);

    j.at("textureFileName").get_to(e.textureFileName_);

    j.at("activeTime").get_to(e.activeTime_);
    j.at("spawnParticleVal").get_to(e.spawnParticleVal_);
    j.at("shapeType").get_to(e.shapeType_);
    j.at("spawnCoolTime").get_to(e.spawnCoolTime_);

    j.at("particleLifeTime").get_to(e.particleLifeTime_);
    j.at("particleIsBillBoard").get_to(e.particleIsBillBoard_);

    j.at("particleColor").get_to(e.particleColor_);
    j.at("particleUvScale").get_to(e.particleUvScale_);
    j.at("particleUvRotate").get_to(e.particleUvRotate_);
    j.at("particleUvTranslate").get_to(e.particleUvTranslate_);
    j.at("updateSettings").get_to(e.updateSettings_);
    j.at("startParticleScaleMin").get_to(e.startParticleScaleMin_);
    j.at("startParticleScaleMax").get_to(e.startParticleScaleMax_);
    j.at("startParticleRotateMin").get_to(e.startParticleRotateMin_);
    j.at("startParticleRotateMax").get_to(e.startParticleRotateMax_);
    j.at("startParticleVelocityMin").get_to(e.startParticleVelocityMin_);
    j.at("startParticleVelocityMax").get_to(e.startParticleVelocityMax_);
    j.at("updateParticleScaleMin").get_to(e.updateParticleScaleMin_);
    j.at("updateParticleScaleMax").get_to(e.updateParticleScaleMax_);
    j.at("updateParticleRotateMin").get_to(e.updateParticleRotateMin_);
    j.at("updateParticleRotateMax").get_to(e.updateParticleRotateMax_);
    j.at("updateParticleVelocityMin").get_to(e.updateParticleVelocityMin_);
    j.at("updateParticleVelocityMax").get_to(e.updateParticleVelocityMax_);
    j.at("randMass").get_to(e.randMass_);

    auto curveLoad = [](const nlohmann::json& _curveJson, auto& _curve) {
        for (auto& keyframeJson : _curveJson) {
            typename std::remove_reference<decltype(_curve)>::type::value_type key;
            keyframeJson.at("time").get_to(key.time);
            keyframeJson.at("value").get_to(key.value);
            _curve.push_back(key);
        }
    };

    if (!e.particleKeyFrames_) {
        e.particleKeyFrames_ = std::make_shared<ParticleKeyFrames>();
    }

    j.at("transformInterpolationType").get_to(e.transformInterpolationType_);
    j.at("colorInterpolationType").get_to(e.colorInterpolationType_);
    j.at("uvInterpolationType").get_to(e.uvInterpolationType_);

    if (j.find("scaleCurve") != j.end()) {
        curveLoad(j.at("scaleCurve"), e.particleKeyFrames_->scaleCurve);
    }
    if (j.find("rotateCurve") != j.end()) {
        curveLoad(j.at("rotateCurve"), e.particleKeyFrames_->rotateCurve);
    }
    if (j.find("velocityCurve") != j.end()) {
        curveLoad(j.at("velocityCurve"), e.particleKeyFrames_->velocityCurve);
    }

    if (j.find("colorCurve") != j.end()) {
        curveLoad(j.at("colorCurve"), e.particleKeyFrames_->colorCurve);
    }

    if (j.find("uvScaleCurve") != j.end()) {
        curveLoad(j.at("uvScaleCurve"), e.particleKeyFrames_->uvScaleCurve);
    }
    if (j.find("uvRotateCurve") != j.end()) {
        curveLoad(j.at("uvRotateCurve"), e.particleKeyFrames_->uvRotateCurve);
    }
    if (j.find("uvTranslateCurve") != j.end()) {
        curveLoad(j.at("uvTranslateCurve"), e.particleKeyFrames_->uvTranslateCurve);
    }

    if (j.find("EmitterShape") != j.end()) {
        nlohmann::json shapeJson = j["EmitterShape"];
        switch (e.shapeType_) {
        case EmitterShapeType::SPHERE: {
            EmitterSphere sphereShape;
            shapeJson.get_to(sphereShape);
            e.emitterSpawnShape_ = std::make_shared<EmitterSphere>(sphereShape);
            break;
        }
        case EmitterShapeType::BOX: {
            EmitterBox obbShape;
            shapeJson.get_to(obbShape);
            e.emitterSpawnShape_ = std::make_shared<EmitterBox>(obbShape);
            break;
        }
        case EmitterShapeType::CAPSULE: {
            EmitterCapsule capsuleShape;
            shapeJson.get_to(capsuleShape);
            e.emitterSpawnShape_ = std::make_shared<EmitterCapsule>(capsuleShape);
            break;
        }
        case EmitterShapeType::CONE: {
            EmitterCone coneShape;
            shapeJson.get_to(coneShape);
            e.emitterSpawnShape_ = std::make_shared<EmitterCone>(coneShape);
            break;
        }
        case EmitterShapeType::Count:
            LOG_ERROR("EmitterShapeType is not defined. Please check the EmitterShapeType.");
            break;
        default:
            break;
        }
    } else {
        e.emitterSpawnShape_ = std::make_shared<EmitterSphere>();
    }
}

void to_json(nlohmann::json& j, const Emitter& e) {
    j = nlohmann::json{
        {"blendMode", e.blendMode_},
        {"isActive", e.isActive_},
        {"isLoop", e.isLoop_},
        {"originPos", e.originPos_},
        {"activeTime", e.activeTime_},
        {"spawnParticleVal", e.spawnParticleVal_},
        {"shapeType", e.shapeType_},
        {"textureFileName", e.textureFileName_},
        {"spawnCoolTime", e.spawnCoolTime_},
        {"particleLifeTime", e.particleLifeTime_},
        {"particleIsBillBoard", e.particleIsBillBoard_},
        {"particleColor", e.particleColor_},
        {"particleUvScale", e.particleUvScale_},
        {"particleUvRotate", e.particleUvRotate_},
        {"particleUvTranslate", e.particleUvTranslate_},
        {"updateSettings", e.updateSettings_},
        {"startParticleScaleMin", e.startParticleScaleMin_},
        {"startParticleScaleMax", e.startParticleScaleMax_},
        {"startParticleRotateMin", e.startParticleRotateMin_},
        {"startParticleRotateMax", e.startParticleRotateMax_},
        {"startParticleVelocityMin", e.startParticleVelocityMin_},
        {"startParticleVelocityMax", e.startParticleVelocityMax_},
        {"updateParticleScaleMin", e.updateParticleScaleMin_},
        {"updateParticleScaleMax", e.updateParticleScaleMax_},
        {"updateParticleRotateMin", e.updateParticleRotateMin_},
        {"updateParticleRotateMax", e.updateParticleRotateMax_},
        {"updateParticleVelocityMin", e.updateParticleVelocityMin_},
        {"updateParticleVelocityMax", e.updateParticleVelocityMax_},
        {"randMass", e.randMass_}};

    nlohmann::json shapeJson = nlohmann::json::object();
    if (e.emitterSpawnShape_) {
        switch (e.emitterSpawnShape_->type) {
        case EmitterShapeType::SPHERE: {
            EmitterSphere* sphereShape = dynamic_cast<EmitterSphere*>(e.emitterSpawnShape_.get());
            if (sphereShape) {
                shapeJson = *sphereShape;
            } else {
                LOG_ERROR("EmitterSphere is not Sphere type. Please check the emitterSpawnShape_ type.");
            }
            break;
        }
        case EmitterShapeType::BOX: {
            EmitterBox* obbShape = dynamic_cast<EmitterBox*>(e.emitterSpawnShape_.get());

            if (obbShape) {
                shapeJson = *obbShape;
            } else {
                LOG_ERROR("EmitterBox is not OBB type. Please check the emitterSpawnShape_ type.");
            }

            break;
        }
        case EmitterShapeType::CAPSULE: {
            EmitterCapsule* capsuleShape = dynamic_cast<EmitterCapsule*>(e.emitterSpawnShape_.get());

            if (capsuleShape) {
                shapeJson = *capsuleShape;
            } else {
                LOG_ERROR("EmitterCapsule is not Capsule type. Please check the emitterSpawnShape_ type.");
            }

            break;
        }
        case EmitterShapeType::CONE: {

            EmitterCone* coneShape = dynamic_cast<EmitterCone*>(e.emitterSpawnShape_.get());
            if (coneShape) {
                shapeJson = *coneShape;
            } else {
                LOG_ERROR("EmitterCone is not Cone type. Please check the emitterSpawnShape_ type.");
            }

            break;
        }
        default:
            break;
        }
    }
    j["EmitterShape"] = shapeJson;

    auto curveSave = [](const auto& _curve) {
        nlohmann::json curve = nlohmann::json::array();

        for (auto& keyframe : _curve) {
            nlohmann::json keyframeJson = {
                {"time", keyframe.time},
                {"value", keyframe.value}};
            curve.push_back(keyframeJson);
        }
        return curve;
    };

    j["transformInterpolationType"] = e.transformInterpolationType_;
    j["colorInterpolationType"]     = e.colorInterpolationType_;
    j["uvInterpolationType"]        = e.uvInterpolationType_;

    if (e.particleKeyFrames_) {
        if ((e.updateSettings_ & (int32_t)ParticleUpdateType::ScalePerLifeTime) != 0) {
            j["scaleCurve"] = curveSave(e.particleKeyFrames_->scaleCurve);
        }
        if ((e.updateSettings_ & (int32_t)ParticleUpdateType::RotatePerLifeTime) != 0) {
            j["rotateCurve"] = curveSave(e.particleKeyFrames_->rotateCurve);
        }
        if ((e.updateSettings_ & (int32_t)ParticleUpdateType::VelocityPerLifeTime) != 0) {
            j["velocityCurve"] = curveSave(e.particleKeyFrames_->velocityCurve);
        }

        if ((e.updateSettings_ & (int32_t)ParticleUpdateType::ColorPerLifeTime) != 0) {
            j["colorCurve"] = curveSave(e.particleKeyFrames_->colorCurve);
        }

        if ((e.updateSettings_ & (int32_t)ParticleUpdateType::UvScalePerLifeTime) != 0) {
            j["uvScaleCurve"] = curveSave(e.particleKeyFrames_->uvScaleCurve);
        }
        if ((e.updateSettings_ & (int32_t)ParticleUpdateType::UvRotatePerLifeTime) != 0) {
            j["uvRotateCurve"] = curveSave(e.particleKeyFrames_->uvRotateCurve);
        }
        if ((e.updateSettings_ & (int32_t)ParticleUpdateType::UvTranslatePerLifeTime) != 0) {
            j["uvTranslateCurve"] = curveSave(e.particleKeyFrames_->uvTranslateCurve);
        }
    } else {
        LOG_ERROR("particleKeyFrames_ is nullptr. Please check the Emitter class.");
    }
}
