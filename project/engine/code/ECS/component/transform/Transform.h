#pragma once

/// microsoft
#include <wrl.h>
/// dx12
#include <d3d12.h>
/// stl
#include <array>
#include <functional>
#include <string>

/// engine
#include "component/IComponent.h"

/// math
#include "Matrix4x4.h"
#include "Quaternion.h"

/// <summary>
/// Transform コンポーネント(3次元)
/// </summary>
struct Transform
    : public IComponent {
public:
    Transform();
    Transform(const Vec3f& _scale, const Quaternion& _rotate, const Vec3f& _translate)
        : scale(_scale), rotate(_rotate), translate(_translate), worldMat(MakeMatrix4x4::Identity()) {}
    ~Transform() {}

    void Initialize(Entity* _entity) override;
    void UpdateMatrix();
    Quaternion CalculateWorldRotate() const;
    void Edit(Scene* _scene, Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;

    void Finalize() override {};

public:
    Vec3f scale        = {1.0f, 1.0f, 1.0f};
    Quaternion rotate  = {0.0f, 0.0f, 0.0f, 1.0f};
    Vec3f translate    = {0.0f, 0.0f, 0.0f};
    Matrix4x4 worldMat = MakeMatrix4x4::Identity();

    Transform* parent = nullptr;

public:
    Vec3f GetWorldTranslate() const { return worldMat[3]; }
    Vec3f GetWorldScale() const {
        Vec3f worldScale;
        worldScale[X] = Vec3f::Length(worldMat[0]);
        worldScale[Y] = Vec3f::Length(worldMat[1]);
        worldScale[Z] = Vec3f::Length(worldMat[2]);
        return worldScale;
    }

public:
    struct ConstantBuffer {
        Matrix4x4 world;
        ConstantBuffer& operator=(const Transform& transform) {
            world = transform.worldMat;
            return *this;
        }
    };
};

inline void from_json(const nlohmann::json& j, Transform& t) {
    j.at("scale").get_to(t.scale);
    j.at("rotate").get_to(t.rotate);
    j.at("translate").get_to(t.translate);
}

inline void to_json(nlohmann::json& j, const Transform& t) {
    j = nlohmann::json{{"scale", t.scale}, {"rotate", t.rotate}, {"translate", t.translate}};
}
