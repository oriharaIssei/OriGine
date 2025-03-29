#include "Emitter.h"

#include "effect/manager/EffectManager.h"

// stl
// container
#include <array>

/// engine
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
#include <cmath>

#ifdef _DEBUG
#include "animationEditor/Timeline.h"
#include "imgui/imgui.h"
#endif // _DEBUG

Emitter::Emitter(DxSrvArray* _srvArray) : IComponent(), srvArray_(_srvArray) {
    isActive_       = false;
    leftActiveTime_ = 0.0f;
}

Emitter::~Emitter() {
    if (!particles_.empty()) {
        particles_.clear();
    }
    structuredTransform_.Finalize();
}

static std::list<std::pair<std::string, std::string>> SearchModelFile() {
    std::list<std::pair<std::string, std::string>> modelFiles = MyFileSystem::SearchFile(kEngineResourceDirectory, "obj", false);
    modelFiles.merge(MyFileSystem::SearchFile(kEngineResourceDirectory, "gltf", false));
    modelFiles.merge(MyFileSystem::SearchFile(kApplicationResourceDirectory, "obj", false));
    modelFiles.merge(MyFileSystem::SearchFile(kApplicationResourceDirectory, "gltf", false));
    return modelFiles;
}
static std::list<std::pair<std::string, std::string>> SearchTextureFile() {
    std::list<std::pair<std::string, std::string>> texutreFiles = MyFileSystem::SearchFile(kEngineResourceDirectory, "png", false);
    texutreFiles.merge(MyFileSystem::SearchFile(kApplicationResourceDirectory, "png", false));
    return texutreFiles;
}
static std::list<std::pair<std::string, std::string>> objectFiles  = SearchModelFile();
static std::list<std::pair<std::string, std::string>> textureFiles = SearchTextureFile();

void Emitter::Initialize(GameEntity* _entity) {
    { // Initialize DrawingData Size
        CalculateMaxSize();
        structuredTransform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice(), srvArray_, particleMaxSize_);
        particles_.reserve(particleMaxSize_);
    }

    { // Initialize Active State
        isActive_       = true;
        leftActiveTime_ = activeTime_;
    }

    particleKeyFrames_ = std::make_unique<ParticleKeyFrames>();
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

    { // Particles Update
        for (auto& particle : particles_) {
            particle->Update(deltaTime);
        }
        // isAliveでないもの は 消す
        std::erase_if(particles_, [](std::unique_ptr<Particle>& particle) {
            return !particle->getIsAlive();
        });
    }

    { // Update Spawn
        currentCoolTime_ -= deltaTime;
        if (currentCoolTime_ <= 0.0f) {
            currentCoolTime_ = spawnCoolTime_;
            SpawnParticle();
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

    std::string preDataName = emitterDataName_;
    if (ImGui::InputText("##emitterName", &emitterDataName_[0], sizeof(char*) * 64)) {
        GlobalVariables::getInstance()->ChangeGroupName("Emitters", preDataName, emitterDataName_);
        if (particleKeyFrames_) {
        }
    }

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
                particleModel_ = ModelManager::getInstance()->Create(fileName.first, fileName.second);
                modelFileName_ = fileName.first + "/" + fileName.second;
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

    static float changingSrvSizeLeftTime       = 0.f;
    static const float changingSrvSizeInterval = 2.f;
    changingSrvSizeLeftTime -= 0.16;
    if (changingSrvSizeLeftTime < 0.0f) {
        CalculateMaxSize();
        if (structuredTransform_.capacity() <= particleMaxSize_) {
            structuredTransform_.resize(Engine::getInstance()->getDxDevice()->getDevice(), particleMaxSize_ * 2);
            changingSrvSizeLeftTime = changingSrvSizeInterval;
        }
    }
#endif // _DEBUG
}
void Emitter::Save(BinaryWriter& _writer) {
    _writer.Write(particleKeyFrameFileName_);

    _writer.Write(isActive_);
    _writer.Write(isLoop_);

    _writer.Write(activeTime_);
    _writer.Write(leftActiveTime_);
    _writer.Write(spawnParticleVal_);
    _writer.Write(spawnCoolTime_);
    _writer.Write(currentCoolTime_);
    _writer.Write(particleMaxSize_);

    _writer.Write(modelFileName_);
    _writer.Write(textureFileName_);

    _writer.Write(static_cast<int32_t>(blendMode_));
    _writer.Write(particleIsBillBoard_);
    _writer.Write(static_cast<int32_t>(shapeType_));
}

void Emitter::Load(BinaryReader& _reader) {
    _reader.Read(particleKeyFrameFileName_);
    if (!particleKeyFrameFileName_.empty()) {
        particleKeyFrames_->Load(particleKeyFrameFileName_);
    }

    _reader.Read(isActive_);
    _reader.Read(isLoop_);
    _reader.Read(activeTime_);
    _reader.Read(leftActiveTime_);
    _reader.Read(spawnParticleVal_);
    _reader.Read(spawnCoolTime_);
    _reader.Read(currentCoolTime_);
    _reader.Read(particleMaxSize_);

    _reader.Read(modelFileName_);
    _reader.Read(textureFileName_);
    _reader.Read(textureIndex_);
    int32_t blendMode;
    _reader.Read(blendMode);
    blendMode_ = BlendMode(blendMode);
    _reader.Read(particleIsBillBoard_);
    int32_t shapeType;
    _reader.Read(shapeType);
    shapeType_ = EmitterShapeType(shapeType);

}

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
                switch (shapeType_) {
                case EmitterShapeType::SPHERE:
                    emitterSpawnShape_ = std::make_unique<EmitterSphere>("Emitters", emitterDataName_);
                    break;
                case EmitterShapeType::OBB:
                    emitterSpawnShape_ = std::make_unique<EmitterOBB>("Emitters", emitterDataName_);
                    break;
                case EmitterShapeType::Capsule:
                    emitterSpawnShape_ = std::make_unique<EmitterCapsule>("Emitters", emitterDataName_);
                    break;
                case EmitterShapeType::Cone:
                    emitterSpawnShape_ = std::make_unique<EmitterCone>("Emitters", emitterDataName_);
                    break;
                default:
                    break;
                }

                shapeType_ = EmitterShapeType(i);
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
        ImGui::ColorEdit4("##Particle Color", particleColor_.v);
        // curveで変更するかどうか
        bool updatePerLifeTime    = (updateSettings_ & static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;
        ImGui::Checkbox("UpdateColorPerLifeTime", &updatePerLifeTime);
        if (updatePerLifeTime) {
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime));

            particleKeyFrames_->colorCurve_[0].value = particleColor_;
            ImGui::EditKeyFrame(emitterDataName_ + "ColorLine", particleKeyFrames_->colorCurve_, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::ColorPerLifeTime));
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Particle Velocity")) {
        ImGui::Text("Min");
        ImGui::DragFloat3("##ParticleVelocityMin", startParticleVelocityMin_.v, 0.1f);
        ImGui::Text("Max");
        ImGui::DragFloat3("##ParticleVelocityMax", startParticleVelocityMax_.v, 0.1f);

        int randomOrPerLifeTime    = (updateSettings_ & static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime)) ? 2 : ((updateSettings_ & static_cast<int32_t>(ParticleUpdateType::VelocityRandom)) ? 1 : 0);
        int preRandomOrPerLifeTime = randomOrPerLifeTime;
        ImGui::RadioButton("Update Velocity None", &randomOrPerLifeTime, 0);
        ImGui::RadioButton("Update Velocity Random", &randomOrPerLifeTime, 1);
        ImGui::RadioButton("Update Velocity PerLifeTime", &randomOrPerLifeTime, 2);
        if (randomOrPerLifeTime == 2) {
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime));
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::VelocityRandom));

            particleKeyFrames_->velocityCurve_[0].value = startParticleVelocityMax_;
            ImGui::EditKeyFrame(emitterDataName_ + "SpeedLine", particleKeyFrames_->velocityCurve_, particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            // ランダムな速度を設定
            ImGui::Text("UpdateMin");
            ImGui::DragFloat3("##UpdateParticleVelocityMin", updateParticleVelocityMin_.v, 0.1f);
            ImGui::Text("UpdateMax");
            ImGui::DragFloat3("##UpdateParticleVelocityMax", updateParticleVelocityMax_.v, 0.1f);

            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::VelocityPerLifeTime));
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::VelocityRandom));
        } else if (preRandomOrPerLifeTime == 2 && randomOrPerLifeTime == 0) {
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
            ImGui::EditKeyFrame(emitterDataName_ + "ScaleLine", particleKeyFrames_->scaleCurve_, particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            // ランダムなScaleを設定
            ImGui::Text("UpdateMin");
            ImGui::DragFloat3("##UpdateParticleScaleMin", updateParticleScaleMin_.v, 0.1f);
            ImGui::Text("UpdateMax");
            ImGui::DragFloat3("##UpdateParticleScaleMax", updateParticleScaleMax_.v, 0.1f);

            // ランダムなスケールを設定
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime));
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::ScaleRandom));
        } else if (preRandomOrPerLifeTime == 2 && randomOrPerLifeTime == 0) {
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::ScalePerLifeTime));
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::ScaleRandom));
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Particle Rotate")) {
        ImGui::DragFloat3("##ParticleRotateMin", startParticleRotateMin_.v, 0.1f);
        ImGui::DragFloat3("##ParticleRotateMax", startParticleRotateMax_.v, 0.1f);
        int randomOrPerLifeTime    = (updateSettings_ & static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime)) ? 2 : ((updateSettings_ & static_cast<int32_t>(ParticleUpdateType::RotateRandom)) ? 1 : 0);
        int preRandomOrPerLifeTime = randomOrPerLifeTime;
        ImGui::RadioButton("Update Rotate None", &randomOrPerLifeTime, 0);
        ImGui::RadioButton("Update Rotate Random", &randomOrPerLifeTime, 1);
        ImGui::RadioButton("Update Rotate PerLifeTime", &randomOrPerLifeTime, 2);
        if (randomOrPerLifeTime == 2) {
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::RotateRandom));

            particleKeyFrames_->rotateCurve_[0].value = startParticleRotateMax_;
            ImGui::EditKeyFrame(emitterDataName_ + "RotateLine", particleKeyFrames_->rotateCurve_, particleLifeTime_);
        } else if (randomOrPerLifeTime == 1) {
            // ランダムな回転を設定
            ImGui::Text("UpdateMin");
            ImGui::DragFloat3("##UpdateParticleRotateMin", updateParticleRotateMin_.v, 0.1f);
            ImGui::Text("UpdateMax");
            ImGui::DragFloat3("##UpdateParticleRotateMax", updateParticleRotateMax_.v, 0.1f);

            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::RotateRandom));
        } else if (preRandomOrPerLifeTime == 2 && randomOrPerLifeTime == 0) {
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::RotatePerLifeTime));
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::RotateRandom));
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Particle UV Scale")) {
        ImGui::DragFloat3("##ParticleUvScale", particleUvScale_.v, 0.1f);
        bool updatePerLifeTime    = (updateSettings_ & static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime)) != 0;
        bool preUpdatePerLifeTime = updatePerLifeTime;
        ImGui::Checkbox("Update UvScale PerLifeTime", &updatePerLifeTime);
        if (updatePerLifeTime) {
            updateSettings_ = (updateSettings_ | static_cast<int32_t>(ParticleUpdateType::UvScalePerLifeTime));

            particleKeyFrames_->uvScaleCurve_[0].value = particleUvScale_;
            ImGui::EditKeyFrame(emitterDataName_ + "UvScaleLine", particleKeyFrames_->uvScaleCurve_, particleLifeTime_);
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
            ImGui::EditKeyFrame(emitterDataName_ + "UvRotateLine", particleKeyFrames_->uvRotateCurve_, particleLifeTime_);
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
            ImGui::EditKeyFrame(emitterDataName_ + "UvTranslateLine", particleKeyFrames_->uvTranslateCurve_, particleLifeTime_);
        } else if (preUpdatePerLifeTime && !updatePerLifeTime) {
            particleKeyFrames_->uvTranslateCurve_.clear();
            particleKeyFrames_->uvTranslateCurve_.emplace_back(0.0f, particleUvTranslate_);
            updateSettings_ = (updateSettings_ & ~static_cast<int32_t>(ParticleUpdateType::UvTranslatePerLifeTime));
        }
        ImGui::TreePop();
    }
}

void Emitter::Draw(ID3D12GraphicsCommandList* _commandList) {
    if (!particleModel_ || particleModel_->meshData_->currentState_ != LoadState::Loaded) {
        return;
    }

    EffectManager::getInstance()->ChangeBlendMode(blendMode_);

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
        }
    }

    structuredTransform_.ConvertToBuffer();

    auto* commandList               = EffectManager::getInstance()->dxCommand_->getCommandList();
    ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
    commandList->SetDescriptorHeaps(1, ppHeaps);

    auto& model    = particleModel_->meshData_->meshGroup_[0];
    auto& material = particleModel_->materialData_[0];
    commandList->SetGraphicsRootDescriptorTable(
        3,
        TextureManager::getDescriptorGpuHandle(textureIndex_));

    commandList->IASetVertexBuffers(0, 1, &model.vbView);
    commandList->IASetIndexBuffer(&model.ibView);

    structuredTransform_.SetForRootParameter(commandList, 0);

    material.material->SetForRootParameter(commandList, 2);
    // 描画!!!
    commandList->DrawIndexedInstanced(UINT(model.indexSize), static_cast<UINT>(structuredTransform_.openData_.size()), 0, 0, 0);
}

void Emitter::Finalize() {
    if (!particles_.empty()) {
        particles_.clear();
    }
    structuredTransform_.Finalize();
}

void Emitter::CalculateMaxSize() {
    // 1秒あたりの生成回数
    float spawnRatePerSecond = spawnParticleVal_ / spawnCoolTime_;

    // 最大個数
    particleMaxSize_ = (std::max<uint32_t>)(static_cast<uint32_t>(std::ceil(spawnRatePerSecond * particleLifeTime_)), spawnParticleVal_);
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
            velocity);

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
