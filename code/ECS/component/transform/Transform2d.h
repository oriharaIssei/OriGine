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
    ORIGINE_COMPONENT();

    Transform2d() = default;

    Transform2d(const Vec2f& _scale, float _rotate, const Vec2f& _translate)
        : scale(_scale), rotate(_rotate), translate(_translate),
          worldMat(MakeMatrix3x3::Affine(scale, rotate, translate)) {}

    ~Transform2d() = default;

    void Initialize(Scene* /*_scene*/, const EntityHandle& /*_owner*/) {}
    void Finalize() {}

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
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel);

public:
    Vec2f scale     = {1.0f, 1.0f};
    // 2D では回転がXY平面内の1自由度(スクリーンに垂直な軸周りの回転)しかないため、
    // 3D の Transform のようにクォータニオンを使わず、角度(ラジアン)そのものを保持する。
    // 角度同士は単純な加算で合成できる(CalculateWorldRotate 参照)ため、この方が軽量
    float rotate    = 0.0f; // ラジアン
    Vec2f translate = {0.0f, 0.0f};

    ORIGINE_FIELD(no_save); // 毎フレーム UpdateMatrix() で再計算されるため保存しない
    Matrix3x3 worldMat = MakeMatrix3x3::Identity();

    ORIGINE_FIELD(no_save); // 実行時の生ポインタは永続化できない
    Transform2d* parent = nullptr;

public:
    Vec2f GetWorldTranslate() const {
        // 3x3 のアフィン行列(行ベクトル規約)では、平行移動成分は3行目(インデックス2)に入る
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

// D-1: 保存対象フィールドに Opaque が無いため、表経由でシリアライズする。
// 手書きの to_json/from_json は D-4 で削除済み(ComponentArray 経由以外の呼び出しが
// 無いことを確認済み。docs/todo.html Phase 3 D-4 参照)。
template <>
inline constexpr bool kUsesDescriptorSerialization<Transform2d> = true;

} // namespace OriGine
