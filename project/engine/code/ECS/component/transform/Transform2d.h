#pragma once
#include "component/IComponent.h"

/// math
#include "math/Matrix3x3.h"
#include "math/Matrix4x4.h"
#include "math/Vector2.h"

namespace OriGine {

/// <summary>
/// Transform コンポーネント(2次元)
/// </summary>
struct Transform2d
    : public IComponent {
public:
    Transform2d() = default;

    Transform2d(const Vec2f& _scale, float _rotate, const Vec2f& _translate)
        : scale(_scale), rotate(_rotate), translate(_translate),
          worldMat(MakeMatrix3x3::Affine(scale, rotate, translate)) {}

    ~Transform2d() = default;

    void Initialize(Scene* /*_scene*/, EntityHandle /*_owner*/) override {}
    void Finalize() override {}

    /// <summary>
    /// ローカルの変換行列を更新
    /// </summary>
    void UpdateMatrix();

    /// <summary>
    /// World の回転角を取得
    /// </summary>
    float CalculateWorldRotate() const {
        if (!parent) {
            return rotate;
        }
        return parent->CalculateWorldRotate() + rotate;
    }

    /// <summary>
    /// エディタ表示
    /// </summary>
    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

public:
    Vec2f scale     = {1.0f, 1.0f};
    float rotate    = 0.0f; // ラジアン
    Vec2f translate = {0.0f, 0.0f};

    Matrix3x3 worldMat = MakeMatrix3x3::Identity();

    Transform2d* parent = nullptr;

public:
    Vec2f GetWorldTranslate() const {
        return {worldMat[2][X], worldMat[2][Y]};
    }

    Vec2f GetWorldScale() const {
        // 2D では行列の x, y の長さでスケールを算出
        return {
            Vec2f::Length({worldMat[0][X], worldMat[0][Y]}),
            Vec2f::Length({worldMat[1][X], worldMat[1][Y]})};
    }

public:
    struct ConstantBuffer {
        Matrix3x3 world;
        ConstantBuffer& operator=(const Transform2d& _transform) {
            world = _transform.worldMat;
            return *this;
        }
    };
};

/// JSON 読み取り
inline void from_json(const nlohmann::json& _j, Transform2d& _comp) {
    _j.at("scale").get_to(_comp.scale);
    _j.at("rotate").get_to(_comp.rotate);
    _j.at("translate").get_to(_comp.translate);
}

/// JSON 書き込み
inline void to_json(nlohmann::json& _j, const Transform2d& _comp) {
    _j = nlohmann::json{
        {"scale", _comp.scale},
        {"rotate", _comp.rotate},
        {"translate", _comp.translate}};
}

} // namespace OriGine
