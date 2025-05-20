#include "Emitter.h"

// stl
// container
#include <array>

/// engine
#include "ECS/ECSManager.h"
#include "Engine.h"
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
// component
#include "component/renderer/primitive/Primitive.h"
// module
#include "camera/CameraManager.h"
#include "directX12/ShaderManager.h"
#include "globalVariables/GlobalVariables.h"
#include "model/ModelManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "myRandom/MyRandom.h"
#include "texture/TextureManager.h"
// assets
#include "EmitterShape.h"
#include "model/Model.h"

// math
#include "math/Matrix4x4.h"
#include <cmath>

#ifdef _DEBUG
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"
#endif // _DEBUG

static std::list<std::pair<std::string, std::string>> SearchTextureFile() {
    std::list<std::pair<std::string, std::string>> textureFiles = MyFileSystem::searchFile(kEngineResourceDirectory, "png", false);
    std::list<std::pair<std::string, std::string>> appPngFiles  = MyFileSystem::searchFile(kApplicationResourceDirectory, "png", false);

    textureFiles.insert(textureFiles.end(), appPngFiles.begin(), appPngFiles.end());

    return textureFiles;
}
static std::list<std::pair<std::string, std::string>> textureFiles = SearchTextureFile();

Emitter::Emitter() : IComponent(), currentCoolTime_(0.f), leftActiveTime_(0.f) {
    ComponentArray<Emitter>* emittterArray = ECSManager::getInstance()->getInstance()->getComponentArray<Emitter>();
    srvArray_                              = emittterArray->getSrvArray();
    isActive_       = false;
    leftActiveTime_ = 0.0f;
}

Emitter::Emitter(DxSrvArray* _srvArray) : IComponent(), srvArray_(_srvArray), currentCoolTime_(0.f), leftActiveTime_(0.f) {
    isActive_       = false;
    leftActiveTime_ = 0.0f;
}

Emitter::~Emitter() {}

void Emitter::Initialize(GameEntity* /*_entity*/) {
    { // Initialize DrawingData Size
        CalculateMaxSize();
        structuredTransform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice(), srvArray_, particleMaxSize_);
        particles_.reserve(particleMaxSize_);
    }

    { // Initialize Active State
        leftActiveTime_  = activeTime_;
        currentCoolTime_ = 0.f;
    }

    // shape Type
    switch (shapeType_) {
    case EmitterShapeType::SPHERE:
        emitterSpawnShape_ = std::make_shared<EmitterSphere>();
        break;
    case EmitterShapeType::OBB:
        emitterSpawnShape_ = std::make_shared<EmitterOBB>();
        break;
    case EmitterShapeType::CAPSULE:
        emitterSpawnShape_ = std::make_shared<EmitterCapsule>();
        break;
    case EmitterShapeType::CONE:
        emitterSpawnShape_ = std::make_shared<EmitterCone>();
        break;
    default:
        emitterSpawnShape_ = std::make_shared<EmitterSphere>();
        break;
    }

    // keyFrames

    // resource
    Plane planeGenerater;
    planeGenerater.createMesh(&mesh_);

    material_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

    if (!textureFileName_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureFileName_);
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
        leftActiveTime_ -= deltaTime;
        // Loop するなら スキップ
        if (!isLoop_) {
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
        // isAliveでないもの は 消す
        std::erase_if(particles_, [](std::shared_ptr<Particle>& particle) {
            return !particle->getIsAlive();
        });
    }

    { // Update Spawn
        currentCoolTime_ -= _deltaTime;
        // leftActiveTimeが 0以上のときだけ
        if (leftActiveTime_ > 0.f) {
            if (currentCoolTime_ <= 0.0f) {
                currentCoolTime_ = spawnCoolTime_;
                SpawnParticle();
            }
        }
    }

    { // push Drawing InstanceData
        structuredTransform_.openData_.clear();
        for (auto& particle : particles_) {
            structuredTransform_.openData_.push_back(particle->getTransform());
        }
    }
}

bool Emitter::Edit() {
#ifdef _DEBUG
    bool isChange = false;
    CheckBoxCommand("isActive", isActive_);
    CheckBoxCommand("isLoop", isLoop_);

    if (ImGui::Button("Play")) {
        leftActiveTime_ = activeTime_;
        isActive_       = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
    }

    ImGui::Spacing();

    if (ImGui::Button("reload FileList")) {
        textureFiles = SearchTextureFile();
    }
    {
        ImGui::Text("Texture :");
        ImGui::SameLine();

        if (ImGui::BeginCombo("ParticleTexture", textureFileName_.c_str())) {
            for (auto& fileName : textureFiles) {
                bool isSelected = (fileName.second == textureFileName_); // 現在選択中かどうか
                if (ImGui::Selectable(fileName.second.c_str(), isSelected)) {
                    textureFileName_ = fileName.first + "/" + fileName.second;
                    textureIndex_    = TextureManager::LoadTexture(textureFileName_);
                }
            }
            ImGui::EndCombo();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::TreeNode("ShapeType")) {
        EditShapeType();
        ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::TreeNode("Emitter")) {
        EditEmitter();
        ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::TreeNode("Particle")) {
        EditParticle();
        ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Calculate Particle MaxSize")) {
        CalculateMaxSize();
        if (structuredTransform_.capacity() <= particleMaxSize_) {
            structuredTransform_.resize(Engine::getInstance()->getDxDevice()->getDevice(), particleMaxSize_ * 2);
        }
    }

    return isChange;
#else
    return false;
#endif // _DEBUG
}

#ifdef _DEBUG
void Emitter::EditEmitter() {
    //======================== Emitter の 編集 ========================//
    if (ImGui::BeginCombo("BlendMode", blendModeStr[int(blendMode_)].c_str())) {
        for (int32_t i = 0; i < kBlendNum; i++) {
            bool isSelected = (blendMode_ == BlendMode(i)); // 現在選択中かどうか
            if (ImGui::Selectable(blendModeStr[i].c_str(), isSelected)) {
                auto command = std::make_unique<SetterCommand<BlendMode>>(&blendMode_, (BlendMode)i);
                EditorGroup::getInstance()->pushCommand(std::move(command));
                break;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Text("EmitterActiveTime");
    DragGuiCommand("##EmitterActiveTime", activeTime_, 0.1f);
    ImGui::Text("SpawnParticleVal");
    DragGuiCommand("##spawnParticleVal", spawnParticleVal_, 1, 0, 0, "%d");

    CheckBoxCommand("Particle Is BillBoard", particleIsBillBoard_);

    ImGui::Text("SpawnCoolTime");
    DragGuiCommand("##SpawnCoolTime", spawnCoolTime_, 0.1f, 0.f);
}

void Emitter::EditShapeType() {
    //======================== ShapeType の 切り替え ========================//
    if (ImGui::BeginCombo("EmitterShapeType", emitterShapeTypeWord_[static_cast<int32_t>(shapeType_)].c_str())) {
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
                        case EmitterShapeType::OBB:
                            emitterSpawnShape_ = std::make_shared<EmitterOBB>();
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

                EditorGroup::getInstance()->pushCommand(std::move(command));
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
        emitterSpawnShape_->Debug();
    }
}

void Emitter::EditParticle() {
    int32_t newFlag               = updateSettings_;
    auto commandComboByChangeFlag = std::make_unique<CommandCombo>();
    //======================== Particle の 編集 ========================//

    ImGui::Text("ParticleLifeTime");
    DragGuiCommand<float>(
        "##ParticleLifeTime",
        particleLifeTime_,
        0.1f, 0.f);

    ImGui::Separator();

    {
        int newInterpolationType = static_cast<int>(colorInterpolationType_);
        if (ImGui::Combo("ColorInterpolationType", &newInterpolationType, "LINEAR\0STEP\0\0", static_cast<int>(InterpolationType::COUNT))) {
            auto command = std::make_unique<SetterCommand<InterpolationType>>(&colorInterpolationType_, (InterpolationType)newInterpolationType);
            EditorGroup::getInstance()->pushCommand(std::move(command));
        }
    }

    if (ImGui::TreeNode("Particle Color")) {
        ColorEditGuiCommand<4>(
            "##Particle Color",
            particleColor_,
            [this](Vec<4, float>* _newColor) {
                if (!particleKeyFrames_->colorCurve_.empty()) {
                    particleKeyFrames_->colorCurve_[0].value = *_newColor;
                }
            });

        // curveで変更するかどうか
        bool updatePerLifeTime    = (newFlag & static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;

        ImGui::Checkbox("UpdateColorPerLifeTime", &updatePerLifeTime);
        if (updatePerLifeTime) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime));
            ImGui::EditColorKeyFrame("ColorLine", particleKeyFrames_->colorCurve_, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime));
        }
        ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    {
        int newInterpolationType = static_cast<int>(transformInterpolationType_);
        if (ImGui::Combo("ColorInterpolationType", &newInterpolationType, "LINEAR\0STEP\0\0", static_cast<int>(InterpolationType::COUNT))) {
            auto command = std::make_unique<SetterCommand<InterpolationType>>(&transformInterpolationType_, (InterpolationType)newInterpolationType);
            EditorGroup::getInstance()->pushCommand(std::move(command));
        }
    }

    if (ImGui::TreeNode("Particle Velocity")) {
        ImGui::Text("Min");
        DragGuiVectorCommand<3, float>(
            "##ParticleVelocityMin",
            startParticleVelocityMin_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newVec) {
                *_newVec                                    = MinElement(*_newVec, startParticleVelocityMax_);
                particleKeyFrames_->velocityCurve_[0].value = *_newVec;
            });

        startParticleVelocityMin_                   = MinElement(startParticleVelocityMin_, startParticleVelocityMax_);
        particleKeyFrames_->velocityCurve_[0].value = startParticleVelocityMin_;

        ImGui::Text("Max");
        DragGuiVectorCommand<3, float>(
            "##ParticleVelocityMax",
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
        ImGui::RadioButton("Update Velocity None", &randomOrPerLifeTime, 0);
        ImGui::RadioButton("Update Velocity Random", &randomOrPerLifeTime, 1);
        ImGui::RadioButton("Update Velocity PerLifeTime", &randomOrPerLifeTime, 2);

        bool isUsingGravity = (newFlag & static_cast<int32_t>(ParticleUpdateType::UsingGravity));
        if (ImGui::Checkbox("UsingGravity", &isUsingGravity)) {
            newFlag = isUsingGravity
                          ? newFlag | static_cast<int32_t>(ParticleUpdateType::UsingGravity)
                          : newFlag & ~static_cast<int32_t>(ParticleUpdateType::UsingGravity);
        }
        if (isUsingGravity) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::UsingGravity));

            DragGuiCommand<float>("Min Mass", randMass_[X], 0.1f, {}, {}, "%.3f", [this](float* _newVal) {
                *_newVal = (std::min)(*_newVal, randMass_[Y]);
            });

            DragGuiCommand<float>("Max Mass", randMass_[Y], 0.1f, {}, {}, "%.3f", [this](float* _newVal) {
                *_newVal = (std::max)(randMass_[X], *_newVal);
            });
        }

        if (randomOrPerLifeTime == 2) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::VelocityRandom));

            ImGui::EditKeyFrame("VelocityCurve", particleKeyFrames_->velocityCurve_, particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            // ランダムな速度を設定
            ImGui::Text("UpdateMin");
            DragGuiVectorCommand<3, float>(
                "##UpdateParticleVelocityMin",
                updateParticleVelocityMin_,
                0.1f,
                {}, {},
                "%.3f",
                [this](Vec<3, float>* _newMin) {
                    *_newMin = MinElement(*_newMin, updateParticleVelocityMax_);
                });

            updateParticleVelocityMin_ = MinElement(updateParticleVelocityMin_, updateParticleVelocityMax_);
            ImGui::Text("UpdateMax");
            DragGuiVectorCommand<3, float>(
                "##UpdateParticleVelocityMax",
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

    if (ImGui::TreeNode("Particle Scale")) {
        ImGui::Text("Min");
        DragGuiVectorCommand<3, float>(
            "##ParticleScaleMin",
            startParticleScaleMin_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newMin) {
                *_newMin                                 = MinElement(*_newMin, startParticleScaleMax_);
                particleKeyFrames_->scaleCurve_[0].value = *_newMin;
            });
        startParticleScaleMin_ = MinElement(startParticleScaleMin_, startParticleScaleMax_);

        ImGui::Text("Max");
        DragGuiVectorCommand<3, float>(
            "##ParticleScaleMax",
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

        ImGui::RadioButton("Update Scale None", &randomOrPerLifeTime, 0);
        ImGui::RadioButton("Update Scale Random", &randomOrPerLifeTime, 1);
        ImGui::RadioButton("Update Scale PerLifeTime", &randomOrPerLifeTime, 2);

        if (randomOrPerLifeTime == 2) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::ScaleRandom));

            ImGui::EditKeyFrame("ScaleLine", particleKeyFrames_->scaleCurve_, particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            // ランダムなScaleを設定
            ImGui::Text("UpdateMin");
            DragGuiVectorCommand<3, float>(
                "##UpdateParticleScaleMin",
                updateParticleScaleMin_,
                0.1f,
                {}, {},
                "%.3f",
                [this](Vec<3, float>* _newMin) {
                    *_newMin = MinElement(*_newMin, updateParticleScaleMax_);
                });
            updateParticleScaleMin_ = MinElement(updateParticleScaleMin_, updateParticleScaleMax_);

            ImGui::Text("UpdateMax");
            DragGuiVectorCommand<3, float>(
                "##UpdateParticleScaleMax",
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
        DragGuiVectorCommand<3, float>(
            "##ParticleRotateMin",
            startParticleRotateMin_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newMin) {
                *_newMin                                  = MinElement(*_newMin, startParticleRotateMax_);
                particleKeyFrames_->rotateCurve_[0].value = *_newMin;
            });
        startParticleRotateMin_                   = MinElement(startParticleRotateMin_, startParticleRotateMax_);
        particleKeyFrames_->rotateCurve_[0].value = startParticleRotateMin_;

        ImGui::Text("Max");
        DragGuiVectorCommand<3, float>(
            "##ParticleRotateMax",
            startParticleRotateMax_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newMax) {
                *_newMax = MaxElement(startParticleRotateMin_, *(_newMax));
            });
        startParticleRotateMax_ = MaxElement(startParticleRotateMin_, startParticleRotateMax_);

        int randomOrPerLifeTime    = (newFlag & static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime)) ? 2 : ((newFlag & static_cast<int32_t>(ParticleUpdateType::RotateRandom)) ? 1 : 0);
        int preRandomOrPerLifeTime = randomOrPerLifeTime;

        ImGui::RadioButton("Update Rotate None", &randomOrPerLifeTime, 0);
        ImGui::RadioButton("Update Rotate Random", &randomOrPerLifeTime, 1);
        ImGui::RadioButton("Update Rotate PerLifeTime", &randomOrPerLifeTime, 2);

        if (randomOrPerLifeTime == 2) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateRandom));

            ImGui::EditKeyFrame("RotateLine", particleKeyFrames_->rotateCurve_, particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            // ランダムな回転を設定
            ImGui::Text("UpdateMin");
            DragGuiVectorCommand<3, float>(
                "##UpdateParticleRotateMin",
                updateParticleRotateMin_,
                0.1f,
                {}, {},
                "%.3f",
                [this](Vec<3, float>* _newMin) {
                    *_newMin = MinElement(*_newMin, updateParticleRotateMax_);
                });
            updateParticleRotateMin_ = MinElement(updateParticleRotateMin_, updateParticleRotateMax_);

            ImGui::Text("UpdateMax");
            DragGuiVectorCommand<3, float>(
                "##UpdateParticleRotateMax",
                updateParticleRotateMax_,
                0.1f,
                {}, {},
                "%.3f",
                [this](Vec<3, float>* _newMax) {
                    *_newMax = MaxElement(updateParticleRotateMin_, *(_newMax));
                });
            updateParticleRotateMax_ = MaxElement(updateParticleRotateMin_, updateParticleRotateMax_);

            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::RotateRandom));
        } else if (preRandomOrPerLifeTime != 0 && randomOrPerLifeTime == 0) {
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::RotateRandom));
        }
        ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::TreeNode("UvCurveGenerator Form TextureAnimation")) {
        DragGuiVectorCommand("TileSize", tileSize_, 0.1f);
        DragGuiVectorCommand("TextureSize", textureSize_, 0.1f);
        DragGuiCommand("tilePerTime_", tilePerTime_);
        DragGuiCommand<float>("StartAnimationTime", startAnimationTime_, 0.1f, 0.f);
        DragGuiCommand<float>("AnimationTimeLength", animationTimeLength_, 0.1f, 0.f);
        if (ImGui::Button("Generate Curve")) {
            if (particleKeyFrames_) {
                particleLifeTime_ = animationTimeLength_;

                // すでにあるカーブを消す UV /scale,translate
                commandComboByChangeFlag->addCommand(
                    std::make_shared<ClearCommand<AnimationCurve<Vec3f>>>(&particleKeyFrames_->uvScaleCurve_));
                commandComboByChangeFlag->addCommand(
                    std::make_shared<ClearCommand<AnimationCurve<Vec3f>>>(&particleKeyFrames_->uvTranslateCurve_));

                // uvScale は Animation しない
                newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));
                commandComboByChangeFlag->addCommand(
                    std::make_shared<SetterCommand<Vector3f>>(&particleUvScale_, Vector3f(tileSize_[X] / textureSize_[X], tileSize_[Y] / textureSize_[Y], 0.f)));

                // uv Translate は Animation する
                newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));
                commandComboByChangeFlag->addCommand(
                    std::make_shared<SetterCommand<InterpolationType>>(&uvInterpolationType_, InterpolationType::STEP));

                // 最大タイル数と最大時間を計算
                int32_t maxTilesX = int32_t(textureSize_[X] / tileSize_[X]);
                int32_t maxTilesY = int32_t(textureSize_[Y] / tileSize_[Y]);
                int32_t maxTiles  = maxTilesX * maxTilesY;
                float maxTime     = maxTiles * tilePerTime_;

                // startAnimationTime_ を最大時間内に収める
                commandComboByChangeFlag->addCommand(
                    std::make_shared<SetterCommand<float>>(&startAnimationTime_, fmod(startAnimationTime_, maxTime)));

                // 初期の col と row を計算
                int32_t startTileIndex = int32_t(startAnimationTime_ / tilePerTime_);
                float col              = float(startTileIndex % maxTilesX);
                float row              = float(startTileIndex / maxTilesX);

                // UV座標を計算
                float x = col * (tileSize_[X] / textureSize_[X]);
                float y = row * (tileSize_[Y] / textureSize_[Y]);

                commandComboByChangeFlag->addCommand(
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
                commandComboByChangeFlag->addCommand(
                    std::make_shared<SetterCommand<AnimationCurve<Vec3f>>>(&particleKeyFrames_->uvTranslateCurve_, uvTranslateCurve));
            }
        }

        ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    {
        int newInterpolationType = static_cast<int>(uvInterpolationType_);
        if (ImGui::Combo("UvInterpolationType", &newInterpolationType, "LINEAR\0STEP\0\0", static_cast<int>(InterpolationType::COUNT))) {
            auto command = std::make_unique<SetterCommand<InterpolationType>>(&uvInterpolationType_, (InterpolationType)newInterpolationType);
            EditorGroup::getInstance()->pushCommand(std::move(command));
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::TreeNode("Particle UV Scale")) {
        ImGui::Text("UVScale");
        DragGuiVectorCommand<3, float>(
            "##ParticleUvScale",
            particleUvScale_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newScale) {
                if (!particleKeyFrames_->uvScaleCurve_.empty()) {
                    particleKeyFrames_->uvScaleCurve_[0].value = *_newScale;
                }
            });

        bool updatePerLifeTime    = (newFlag & static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;
        ImGui::Checkbox("Update UvScale PerLifeTime", &updatePerLifeTime);
        if (updatePerLifeTime) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));

            ImGui::EditKeyFrame("UvScaleLine", particleKeyFrames_->uvScaleCurve_, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            // すでにあるカーブを消す
            auto comamndGroup = std::make_unique<CommandCombo>();
            comamndGroup->addCommand(std::make_shared<ClearCommand<AnimationCurve<Vec3f>>>(&particleKeyFrames_->uvScaleCurve_));
            comamndGroup->addCommand(std::make_shared<AddElementCommand<AnimationCurve<Vec3f>>>(&particleKeyFrames_->uvScaleCurve_, KeyFrame<Vec3f>(0.0f, particleUvScale_)));
            EditorGroup::getInstance()->pushCommand(std::move(comamndGroup));

            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Particle UV Rotate")) {
        ImGui::Text("UVRotate");

        DragGuiVectorCommand<3, float>(
            "##ParticleUvRotate",
            particleUvRotate_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newUvRotate) {
                particleKeyFrames_->uvRotateCurve_[0].value = *_newUvRotate;
            });

        bool updatePerLifeTime    = (newFlag & static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;

        ImGui::Checkbox("Update UvRotate PerLifeTime", &updatePerLifeTime);
        if (updatePerLifeTime) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime));

            ImGui::EditKeyFrame("UvRotateLine", particleKeyFrames_->uvRotateCurve_, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            // すでにあるカーブを消す
            auto comamndGroup = std::make_unique<CommandCombo>();
            comamndGroup->addCommand(std::make_shared<ClearCommand<AnimationCurve<Vec3f>>>(&particleKeyFrames_->uvRotateCurve_));
            comamndGroup->addCommand(std::make_shared<AddElementCommand<AnimationCurve<Vec3f>>>(&particleKeyFrames_->uvRotateCurve_, KeyFrame<Vec3f>(0.0f, particleUvRotate_)));
            EditorGroup::getInstance()->pushCommand(std::move(comamndGroup));

            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime));
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Particle UV Translate")) {
        ImGui::Text("UVTranslate");
        DragGuiVectorCommand<3, float>(
            "##ParticleUvTranslate",
            particleUvTranslate_,
            0.1f,
            {}, {},
            "%.3f",
            [this](Vec<3, float>* _newUVTranslate) {
                particleKeyFrames_->uvTranslateCurve_[0].value = *_newUVTranslate;
            });

        bool updatePerLifeTime    = (newFlag & static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;
        ImGui::Checkbox("Update UvTransform PerLifeTime", &updatePerLifeTime);
        if (updatePerLifeTime) {
            newFlag = (newFlag | static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));

            ImGui::EditKeyFrame("UvTranslateLine", particleKeyFrames_->uvTranslateCurve_, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            // すでにあるカーブを消す
            auto comamndGroup = std::make_unique<CommandCombo>();
            comamndGroup->addCommand(std::make_shared<ClearCommand<AnimationCurve<Vec3f>>>(&particleKeyFrames_->uvTranslateCurve_));
            comamndGroup->addCommand(std::make_shared<AddElementCommand<AnimationCurve<Vec3f>>>(&particleKeyFrames_->uvTranslateCurve_, KeyFrame<Vec3f>(0.0f, particleUvTranslate_)));
            EditorGroup::getInstance()->pushCommand(std::move(comamndGroup));

            newFlag = (newFlag & ~static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));
        }
        ImGui::TreePop();
    }

    if (newFlag != updateSettings_) {
        auto command = std::make_unique<SetterCommand<int32_t>>(&updateSettings_, newFlag);
        EditorGroup::getInstance()->pushCommand(std::move(command));
        EditorGroup::getInstance()->pushCommand(std::move(commandComboByChangeFlag));
    }
}
#endif // _DEBUG

void Emitter::Draw(ID3D12GraphicsCommandList* _commandList) {
    const Matrix4x4& viewMat = CameraManager::getInstance()->getTransform().viewMat;

    Matrix4x4 rotateMat = {};
    // パーティクルのスケール行列を事前計算
    Matrix4x4 scaleMat = MakeMatrix::Scale({1.0f, 1.0f, 1.0f});

    if (particleIsBillBoard_) { // Bill Board
                                // カメラの回転行列を取得し、平行移動成分をゼロにする
        Matrix4x4 cameraRotation = viewMat;
        cameraRotation[3][0]     = 0.0f;
        cameraRotation[3][1]     = 0.0f;
        cameraRotation[3][2]     = 0.0f;
        cameraRotation[3][3]     = 1.0f;

        // カメラの回転行列を反転してワールド空間への変換行列を作成
        rotateMat = cameraRotation.inverse();

        // 各パーティクルのワールド行列を計算
        for (size_t i = 0; i < particles_.size(); i++) {
            scaleMat = MakeMatrix::Scale(structuredTransform_.openData_[i].scale);
            // 平行移動行列を計算
            Matrix4x4 translateMat = MakeMatrix::Translate(structuredTransform_.openData_[i].translate + originPos_);
            // ワールド行列を構築
            structuredTransform_.openData_[i].worldMat = scaleMat * rotateMat * translateMat;

            structuredTransform_.openData_[i].uvMat = particles_[i]->getTransform().uvMat;
            structuredTransform_.openData_[i].color = particles_[i]->getTransform().color;
        }
    } else {
        // 各パーティクルのワールド行列を計算
        for (size_t i = 0; i < particles_.size(); i++) {
            scaleMat  = MakeMatrix::Scale(structuredTransform_.openData_[i].scale);
            rotateMat = MakeMatrix::RotateXYZ(structuredTransform_.openData_[i].rotate);
            // 平行移動行列を計算
            Matrix4x4 translateMat = MakeMatrix::Translate(structuredTransform_.openData_[i].translate + originPos_);

            // ワールド行列を構築
            structuredTransform_.openData_[i].worldMat = scaleMat * rotateMat * translateMat;

            structuredTransform_.openData_[i].uvMat = particles_[i]->getTransform().uvMat;
            structuredTransform_.openData_[i].color = particles_[i]->getTransform().color;
        }
    }

    if (parent_) {
        for (size_t i = 0; i < particles_.size(); i++) {
            structuredTransform_.openData_[i].worldMat *= parent_->worldMat;
        }
    }

    structuredTransform_.ConvertToBuffer();
    structuredTransform_.SetForRootParameter(_commandList, 0);

    ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
    _commandList->SetDescriptorHeaps(1, ppHeaps);

    _commandList->SetGraphicsRootDescriptorTable(
        3,
        TextureManager::getDescriptorGpuHandle(textureIndex_));

    _commandList->IASetVertexBuffers(0, 1, &mesh_.getVBView());
    _commandList->IASetIndexBuffer(&mesh_.getIBView());

    material_.SetForRootParameter(_commandList, 2);
    // 描画!!!
    _commandList->DrawIndexedInstanced(UINT(mesh_.getIndexSize()), static_cast<UINT>(structuredTransform_.openData_.size()), 0, 0, 0);
}

void Emitter::CalculateMaxSize() {
    // 1秒あたりの生成回数
    float spawnRatePerSecond = spawnParticleVal_ / spawnCoolTime_;

    // 最大個数
    particleMaxSize_ = (std::max<uint32_t>)((std::max<uint32_t>)(static_cast<uint32_t>(std::ceil(spawnRatePerSecond * particleLifeTime_)), spawnParticleVal_), particleMaxSize_);
}

void Emitter::SpawnParticle() {
    // スポーンして良い数
    int32_t canSpawnParticleValue_ = (std::min<int32_t>)(spawnParticleVal_, static_cast<int32_t>(particleMaxSize_ - particles_.size()));

    for (int32_t i = 0; i < canSpawnParticleValue_; i++) {
        // 割りたてる Transform の 初期化
        structuredTransform_.openData_.push_back({});
        auto& transform = structuredTransform_.openData_.back();

        transform.color = particleColor_;

        MyRandom::Float randX;
        MyRandom::Float randY;
        MyRandom::Float randZ;

        randX.setRange(startParticleVelocityMin_.v[X], startParticleVelocityMax_.v[X]);
        randY.setRange(startParticleVelocityMin_.v[Y], startParticleVelocityMax_.v[Y]);
        randZ.setRange(startParticleVelocityMin_.v[Z], startParticleVelocityMax_.v[Z]);
        Vec3f velocity = {randX.get(), randY.get(), randZ.get()};

        randX.setRange(startParticleScaleMin_.v[X], startParticleScaleMax_.v[X]);
        randY.setRange(startParticleScaleMin_.v[Y], startParticleScaleMax_.v[Y]);
        randZ.setRange(startParticleScaleMin_.v[Z], startParticleScaleMax_.v[Z]);
        transform.scale = {randX.get(), randY.get(), randZ.get()};

        randX.setRange(startParticleRotateMin_.v[X], startParticleRotateMax_.v[X]);
        randY.setRange(startParticleRotateMin_.v[Y], startParticleRotateMax_.v[Y]);
        randZ.setRange(startParticleRotateMin_.v[Z], startParticleRotateMax_.v[Z]);
        transform.rotate    = {randX.get(), randY.get(), randZ.get()};
        transform.translate = emitterSpawnShape_->getSpawnPos();

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
            spawnedParticle->setUpdateVelocityMinMax(&updateParticleVelocityMin_, &updateParticleVelocityMax_);
        }
        if (updateSettings_ & int(ParticleUpdateType::UsingGravity)) {
            randX.setRange(randMass_[X], randMass_[Y]);
            spawnedParticle->setMass(randX.get());
        }
        if (updateSettings_ & int(ParticleUpdateType::ScaleRandom)) {
            spawnedParticle->setUpdateScaleMinMax(&updateParticleScaleMin_, &updateParticleScaleMax_);
        }
        if (updateSettings_ & int(ParticleUpdateType::RotateRandom)) {
            spawnedParticle->setUpdateRotateMinMax(&updateParticleRotateMin_, &updateParticleRotateMax_);
        }

        spawnedParticle->setKeyFrames(updateSettings_, particleKeyFrames_.get());
        spawnedParticle->UpdateKeyFrameValues();
    }
}

void Emitter::PlayStart() {
    isActive_        = true;
    leftActiveTime_  = activeTime_;
    currentCoolTime_ = 0.f;
}

void Emitter::PlayStop() {
    isActive_       = false;
    leftActiveTime_ = 0.f;
}

void from_json(const nlohmann::json& j, Emitter& e) {
    j.at("blendMode").get_to(e.blendMode_);

    j.at("isActive").get_to(e.isActive_);
    j.at("isLoop").get_to(e.isLoop_);

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
    curveLoad(j.at("scaleCurve"), e.particleKeyFrames_->scaleCurve_);
    curveLoad(j.at("rotateCurve"), e.particleKeyFrames_->rotateCurve_);
    curveLoad(j.at("velocityCurve"), e.particleKeyFrames_->velocityCurve_);

    j.at("colorInterpolationType").get_to(e.colorInterpolationType_);
    curveLoad(j.at("colorCurve"), e.particleKeyFrames_->colorCurve_);

    j.at("uvInterpolationType").get_to(e.uvInterpolationType_);
    curveLoad(j.at("uvScaleCurve"), e.particleKeyFrames_->uvScaleCurve_);
    curveLoad(j.at("uvRotateCurve"), e.particleKeyFrames_->uvRotateCurve_);
    curveLoad(j.at("uvTranslateCurve"), e.particleKeyFrames_->uvTranslateCurve_);
}

void to_json(nlohmann::json& j, const Emitter& e) {
    j = nlohmann::json{
        {"blendMode", e.blendMode_},
        {"isActive", e.isActive_},
        {"isLoop", e.isLoop_},
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
    j["scaleCurve"]                 = curveSave(e.particleKeyFrames_->scaleCurve_);
    j["rotateCurve"]                = curveSave(e.particleKeyFrames_->rotateCurve_);
    j["velocityCurve"]              = curveSave(e.particleKeyFrames_->velocityCurve_);

    j["colorInterpolationType"] = e.colorInterpolationType_;
    j["colorCurve"]             = curveSave(e.particleKeyFrames_->colorCurve_);

    j["uvInterpolationType"] = e.uvInterpolationType_;
    j["uvScaleCurve"]        = curveSave(e.particleKeyFrames_->uvScaleCurve_);
    j["uvRotateCurve"]       = curveSave(e.particleKeyFrames_->uvRotateCurve_);
    j["uvTranslateCurve"]    = curveSave(e.particleKeyFrames_->uvTranslateCurve_);
}
