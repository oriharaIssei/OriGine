#include "Emitter.h"

// stl
// container
#include <array>

/// engine
#include "ECS/ECSManager.h"
#include "Engine.h"
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
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
#include "imgui/imgui.h"
#include "util/timeline/Timeline.h"
#endif // _DEBUG

static std::list<std::pair<std::string, std::string>> SearchModelFile() {
    std::list<std::pair<std::string, std::string>> modelFiles   = MyFileSystem::searchFile(kEngineResourceDirectory, "obj", false);
    std::list<std::pair<std::string, std::string>> gltfFiles    = MyFileSystem::searchFile(kEngineResourceDirectory, "gltf", false);
    std::list<std::pair<std::string, std::string>> appObjFiles  = MyFileSystem::searchFile(kApplicationResourceDirectory, "obj", false);
    std::list<std::pair<std::string, std::string>> appGltfFiles = MyFileSystem::searchFile(kApplicationResourceDirectory, "gltf", false);

    modelFiles.insert(modelFiles.end(), gltfFiles.begin(), gltfFiles.end());
    modelFiles.insert(modelFiles.end(), appObjFiles.begin(), appObjFiles.end());
    modelFiles.insert(modelFiles.end(), appGltfFiles.begin(), appGltfFiles.end());

    return modelFiles;
}

static std::list<std::pair<std::string, std::string>> SearchTextureFile() {
    std::list<std::pair<std::string, std::string>> textureFiles = MyFileSystem::searchFile(kEngineResourceDirectory, "png", false);
    std::list<std::pair<std::string, std::string>> appPngFiles  = MyFileSystem::searchFile(kApplicationResourceDirectory, "png", false);

    textureFiles.insert(textureFiles.end(), appPngFiles.begin(), appPngFiles.end());

    return textureFiles;
}
static std::list<std::pair<std::string, std::string>> objectFiles  = SearchModelFile();
static std::list<std::pair<std::string, std::string>> textureFiles = SearchTextureFile();

Emitter::Emitter() : IComponent(), currentCoolTime_(0.f), leftActiveTime_(0.f) {
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
        structuredTransform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice(), srvArray_, particleMaxSize_);
        particles_.reserve(particleMaxSize_);
    }

    { // Initialize Active State
        isActive_        = true;
        leftActiveTime_  = activeTime_;
        currentCoolTime_ = 0.f;
    }

    emitterSpawnShape_ = std::make_unique<EmitterSphere>();

    particleKeyFrames_ = std::make_unique<ParticleKeyFrames>();
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
    ImGui::Checkbox("isActive", &isActive_);
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        isActive_ = false;
    }

    ImGui::Checkbox("isLoop", &isLoop_);

    ImGui::Spacing();

    if (ImGui::Button("reload FileList")) {
        objectFiles  = SearchModelFile();
        textureFiles = SearchTextureFile();
    }

    ImGui::Text("Particle Model : ");
    ImGui::SameLine();
    if (ImGui::BeginCombo("ParticleModel", modelFileName_.c_str())) {
        for (auto& fileName : objectFiles) {
            bool isSelected = (fileName.second == modelFileName_); // 現在選択中かどうか
            if (ImGui::Selectable(fileName.second.c_str(), isSelected)) {
                particleModel_  = ModelManager::getInstance()->Create(fileName.first, fileName.second);
                modelDirectory_ = fileName.first;
                modelFileName_  = fileName.second;
            }
        }
        ImGui::EndCombo();
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

    if (ImGui::TreeNode("ShapeType")) {
        EditShapeType();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Emitter")) {
        EditEmitter();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Particle")) {
        EditParticle();
        ImGui::TreePop();
    }

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

void Emitter::Save(BinaryWriter& _writer) {
    _writer.Write("isActive", isActive_);
    _writer.Write("isLoop", isLoop_);

    _writer.Write("activeTime", activeTime_);
    _writer.Write("spawnParticleVal", spawnParticleVal_);
    _writer.Write("spawnCoolTime", spawnCoolTime_);

    _writer.Write("modelDirectory", modelDirectory_);
    _writer.Write("modelFileName", modelFileName_);
    _writer.Write("textureFileName", textureFileName_);

    _writer.Write("blendMode", static_cast<int32_t>(blendMode_));
    _writer.Write("particleIsBillBoard", particleIsBillBoard_);
    _writer.Write("shapeType", static_cast<int32_t>(shapeType_));
    emitterSpawnShape_->Save(_writer);

    _writer.Write("particleLifeTime", particleLifeTime_);
    _writer.Write<4, float>("particleColor", particleColor_);
    _writer.Write<3, float>("startParticleVelocityMin", startParticleVelocityMin_);
    _writer.Write<3, float>("startParticleVelocityMax", startParticleVelocityMax_);
    _writer.Write<3, float>("updateParticleVelocityMin", updateParticleVelocityMin_);
    _writer.Write<3, float>("updateParticleVelocityMax", updateParticleVelocityMax_);
    _writer.Write<3, float>("startParticleScaleMin", startParticleScaleMin_);
    _writer.Write<3, float>("startParticleScaleMax", startParticleScaleMax_);
    _writer.Write<3, float>("updateParticleScaleMin", updateParticleScaleMin_);
    _writer.Write<3, float>("updateParticleScaleMax", updateParticleScaleMax_);

    _writer.Write<3, float>("uvScale", particleUvScale_);
    _writer.Write<3, float>("uvRotate", particleUvRotate_);
    _writer.Write<3, float>("uvTranslate", particleUvTranslate_);

    _writer.Write<int>("transformInterpolationType", static_cast<int>(transformInterpolationType_));
    _writer.Write<int>("colorInterpolationType", static_cast<int>(colorInterpolationType_));
    _writer.Write<int>("uvInterpolationType", static_cast<int>(uvInterpolationType_));

    _writer.Write("updateSettings", updateSettings_);
    if (updateSettings_ != 0) {
        if (particleKeyFrames_) {
            particleKeyFrames_->SaveKeyFrames(_writer);
        }
    }
}

void Emitter::Load(BinaryReader& _reader) {
    _reader.Read("isActive", isActive_);
    _reader.Read("isLoop", isLoop_);

    _reader.Read("activeTime", activeTime_);
    _reader.Read("spawnParticleVal", spawnParticleVal_);
    _reader.Read("spawnCoolTime", spawnCoolTime_);

    _reader.Read("modelDirectory", modelDirectory_);
    _reader.Read("modelFileName", modelFileName_);
    if (!modelFileName_.empty()) {
        particleModel_ = ModelManager::getInstance()->Create(modelDirectory_, modelFileName_);
    }
    _reader.Read("textureFileName", textureFileName_);
    if (!textureFileName_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureFileName_);
    }

    int32_t blendMode;
    _reader.Read("blendMode", blendMode);
    blendMode_ = BlendMode(blendMode);
    _reader.Read("particleIsBillBoard", particleIsBillBoard_);

    int32_t shapeType;
    _reader.Read("shapeType", shapeType);
    shapeType_ = EmitterShapeType(shapeType);
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
    emitterSpawnShape_->Load(_reader);

    _reader.Read("particleLifeTime", particleLifeTime_);
    _reader.Read<4, float>("particleColor", particleColor_);
    _reader.Read<3, float>("startParticleVelocityMin", startParticleVelocityMin_);
    _reader.Read<3, float>("startParticleVelocityMax", startParticleVelocityMax_);
    _reader.Read<3, float>("updateParticleVelocityMin", updateParticleVelocityMin_);
    _reader.Read<3, float>("updateParticleVelocityMax", updateParticleVelocityMax_);
    _reader.Read<3, float>("startParticleScaleMin", startParticleScaleMin_);
    _reader.Read<3, float>("startParticleScaleMax", startParticleScaleMax_);
    _reader.Read<3, float>("updateParticleScaleMin", updateParticleScaleMin_);
    _reader.Read<3, float>("updateParticleScaleMax", updateParticleScaleMax_);

    _reader.Read<3, float>("uvScale", particleUvScale_);
    _reader.Read<3, float>("uvRotate", particleUvRotate_);
    _reader.Read<3, float>("uvTranslate", particleUvTranslate_);

    _reader.Read("updateSettings", updateSettings_);
    if (updateSettings_ != 0) {
        if (!particleKeyFrames_) {
            particleKeyFrames_ = std::make_shared<ParticleKeyFrames>();
        }
        particleKeyFrames_->LoadKeyFrames(_reader);
    }

    // InterpolationType
    int transformInterpolationType;
    _reader.Read<int>("transformInterpolationType", transformInterpolationType);
    transformInterpolationType_ = InterpolationType(transformInterpolationType);

    int colorInterpolationType;
    _reader.Read<int>("colorInterpolationType", colorInterpolationType);
    colorInterpolationType_ = InterpolationType(colorInterpolationType);

    int uvInterpolationType;
    _reader.Read<int>("uvInterpolationType", uvInterpolationType);
    uvInterpolationType_ = InterpolationType(uvInterpolationType);

    { // Initialize DrawingData Size
        CalculateMaxSize();
        structuredTransform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice(), srvArray_, particleMaxSize_);
        particles_.reserve(particleMaxSize_);
    }

    leftActiveTime_  = activeTime_;
    currentCoolTime_ = 0.f;
}

#ifdef _DEBUG
void Emitter::EditEmitter() {
    //======================== Emitter の 編集 ========================//
    if (ImGui::BeginCombo("BlendMode", blendModeStr[int(blendMode_)].c_str())) {
        for (int32_t i = 0; i < kBlendNum; i++) {
            bool isSelected = (blendMode_ == BlendMode(i)); // 現在選択中かどうか
            if (ImGui::Selectable(blendModeStr[i].c_str(), isSelected)) {
                blendMode_ = BlendMode(i);
                break;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Text("EmitterActiveTime");
    ImGui::DragFloat("##EmitterActiveTime", &activeTime_, 0.1f);
    ImGui::Text("SpawnParticleVal");
    if (ImGui::DragInt("##spawnParticleVal", &spawnParticleVal_, 1, 0)) {
        CalculateMaxSize();
    }

    ImGui::Checkbox("Particle Is BillBoard", &particleIsBillBoard_);

    ImGui::Text("SpawnCoolTime");
    ImGui::DragFloat("##SpawnCoolTime", &spawnCoolTime_, 0.1f, 0);
}

void Emitter::EditShapeType() {
    //======================== ShapeType の 切り替え ========================//
    if (ImGui::BeginCombo("EmitterShapeType", emitterShapeTypeWord_[static_cast<int32_t>(shapeType_)].c_str())) {
        for (int32_t i = 0; i < shapeTypeCount; i++) {
            bool isSelected = (shapeType_ == EmitterShapeType(i)); // 現在選択中かどうか

            if (ImGui::Selectable(emitterShapeTypeWord_[i].c_str(), isSelected)) {
                shapeType_ = EmitterShapeType(i);

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
                    break;
                }
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
    //======================== Particle の 編集 ========================//

    ImGui::Text("ParticleLifeTime");
    float preLifeTime = particleLifeTime_;
    if (ImGui::DragFloat("##ParticleLifeTime", &particleLifeTime_, 0.1f, 0)) {
        // 各Curveのノードの時間を変更前と同じ割合になるように
        for (auto& colorNode : particleKeyFrames_->colorCurve_) {
            colorNode.time = (colorNode.time / preLifeTime) * particleLifeTime_;
        }
        for (auto& speedNode : particleKeyFrames_->velocityCurve_) {
            speedNode.time = (speedNode.time / preLifeTime) * particleLifeTime_;
        }
        for (auto& scaleNode : particleKeyFrames_->scaleCurve_) {
            scaleNode.time = (scaleNode.time / preLifeTime) * particleLifeTime_;
        }
        for (auto& rotateNode : particleKeyFrames_->rotateCurve_) {
            rotateNode.time = (rotateNode.time / preLifeTime) * particleLifeTime_;
        }

        for (auto& uvScaleNode : particleKeyFrames_->uvScaleCurve_) {
            uvScaleNode.time = (uvScaleNode.time / preLifeTime) * particleLifeTime_;
        }
        for (auto& uvRotateNode : particleKeyFrames_->uvRotateCurve_) {
            uvRotateNode.time = (uvRotateNode.time / preLifeTime) * particleLifeTime_;
        }
        for (auto& uvTranslateNode : particleKeyFrames_->uvTranslateCurve_) {
            uvTranslateNode.time = (uvTranslateNode.time / preLifeTime) * particleLifeTime_;
        }
    }

    if (ImGui::TreeNode("Particle Color")) {

        ImGui::Combo("ColorInterpolationType", reinterpret_cast<int*>(&colorInterpolationType_), "LINEAR\0STEP\0\0", static_cast<int>(InterpolationType::COUNT));

        ImGui::ColorEdit4("##Particle Color", particleColor_.v);
        // curveで変更するかどうか
        bool updatePerLifeTime    = (updateSettings_ & static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;
        ImGui::Checkbox("UpdateColorPerLifeTime", &updatePerLifeTime);
        if (updatePerLifeTime) {
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime));

            particleKeyFrames_->colorCurve_[0].value = particleColor_;
            ImGui::EditColorKeyFrame("ColorLine", particleKeyFrames_->colorCurve_, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime));
        }
        ImGui::TreePop();
    }

    ImGui::Combo("TransformInterpolationType", reinterpret_cast<int*>(&colorInterpolationType_), "LINEAR\0STEP\0\0", static_cast<int>(InterpolationType::COUNT));

    if (ImGui::TreeNode("Particle Velocity")) {
        ImGui::Text("Min");
        ImGui::DragFloat3("##ParticleVelocityMin", startParticleVelocityMin_.v, 0.1f);
        ImGui::Text("Max");
        ImGui::DragFloat3("##ParticleVelocityMax", startParticleVelocityMax_.v, 0.1f);

        startParticleVelocityMin_ = (std::min)(startParticleVelocityMin_, startParticleVelocityMax_);
        startParticleVelocityMax_ = (std::max)(startParticleVelocityMin_, startParticleVelocityMax_);

        int randomOrPerLifeTime    = (updateSettings_ & static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime)) ? 2 : ((updateSettings_ & static_cast<int32_t>(ParticleUpdateType::VelocityRandom)) ? 1 : 0);
        int preRandomOrPerLifeTime = randomOrPerLifeTime;
        ImGui::RadioButton("Update Velocity None", &randomOrPerLifeTime, 0);
        ImGui::RadioButton("Update Velocity Random", &randomOrPerLifeTime, 1);
        ImGui::RadioButton("Update Velocity PerLifeTime", &randomOrPerLifeTime, 2);
        if (randomOrPerLifeTime == 2) {
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime));
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::VelocityRandom));

            particleKeyFrames_->velocityCurve_[0].value = startParticleVelocityMax_;
            ImGui::EditKeyFrame("SpeedLine", particleKeyFrames_->velocityCurve_, particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            // ランダムな速度を設定
            ImGui::Text("UpdateMin");
            ImGui::DragFloat3("##UpdateParticleVelocityMin", updateParticleVelocityMin_.v, 0.1f);
            ImGui::Text("UpdateMax");
            ImGui::DragFloat3("##UpdateParticleVelocityMax", updateParticleVelocityMax_.v, 0.1f);

            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime));
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::VelocityRandom));
        } else if (preRandomOrPerLifeTime != 0 && randomOrPerLifeTime == 0) {
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime));
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::VelocityRandom));
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Particle Scale")) {
        ImGui::Text("Min");
        ImGui::DragFloat3("##ParticleScaleMin", startParticleScaleMin_.v, 0.1f);
        ImGui::Text("Max");
        ImGui::DragFloat3("##ParticleScaleMax", startParticleScaleMax_.v, 0.1f);

        startParticleScaleMin_ = (std::min)(startParticleScaleMin_, startParticleScaleMax_);
        startParticleScaleMax_ = (std::max)(startParticleScaleMin_, startParticleScaleMax_);

        // curveかrandom か
        int randomOrPerLifeTime    = (updateSettings_ & static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime)) ? 2 : ((updateSettings_ & static_cast<int32_t>(ParticleUpdateType::ScaleRandom)) ? 1 : 0);
        int preRandomOrPerLifeTime = randomOrPerLifeTime;
        ImGui::RadioButton("Update Scale None", &randomOrPerLifeTime, 0);
        ImGui::RadioButton("Update Scale Random", &randomOrPerLifeTime, 1);
        ImGui::RadioButton("Update Scale PerLifeTime", &randomOrPerLifeTime, 2);
        if (randomOrPerLifeTime == 2) {
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime));
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::ScaleRandom));

            particleKeyFrames_->scaleCurve_[0].value = startParticleScaleMax_;
            ImGui::EditKeyFrame("ScaleLine", particleKeyFrames_->scaleCurve_, particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            // ランダムなScaleを設定
            ImGui::Text("UpdateMin");
            ImGui::DragFloat3("##UpdateParticleScaleMin", updateParticleScaleMin_.v, 0.1f);
            ImGui::Text("UpdateMax");
            ImGui::DragFloat3("##UpdateParticleScaleMax", updateParticleScaleMax_.v, 0.1f);

            // ランダムなスケールを設定
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime));
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::ScaleRandom));
        } else if (preRandomOrPerLifeTime != 0 && randomOrPerLifeTime == 0) {
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime));
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::ScaleRandom));
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Particle Rotate")) {
        ImGui::DragFloat3("##ParticleRotateMin", startParticleRotateMin_.v, 0.1f);
        ImGui::DragFloat3("##ParticleRotateMax", startParticleRotateMax_.v, 0.1f);

        startParticleRotateMin_ = (std::min)(startParticleRotateMin_, startParticleRotateMax_);
        startParticleRotateMax_ = (std::max)(startParticleRotateMin_, startParticleRotateMax_);

        int randomOrPerLifeTime    = (updateSettings_ & static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime)) ? 2 : ((updateSettings_ & static_cast<int32_t>(ParticleUpdateType::RotateRandom)) ? 1 : 0);
        int preRandomOrPerLifeTime = randomOrPerLifeTime;
        ImGui::RadioButton("Update Rotate None", &randomOrPerLifeTime, 0);
        ImGui::RadioButton("Update Rotate Random", &randomOrPerLifeTime, 1);
        ImGui::RadioButton("Update Rotate PerLifeTime", &randomOrPerLifeTime, 2);
        if (randomOrPerLifeTime == 2) {
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::RotateRandom));

            particleKeyFrames_->rotateCurve_[0].value = startParticleRotateMax_;
            ImGui::EditKeyFrame("RotateLine", particleKeyFrames_->rotateCurve_, particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            // ランダムな回転を設定
            ImGui::Text("UpdateMin");
            ImGui::DragFloat3("##UpdateParticleRotateMin", updateParticleRotateMin_.v, 0.1f);
            ImGui::Text("UpdateMax");
            ImGui::DragFloat3("##UpdateParticleRotateMax", updateParticleRotateMax_.v, 0.1f);

            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::RotateRandom));
        } else if (preRandomOrPerLifeTime != 0 && randomOrPerLifeTime == 0) {
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::RotateRandom));
        }
        ImGui::TreePop();
    }

    ImGui::Combo("UvInterpolationType", reinterpret_cast<int*>(&uvInterpolationType_), "LINEAR\0STEP\0\0", static_cast<int>(InterpolationType::COUNT));

    ImGui::Spacing();

    if (ImGui::TreeNode("UvCurveGenerator Form TextureAnimation")) {
        ImGui::DragFloat2("TileSize", tileSize_.v, 0.1f);
        ImGui::DragFloat2("TextureSize", textureSize_.v, 0.1f);
        ImGui::DragFloat("tilePerTime_", &tilePerTime_);
        ImGui::DragFloat("StartAnimationTime", &startAnimationTime_, 0.1f, 0);
        ImGui::DragFloat("AnimationTimeLength", &animationTimeLength_, 0.1f, 0);
        if (ImGui::Button("Generate Curve")) {
            if (particleKeyFrames_) {
                particleLifeTime_ = animationTimeLength_;

                particleKeyFrames_->uvScaleCurve_.clear();
                particleKeyFrames_->uvTranslateCurve_.clear();

                // uvScale は Animation しない
                updateSettings_  = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));
                particleUvScale_ = Vector3f(tileSize_ / textureSize_, 0.f);

                // uv Translate は Animation する
                updateSettings_      = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));
                uvInterpolationType_ = InterpolationType::STEP;

                // 最大タイル数と最大時間を計算
                int32_t maxTilesX = int32_t(textureSize_[X] / tileSize_[X]);
                int32_t maxTilesY = int32_t(textureSize_[Y] / tileSize_[Y]);
                int32_t maxTiles  = maxTilesX * maxTilesY;
                float maxTime     = maxTiles * tilePerTime_;

                // startAnimationTime_ を最大時間内に収める
                startAnimationTime_ = fmod(startAnimationTime_, maxTime);

                // 初期の col と row を計算
                int32_t startTileIndex = int32_t(startAnimationTime_ / tilePerTime_);
                float col              = float(startTileIndex % maxTilesX);
                float row              = float(startTileIndex / maxTilesX);

                // UV座標を計算
                float x = col * (tileSize_[X] / textureSize_[X]);
                float y = row * (tileSize_[Y] / textureSize_[Y]);

                particleUvTranslate_ = Vector3f(x, y, 0.f);

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
                    particleKeyFrames_->uvTranslateCurve_.emplace_back(time, Vector3f(x, y, 0.f));
                }
            }
        }

        ImGui::TreePop();
    }

    ImGui::Spacing();

    if (ImGui::TreeNode("Particle UV Scale")) {
        ImGui::DragFloat3("##ParticleUvScale", particleUvScale_.v, 0.1f);
        bool updatePerLifeTime    = (updateSettings_ & static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;
        ImGui::Checkbox("Update UvScale PerLifeTime", &updatePerLifeTime);
        if (updatePerLifeTime) {
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));

            particleKeyFrames_->uvScaleCurve_[0].value = particleUvScale_;
            ImGui::EditKeyFrame("UvScaleLine", particleKeyFrames_->uvScaleCurve_, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            particleKeyFrames_->uvScaleCurve_.clear();
            particleKeyFrames_->uvScaleCurve_.emplace_back(0.0f, particleUvScale_);
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Particle UV Rotate")) {
        ImGui::DragFloat3("##ParticleUvRotate", particleUvRotate_.v, 0.1f);
        bool updatePerLifeTime    = (updateSettings_ & static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;
        ImGui::Checkbox("Update UvRotate PerLifeTime", &updatePerLifeTime);
        if (updatePerLifeTime) {
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime));

            particleKeyFrames_->uvRotateCurve_[0].value = particleUvRotate_;
            ImGui::EditKeyFrame("UvRotateLine", particleKeyFrames_->uvRotateCurve_, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            particleKeyFrames_->uvRotateCurve_.clear();
            particleKeyFrames_->uvRotateCurve_.emplace_back(0.0f, particleUvRotate_);
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::UvRotatePerLifeTime));
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Particle UV Translate")) {
        ImGui::DragFloat3("##ParticleUvTranslate", particleUvTranslate_.v, 0.1f);
        bool updatePerLifeTime    = (updateSettings_ & static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;
        ImGui::Checkbox("Update UvTransform PerLifeTime", &updatePerLifeTime);
        if (updatePerLifeTime) {
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));

            particleKeyFrames_->uvTranslateCurve_[0].value = particleUvTranslate_;
            ImGui::EditKeyFrame("UvTranslateLine", particleKeyFrames_->uvTranslateCurve_, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            particleKeyFrames_->uvTranslateCurve_.clear();
            particleKeyFrames_->uvTranslateCurve_.emplace_back(0.0f, particleUvTranslate_);
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));
        }
        ImGui::TreePop();
    }
}
#endif // _DEBUG

void Emitter::Draw(ID3D12GraphicsCommandList* _commandList) {
    if (!particleModel_ || particleModel_->meshData_->currentState_ != LoadState::Loaded) {
        return;
    }

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

    ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
    _commandList->SetDescriptorHeaps(1, ppHeaps);

    int32_t meshIndex = 0;
    for (auto& [meshName, mesh] : particleModel_->meshData_->meshGroup_) {

        auto& material = particleModel_->materialData_[meshIndex];
        _commandList->SetGraphicsRootDescriptorTable(
            3,
            TextureManager::getDescriptorGpuHandle(textureIndex_));

        _commandList->IASetVertexBuffers(0, 1, &mesh.getVBView());
        _commandList->IASetIndexBuffer(&mesh.getIBView());

        structuredTransform_.SetForRootParameter(_commandList, 0);

        material.material.SetForRootParameter(_commandList, 2);
        // 描画!!!
        _commandList->DrawIndexedInstanced(UINT(mesh.getIndexSize()), static_cast<UINT>(structuredTransform_.openData_.size()), 0, 0, 0);
    }
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
