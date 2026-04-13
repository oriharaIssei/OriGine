#include "Emitter.h"

#include "component/transform/Transform.h"
#include "scene/Scene.h"
#include "logger/Logger.h"

using namespace OriGine;

Emitter::Emitter() {
    EnsureShape();
}

void Emitter::Initialize() {
    leftActiveTime_  = activeTime_;
    currentCoolTime_ = 0.f;
    EnsureShape();
}

void Emitter::EnsureShape() {
    if (!spawnShape_) {
        spawnShape_ = std::make_shared<EmitterSphere>();
    }
}

int32_t Emitter::Update(float _deltaTime) {
    if (!isActive_) {
        return 0;
    }

    if (!isLoop_) {
        leftActiveTime_ -= _deltaTime;
    }

    if (isLoop_ || leftActiveTime_ > 0.f) {
        currentCoolTime_ -= _deltaTime;
        if (currentCoolTime_ <= 0.f && spawnCoolTime_ > 0.f) {
            currentCoolTime_ = spawnCoolTime_ / static_cast<float>(spawnCount_);
            return (std::max)(1, static_cast<int32_t>(_deltaTime / currentCoolTime_));
        }
    }

    return 0;
}

Vec3f Emitter::GetSpawnPos() const {
    if (spawnShape_) {
        return spawnShape_->GetSpawnPos();
    }
    return {};
}

Vec3f Emitter::GetInterpolatedOriginPos(int32_t _index, int32_t _total) const {
    if (!interpolateSpawnPos_ || _total <= 0) {
        return worldOriginPos_;
    }
    float t = float(_index + 1) / float(_total);
    return Lerp(preWorldOriginPos_, worldOriginPos_, t);
}

void Emitter::UpdateWorldOriginPos() {
    preWorldOriginPos_ = worldOriginPos_;
    if (parent_) {
        worldOriginPos_ = parent_->GetWorldTranslate() + originPos_;
    } else {
        worldOriginPos_ = originPos_;
    }
}

void Emitter::PlayStart() {
    isActive_        = true;
    leftActiveTime_  = activeTime_;
    currentCoolTime_ = 0.f;
    // 初回は pre/current を同じ値に揃えてフレーム間補間を無効化
    if (parent_) {
        worldOriginPos_ = parent_->GetWorldTranslate() + originPos_;
    } else {
        worldOriginPos_ = originPos_;
    }
    preWorldOriginPos_ = worldOriginPos_;
}

void Emitter::ResolveParent(Scene* _scene) {
    if (_scene && parentHandle_.IsValid()) {
        parent_ = _scene->GetComponent<Transform>(parentHandle_);
    } else {
        parent_ = nullptr;
    }
}

void Emitter::PlayContinue() {
    isActive_ = true;
}

void Emitter::PlayStop() {
    isActive_       = false;
    leftActiveTime_ = 0.f;
}

// ── Serialization ──────────────────────────────────────────────────────────

void OriGine::to_json(nlohmann::json& _j, const Emitter& _ctrl) {
    _j["isActive"]            = _ctrl.isActive_;
    _j["isLoop"]              = _ctrl.isLoop_;
    _j["interpolateSpawnPos"] = _ctrl.interpolateSpawnPos_;
    _j["activeTime"]          = _ctrl.activeTime_;
    _j["spawnCount"]    = _ctrl.spawnCount_;
    _j["spawnCoolTime"] = _ctrl.spawnCoolTime_;
    _j["originPos"]     = _ctrl.originPos_;
    _j["shapeType"]     = _ctrl.shapeType_;
    _j["parentHandle"]  = _ctrl.parentHandle_;

    nlohmann::json shapeJson = nlohmann::json::object();
    if (_ctrl.spawnShape_) {
        switch (_ctrl.shapeType_) {
        case EmitterShapeType::SPHERE: {
            auto* s = dynamic_cast<EmitterSphere*>(_ctrl.spawnShape_.get());
            if (s) { shapeJson = *s; }
            break;
        }
        case EmitterShapeType::BOX: {
            auto* s = dynamic_cast<EmitterBox*>(_ctrl.spawnShape_.get());
            if (s) { shapeJson = *s; }
            break;
        }
        case EmitterShapeType::CAPSULE: {
            auto* s = dynamic_cast<EmitterCapsule*>(_ctrl.spawnShape_.get());
            if (s) { shapeJson = *s; }
            break;
        }
        case EmitterShapeType::CONE: {
            auto* s = dynamic_cast<EmitterCone*>(_ctrl.spawnShape_.get());
            if (s) { shapeJson = *s; }
            break;
        }
        default:
            break;
        }
    }
    _j["EmitterShape"] = shapeJson;
}

void OriGine::from_json(const nlohmann::json& _j, Emitter& _ctrl) {
    _j.at("isActive").get_to(_ctrl.isActive_);
    _j.at("isLoop").get_to(_ctrl.isLoop_);
    if (_j.contains("interpolateSpawnPos")) {
        _j.at("interpolateSpawnPos").get_to(_ctrl.interpolateSpawnPos_);
    }
    _j.at("activeTime").get_to(_ctrl.activeTime_);
    _j.at("spawnCount").get_to(_ctrl.spawnCount_);
    _j.at("spawnCoolTime").get_to(_ctrl.spawnCoolTime_);
    _j.at("originPos").get_to(_ctrl.originPos_);
    _j.at("shapeType").get_to(_ctrl.shapeType_);
    if (_j.contains("parentHandle")) {
        _j.at("parentHandle").get_to(_ctrl.parentHandle_);
    }

    if (_j.find("EmitterShape") != _j.end()) {
        const auto& shapeJson = _j["EmitterShape"];
        switch (_ctrl.shapeType_) {
        case EmitterShapeType::SPHERE: {
            EmitterSphere s;
            shapeJson.get_to(s);
            _ctrl.spawnShape_ = std::make_shared<EmitterSphere>(s);
            break;
        }
        case EmitterShapeType::BOX: {
            EmitterBox s;
            shapeJson.get_to(s);
            _ctrl.spawnShape_ = std::make_shared<EmitterBox>(s);
            break;
        }
        case EmitterShapeType::CAPSULE: {
            EmitterCapsule s;
            shapeJson.get_to(s);
            _ctrl.spawnShape_ = std::make_shared<EmitterCapsule>(s);
            break;
        }
        case EmitterShapeType::CONE: {
            EmitterCone s;
            shapeJson.get_to(s);
            _ctrl.spawnShape_ = std::make_shared<EmitterCone>(s);
            break;
        }
        default:
            _ctrl.spawnShape_ = std::make_shared<EmitterSphere>();
            break;
        }
    } else {
        _ctrl.spawnShape_ = std::make_shared<EmitterSphere>();
    }
}
