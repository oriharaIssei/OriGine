#pragma once

/// parent
#include "component/IComponent.h"
/// stl
#include <concepts>
#include <unordered_map>

/// engine
/// ECS
// component
#include "component/collision/collider/base/CollisionCategory.h"
#include "component/transform/Transform.h"

/// math
#include "bounds/AABB.h"
#include "bounds/base/IBounds.h"

// external
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

namespace OriGine {

/// <summary>
/// 衝突状態
/// </summary>
enum class CollisionState {
    None, // 衝突していない
    Stay, // 衝突中
    Enter, // 衝突開始時
    Exit // 衝突終了時
};

// 前方宣言。Collider<BoundsClass>::CalculateWorldShape() が形状ごとに選ぶ
// Bounds::CalculateWorldShape(...) のオーバーロード群。
// 実引数は参照なので定義側(各コライダーの.cpp)でだけ完全な型が要る。
// ここで前方宣言しておくのは、テンプレート内からの修飾呼び出し(Bounds::CalculateWorldShape(...))が
// 「テンプレート定義時点で名前空間内に見えている宣言だけを候補にする」という二段階名前探索の規則を
// 満たすため(qualified callはADLの対象にならないので、テンプレート定義より後に足しても拾われない)。
namespace Bounds {
struct Sphere;
struct OBB;
struct Capsule;
struct Segment;
struct Ray;

/// <summary>
/// ローカル形状とTransformの現在値から、ワールド空間の形状を再計算する(形状ごとのオーバーロード)。
/// 計算式そのものは各コライダーの.cppにある(旧: 各コライダーのCalculateWorldShape()の中身そのまま)。
/// </summary>
void CalculateWorldShape(const Sphere& _local, Sphere& _world, const Transform& _transform);
void CalculateWorldShape(const AABB& _local, AABB& _world, const Transform& _transform);
void CalculateWorldShape(const OBB& _local, OBB& _world, const Transform& _transform);
void CalculateWorldShape(const Capsule& _local, Capsule& _world, const Transform& _transform);
void CalculateWorldShape(const Segment& _local, Segment& _world, const Transform& _transform);
void CalculateWorldShape(const Ray& _local, Ray& _world, const Transform& _transform);
} // namespace Bounds

/// <summary>
/// コライダーのインターフェース
/// </summary>
class ICollider
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const ICollider& _c);
    friend void from_json(const nlohmann::json& _j, ICollider& _c);

public:
    ICollider() {}
    ~ICollider() {}

    // Initialize / Finalize / CalculateWorldShape / ToWorldAABB は
    // ここでは実装を持たない(旧: 純粋仮想)ため、宣言ごと派生側へ移した(Phase 3 3B)。
    // Initialize/FinalizeはCollider<BoundsClass>が、CalculateWorldShape/ToWorldAABBは
    // 形状ごとに異なるため後述のとおりCollider<BoundsClass>とconcrete型に分かれて実装を持つ。

    /// <summary>
    /// デバッグ用GUIでのパラメータ編集（isActive_・collisionCategory_など、形状によらない共通部分のみ）
    /// </summary>
    void Edit(Scene* _scene, const EntityHandle& _handle, const std::string& _parentLabel);

    /// <summary>
    /// 衝突判定開始時の状態更新のうち、形状によらない共通部分（前フレームの状態を保存し、今フレーム分をクリアする）。
    /// ワールド形状の再計算(CalculateWorldShape)は形状型を知らないここでは行えないため、
    /// Collider&lt;BoundsClass&gt;::StartCollision() がこれを呼んでから自分でCalculateWorldShape()を呼ぶ。
    /// </summary>
    void StartCollision();
    /// <summary>
    /// 衝突判定終了時の状態更新（このフレームで衝突が無くなった相手をExit状態にする）
    /// </summary>
    void EndCollision();

    /// <summary>
    /// 衝突可能か判定（Manager経由でマトリクス参照）
    /// </summary>
    /// <param name="_other">相手のコライダー</param>
    bool CanCollideWith(const ICollider& _other) const;

protected:
    bool isActive_ = true; // このコライダーが衝突判定の対象かどうか

    CollisionCategory collisionCategory_ = CollisionCategory(); // 所属する衝突カテゴリ

    Transform transform_;
    std::unordered_map<EntityHandle, CollisionState> collisionStateMap_; // 現フレームの相手ごとの衝突状態
    std::unordered_map<EntityHandle, CollisionState> preCollisionStateMap_; // 前フレームの相手ごとの衝突状態

public: // accessor
    bool IsActive() const { return isActive_; }
    void SetActive(bool _isActive) { isActive_ = _isActive; }

    const Transform& GetTransform() const { return transform_; }
    void SetParent(Transform* _parent) { transform_.parent = _parent; }

    const CollisionCategory& GetCollisionCategory() const { return collisionCategory_; }
    void SetCollisionCategory(const CollisionCategory& _category) { collisionCategory_ = _category; }

    // 衝突状態の操作
    /// <summary>
    /// 相手エンティティとの衝突状態を更新する（前フレーム未衝突ならEnter、それ以外はStayにする）
    /// </summary>
    /// <param name="_otherHandle">相手エンティティのハンドル</param>
    void SetCollisionState(const EntityHandle& _otherHandle) {
        if (this->preCollisionStateMap_[_otherHandle] == CollisionState::None) {
            this->collisionStateMap_[_otherHandle] = CollisionState::Enter;
        } else {
            this->collisionStateMap_[_otherHandle] = CollisionState::Stay;
        }
    }
    const std::unordered_map<EntityHandle, CollisionState>& GetCollisionStateMap() const { return collisionStateMap_; }
};

/// <summary>
/// 形状(BoundsClass)を持つコライダーの共通実装。ローカル/ワールド形状の保持を担う。
/// </summary>
template <Bounds::IsBounds BoundsClass>
class Collider
    : public ICollider {
public:
    Collider() {}
    void Initialize(Scene* /*_scene*/, const EntityHandle& /*_entity*/) {}
    void Finalize() {
        this->collisionStateMap_.clear();
        this->preCollisionStateMap_.clear();
    }

    // Edit / ToWorldAABB はここでは実装を持たない（形状固有のUI・計算が要るため）。
    // 具象コライダー（SphereCollider等）が自分の非仮想メンバとして直接持つ(呼ぶ側が具象型を知っているため)。

    /// <summary>
    /// 衝突判定開始時の状態更新。形状によらない共通部分(状態mapの退避・クリア)はICollider側、
    /// ワールド形状の再計算はこのクラスのCalculateWorldShape()が担う。
    /// </summary>
    void StartCollision() {
        ICollider::StartCollision();
        CalculateWorldShape();
    }

    /// <summary>
    /// ローカル形状(shape_)とTransformの現在値から、ワールド空間の形状(worldShape_)を再計算する。
    /// 形状ごとの計算式はBounds::CalculateWorldShape(...)のオーバーロード(形状型で選ばれる)に委譲する。
    /// </summary>
    void CalculateWorldShape() {
        transform_.UpdateMatrix();
        Bounds::CalculateWorldShape(shape_, worldShape_, transform_);
    }

protected:
    BoundsClass shape_; // ローカル空間での形状
    BoundsClass worldShape_; // ワールド空間に変換された形状（CalculateWorldShapeで更新される）

public:
    const BoundsClass& GetLocalShape() { return shape_; }
    const BoundsClass& GetWorldShape() { return worldShape_; }

    BoundsClass* GetLocalShapePtr() { return &shape_; }
    BoundsClass* GetWorldShapePtr() { return &worldShape_; }
};

} // namespace OriGine
