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
// assets
#include "EmitterShape.h"
#include "model/Model.h"

#include "logger/Logger.h"

// math
#include "math/Matrix4x4.h"
#include <cmath>

#ifdef _DEBUG
#include "EmitterEditor.h"
#endif // _DEBUG

using namespace OriGine;

Emitter::Emitter() : IComponent(), currentCoolTime_(0.f), leftActiveTime_(0.f) {
    isActive_       = false;
    leftActiveTime_ = 0.0f;
}

Emitter::~Emitter() {}

void Emitter::Initialize(Scene* /*_scene*/, EntityHandle /*_entity*/) {
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

void Emitter::Update(float _deltaTime) {
    { // Update Active
        if (!isActive_) {
            return;
        }

        // Loop するなら スキップ
        if (!isLoop_) {
            leftActiveTime_ -= _deltaTime;
            // leftActiveTime が 0 以下で Particle が 全て消えたら
            if (leftActiveTime_ <= 0.0f && particles_.empty()) {
                isActive_ = false;
                return;
            }
        }
    }

    UpdateParticle(_deltaTime);
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
    float spawnRatePerSecond = spawnParticleVal_ / spawnCoolTime_;

    // 最大個数
    particleMaxSize_ = (std::max<uint32_t>)((std::max<uint32_t>)(static_cast<uint32_t>(std::ceil(spawnRatePerSecond * particleLifeTime_)), spawnParticleVal_), particleMaxSize_);
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

void Emitter::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    EmitterEditor::Draw(*this, _scene, _entity, _parentLabel);
#endif // _DEBUG
}

void Emitter::Draw(const Matrix4x4& _viewMat, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList) {

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
        Matrix4x4 cameraRotation = _viewMat;
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

void OriGine::from_json(const nlohmann::json& _j, Emitter& _comp) {
    _j.at("blendMode").get_to(_comp.blendMode_);

    _j.at("isActive").get_to(_comp.isActive_);
    _j.at("isLoop").get_to(_comp.isLoop_);

    _j.at("originPos").get_to(_comp.originPos_);

    _j.at("textureFileName").get_to(_comp.textureFileName_);

    _j.at("activeTime").get_to(_comp.activeTime_);
    _j.at("spawnParticleVal").get_to(_comp.spawnParticleVal_);
    _j.at("shapeType").get_to(_comp.shapeType_);
    _j.at("spawnCoolTime").get_to(_comp.spawnCoolTime_);

    _j.at("particleLifeTime").get_to(_comp.particleLifeTime_);
    _j.at("particleIsBillBoard").get_to(_comp.particleIsBillBoard_);

    _j.at("particleColor").get_to(_comp.particleColor_);
    _j.at("particleUvScale").get_to(_comp.particleUvScale_);
    _j.at("particleUvRotate").get_to(_comp.particleUvRotate_);
    _j.at("particleUvTranslate").get_to(_comp.particleUvTranslate_);
    _j.at("updateSettings").get_to(_comp.updateSettings_);
    _j.at("startParticleScaleMin").get_to(_comp.startParticleScaleMin_);
    _j.at("startParticleScaleMax").get_to(_comp.startParticleScaleMax_);
    _j.at("startParticleRotateMin").get_to(_comp.startParticleRotateMin_);
    _j.at("startParticleRotateMax").get_to(_comp.startParticleRotateMax_);
    _j.at("startParticleVelocityMin").get_to(_comp.startParticleVelocityMin_);
    _j.at("startParticleVelocityMax").get_to(_comp.startParticleVelocityMax_);
    _j.at("updateParticleScaleMin").get_to(_comp.updateParticleScaleMin_);
    _j.at("updateParticleScaleMax").get_to(_comp.updateParticleScaleMax_);
    _j.at("updateParticleRotateMin").get_to(_comp.updateParticleRotateMin_);
    _j.at("updateParticleRotateMax").get_to(_comp.updateParticleRotateMax_);
    _j.at("updateParticleVelocityMin").get_to(_comp.updateParticleVelocityMin_);
    _j.at("updateParticleVelocityMax").get_to(_comp.updateParticleVelocityMax_);
    _j.at("randMass").get_to(_comp.randMass_);

    auto curveLoad = [](const nlohmann::json& _curveJson, auto& _curve) {
        for (auto& keyframeJson : _curveJson) {
            typename std::remove_reference<decltype(_curve)>::type::value_type key;
            keyframeJson.at("time").get_to(key.time);
            keyframeJson.at("value").get_to(key.value);
            _curve.push_back(key);
        }
    };

    if (!_comp.particleKeyFrames_) {
        _comp.particleKeyFrames_ = std::make_shared<ParticleKeyFrames>();
    }

    _j.at("transformInterpolationType").get_to(_comp.transformInterpolationType_);
    _j.at("colorInterpolationType").get_to(_comp.colorInterpolationType_);
    _j.at("uvInterpolationType").get_to(_comp.uvInterpolationType_);

    if (_j.find("scaleCurve") != _j.end()) {
        curveLoad(_j.at("scaleCurve"), _comp.particleKeyFrames_->scaleCurve);
    }
    if (_j.find("rotateCurve") != _j.end()) {
        curveLoad(_j.at("rotateCurve"), _comp.particleKeyFrames_->rotateCurve);
    }
    if (_j.find("velocityCurve") != _j.end()) {
        curveLoad(_j.at("velocityCurve"), _comp.particleKeyFrames_->velocityCurve);
    }

    if (_j.find("colorCurve") != _j.end()) {
        curveLoad(_j.at("colorCurve"), _comp.particleKeyFrames_->colorCurve);
    }

    if (_j.find("uvScaleCurve") != _j.end()) {
        curveLoad(_j.at("uvScaleCurve"), _comp.particleKeyFrames_->uvScaleCurve);
    }
    if (_j.find("uvRotateCurve") != _j.end()) {
        curveLoad(_j.at("uvRotateCurve"), _comp.particleKeyFrames_->uvRotateCurve);
    }
    if (_j.find("uvTranslateCurve") != _j.end()) {
        curveLoad(_j.at("uvTranslateCurve"), _comp.particleKeyFrames_->uvTranslateCurve);
    }

    if (_j.find("EmitterShape") != _j.end()) {
        nlohmann::json shapeJson = _j["EmitterShape"];
        switch (_comp.shapeType_) {
        case EmitterShapeType::SPHERE: {
            EmitterSphere sphereShape;
            shapeJson.get_to(sphereShape);
            _comp.emitterSpawnShape_ = std::make_shared<EmitterSphere>(sphereShape);
            break;
        }
        case EmitterShapeType::BOX: {
            EmitterBox obbShape;
            shapeJson.get_to(obbShape);
            _comp.emitterSpawnShape_ = std::make_shared<EmitterBox>(obbShape);
            break;
        }
        case EmitterShapeType::CAPSULE: {
            EmitterCapsule capsuleShape;
            shapeJson.get_to(capsuleShape);
            _comp.emitterSpawnShape_ = std::make_shared<EmitterCapsule>(capsuleShape);
            break;
        }
        case EmitterShapeType::CONE: {
            EmitterCone coneShape;
            shapeJson.get_to(coneShape);
            _comp.emitterSpawnShape_ = std::make_shared<EmitterCone>(coneShape);
            break;
        }
        case EmitterShapeType::Count:
            LOG_ERROR("EmitterShapeType is not defined. Please check the EmitterShapeType.");
            break;
        default:
            break;
        }
    } else {
        _comp.emitterSpawnShape_ = std::make_shared<EmitterSphere>();
    }
}

void OriGine::to_json(nlohmann::json& _j, const Emitter& _comp) {
    _j = nlohmann::json{
        {"blendMode", _comp.blendMode_},
        {"isActive", _comp.isActive_},
        {"isLoop", _comp.isLoop_},
        {"originPos", _comp.originPos_},
        {"activeTime", _comp.activeTime_},
        {"spawnParticleVal", _comp.spawnParticleVal_},
        {"shapeType", _comp.shapeType_},
        {"textureFileName", _comp.textureFileName_},
        {"spawnCoolTime", _comp.spawnCoolTime_},
        {"particleLifeTime", _comp.particleLifeTime_},
        {"particleIsBillBoard", _comp.particleIsBillBoard_},
        {"particleColor", _comp.particleColor_},
        {"particleUvScale", _comp.particleUvScale_},
        {"particleUvRotate", _comp.particleUvRotate_},
        {"particleUvTranslate", _comp.particleUvTranslate_},
        {"updateSettings", _comp.updateSettings_},
        {"startParticleScaleMin", _comp.startParticleScaleMin_},
        {"startParticleScaleMax", _comp.startParticleScaleMax_},
        {"startParticleRotateMin", _comp.startParticleRotateMin_},
        {"startParticleRotateMax", _comp.startParticleRotateMax_},
        {"startParticleVelocityMin", _comp.startParticleVelocityMin_},
        {"startParticleVelocityMax", _comp.startParticleVelocityMax_},
        {"updateParticleScaleMin", _comp.updateParticleScaleMin_},
        {"updateParticleScaleMax", _comp.updateParticleScaleMax_},
        {"updateParticleRotateMin", _comp.updateParticleRotateMin_},
        {"updateParticleRotateMax", _comp.updateParticleRotateMax_},
        {"updateParticleVelocityMin", _comp.updateParticleVelocityMin_},
        {"updateParticleVelocityMax", _comp.updateParticleVelocityMax_},
        {"randMass", _comp.randMass_}};

    nlohmann::json shapeJson = nlohmann::json::object();
    if (_comp.emitterSpawnShape_) {
        switch (_comp.emitterSpawnShape_->type) {
        case EmitterShapeType::SPHERE: {
            EmitterSphere* sphereShape = dynamic_cast<EmitterSphere*>(_comp.emitterSpawnShape_.get());
            if (sphereShape) {
                shapeJson = *sphereShape;
            } else {
                LOG_ERROR("EmitterSphere is not Sphere type. Please check the emitterSpawnShape_ type.");
            }
            break;
        }
        case EmitterShapeType::BOX: {
            EmitterBox* obbShape = dynamic_cast<EmitterBox*>(_comp.emitterSpawnShape_.get());

            if (obbShape) {
                shapeJson = *obbShape;
            } else {
                LOG_ERROR("EmitterBox is not OBB type. Please check the emitterSpawnShape_ type.");
            }

            break;
        }
        case EmitterShapeType::CAPSULE: {
            EmitterCapsule* capsuleShape = dynamic_cast<EmitterCapsule*>(_comp.emitterSpawnShape_.get());

            if (capsuleShape) {
                shapeJson = *capsuleShape;
            } else {
                LOG_ERROR("EmitterCapsule is not Capsule type. Please check the emitterSpawnShape_ type.");
            }

            break;
        }
        case EmitterShapeType::CONE: {

            EmitterCone* coneShape = dynamic_cast<EmitterCone*>(_comp.emitterSpawnShape_.get());
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
    _j["EmitterShape"] = shapeJson;

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

    _j["transformInterpolationType"] = _comp.transformInterpolationType_;
    _j["colorInterpolationType"]     = _comp.colorInterpolationType_;
    _j["uvInterpolationType"]        = _comp.uvInterpolationType_;

    if (_comp.particleKeyFrames_) {
        if ((_comp.updateSettings_ & (int32_t)ParticleUpdateType::ScalePerLifeTime) != 0) {
            _j["scaleCurve"] = curveSave(_comp.particleKeyFrames_->scaleCurve);
        }
        if ((_comp.updateSettings_ & (int32_t)ParticleUpdateType::RotatePerLifeTime) != 0) {
            _j["rotateCurve"] = curveSave(_comp.particleKeyFrames_->rotateCurve);
        }
        if ((_comp.updateSettings_ & (int32_t)ParticleUpdateType::VelocityPerLifeTime) != 0) {
            _j["velocityCurve"] = curveSave(_comp.particleKeyFrames_->velocityCurve);
        }

        if ((_comp.updateSettings_ & (int32_t)ParticleUpdateType::ColorPerLifeTime) != 0) {
            _j["colorCurve"] = curveSave(_comp.particleKeyFrames_->colorCurve);
        }

        if ((_comp.updateSettings_ & (int32_t)ParticleUpdateType::UvScalePerLifeTime) != 0) {
            _j["uvScaleCurve"] = curveSave(_comp.particleKeyFrames_->uvScaleCurve);
        }
        if ((_comp.updateSettings_ & (int32_t)ParticleUpdateType::UvRotatePerLifeTime) != 0) {
            _j["uvRotateCurve"] = curveSave(_comp.particleKeyFrames_->uvRotateCurve);
        }
        if ((_comp.updateSettings_ & (int32_t)ParticleUpdateType::UvTranslatePerLifeTime) != 0) {
            _j["uvTranslateCurve"] = curveSave(_comp.particleKeyFrames_->uvTranslateCurve);
        }
    } else {
        LOG_ERROR("particleKeyFrames_ is nullptr. Please check the Emitter class.");
    }
}
