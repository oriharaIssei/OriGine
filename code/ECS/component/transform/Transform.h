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

/// DLL境界
#include "OriGineApi.h"

namespace OriGine {

/// <summary>
/// Transform コンポーネント(3次元)
/// </summary>
struct ORIGINE_API Transform
    : public IComponent {
public:
    ORIGINE_COMPONENT();

    Transform();
    Transform(const Vec3f& _scale, const Quaternion& _rotate, const Vec3f& _translate)
        : scale(_scale), rotate(_rotate), translate(_translate), worldMat(MakeMatrix4x4::Identity()) {}
    ~Transform() {}

    void Initialize(Scene* _scene, const EntityHandle& _entity);
    void UpdateMatrix();
    Quaternion CalculateWorldRotate() const;
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel);

    void Finalize() {};

public:
    Vec3f scale        = {1.0f, 1.0f, 1.0f};
    Quaternion rotate  = {0.0f, 0.0f, 0.0f, 1.0f};
    Vec3f translate    = {0.0f, 0.0f, 0.0f};
    ORIGINE_FIELD(no_save); // 毎フレーム UpdateMatrix() で再計算されるため保存しない
    Matrix4x4 worldMat = MakeMatrix4x4::Identity();

    ORIGINE_FIELD(no_save); // 実行時の生ポインタは永続化できない
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

    /// <summary>
    /// 回転から求めた前方ベクトルを取得
    /// </summary>
    /// <returns></returns>
    Vec3f FrontVector() const;
    /// <summary>
    /// 回転から求めた右方向ベクトルを取得
    /// </summary>
    /// <returns></returns>
    Vec3f RightVector() const;
    /// <summary>
    /// 回転から求めた上方向ベクトルを取得
    /// </summary>
    /// <returns></returns>
    Vec3f UpVector() const;

public:
    struct ConstantBuffer {
        Matrix4x4 world;
        ConstantBuffer& operator=(const Transform& _comp) {
            world = _comp.worldMat;
            return *this;
        }
    };
};

inline void from_json(const nlohmann::json& _j, Transform& _comp) {
    _j.at("scale").get_to(_comp.scale);
    _j.at("rotate").get_to(_comp.rotate);
    _j.at("translate").get_to(_comp.translate);
}

inline void to_json(nlohmann::json& _j, const Transform& _comp) {
    _j = nlohmann::json{{"scale", _comp.scale}, {"rotate", _comp.rotate}, {"translate", _comp.translate}};
}

// D-1: 保存対象フィールドに Opaque が無いため、表経由でシリアライズする
// (上の to_json/from_json は D-4 で表経由に置き換わるまでのフォールバックとして残す)。
template <>
inline constexpr bool kUsesDescriptorSerialization<Transform> = true;

} // namespace OriGine
