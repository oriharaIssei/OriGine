#include "ParticleSystem.h"

// stl
// container
#include <array>

/// engine
#include "Engine.h"
#define RESOURCE_DIRECTORY
#include "directX12/DxDevice.h"
#include "EngineInclude.h"
#include "scene/Scene.h"
// asset
#include "asset/TextureAsset.h"
// component
#include "component/material/Material.h"
#include "component/renderer/primitive/shape/Plane.h"
// module
#include "asset/AssetSystem.h"
#include "camera/CameraManager.h"
#include "directX12/ShaderManager.h"
#include "globalVariables/GlobalVariables.h"
#include "model/ModelManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "myRandom/MyRandom.h"
// assets
#include "EmitterShape.h"
#include "model/Model.h"

#include "logger/Logger.h"

// math
#include "math/Matrix4x4.h"
#include <cmath>

#ifdef _DEBUG
#include "ParticleSystemEditor.h"
#endif // _DEBUG

using namespace OriGine;

ParticleSystem::ParticleSystem() {}
ParticleSystem::~ParticleSystem() {}

void ParticleSystem::Initialize(Scene* _scene, EntityHandle /*_entity*/) {
    emitter_.Initialize();
    emitter_.ResolveParent(_scene);

    CalculateMaxSize();
    particles_.reserve(particleMaxSize_);

    if (emitter_.isActive_) {
        CreateResource();
    }

    if (!textureFileName_.empty()) {
        textureIndex_ = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(textureFileName_);
    }

    if (!particleKeyFrames_) {
        particleKeyFrames_ = std::make_shared<ParticleKeyFrames>();
    }
}

void ParticleSystem::Finalize() {
    if (!particles_.empty()) {
        particles_.clear();
    }
    structuredTransform_.Finalize();
}

void ParticleSystem::CreateResource() {
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

void ParticleSystem::CalculateMaxSize() {
    // 1秒あたりの生成回数
    float spawnRatePerSecond = emitter_.spawnCount_ / emitter_.spawnCoolTime_;

    // 最大個数
    particleMaxSize_ = (std::max<uint32_t>)((std::max<uint32_t>)(static_cast<uint32_t>(std::ceil(spawnRatePerSecond * particleLifeTime_)), static_cast<uint32_t>(emitter_.spawnCount_)), particleMaxSize_);
}

void ParticleSystem::SpawnParticle(int32_t _spawnVal) {
    // スポーンして良い数
    int32_t canSpawnParticleValue_ = (std::min<int32_t>)(_spawnVal, static_cast<int32_t>(particleMaxSize_ - particles_.size()));

    emitter_.UpdateWorldOriginPos();

    bool uniformScaleRandom = (updateSettings_ & int(ParticleUpdateType::UniformScaleRandom)) != 0;

    for (int32_t i = 0; i < canSpawnParticleValue_; i++) {
        Vec3f spawnPos = emitter_.GetInterpolatedOriginPos(i, canSpawnParticleValue_) + emitter_.GetSpawnPos();

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
            Vec3f(transform.translate - emitter_.originPos_).normalize(),
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

void ParticleSystem::PlayStart() {
    emitter_.PlayStart();
    CreateResource();
}

void ParticleSystem::PlayContinue() {
    emitter_.PlayContinue();
}

void ParticleSystem::PlayStop() {
    emitter_.PlayStop();
}

void ParticleSystem::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    ParticleSystemEditor::Draw(*this, _scene, _entity, _parentLabel);
#endif // _DEBUG
}

void OriGine::from_json(const nlohmann::json& _j, ParticleSystem& _comp) {
    _j.at("blendMode").get_to(_comp.blendMode_);
    _j.at("textureFileName").get_to(_comp.textureFileName_);
    _j.at("particleLifeTime").get_to(_comp.particleLifeTime_);
    _j.at("particleIsBillBoard").get_to(_comp.particleIsBillBoard_);

    _j.at("emitter").get_to(_comp.emitter_);

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
}

void OriGine::to_json(nlohmann::json& _j, const ParticleSystem& _comp) {
    _j = nlohmann::json{
        {"blendMode", _comp.blendMode_},
        {"textureFileName", _comp.textureFileName_},
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

    _j["emitter"] = _comp.emitter_;

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
        LOG_ERROR("particleKeyFrames_ is nullptr. Please check the ParticleSystem class.");
    }
}
