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

/// <summary>
/// コライダーのインターフェース
/// </summary>
class ICollider
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const ICollider& _c);
    friend void from_json(const nlohmann::json& _j, ICollider& _c);

public:
    ICollider() {}
    virtual ~ICollider() {}

    /// <summary>
    /// コライダーの初期化処理
    /// </summary>
    virtual void Initialize(Scene* _scene, const EntityHandle& _entity) = 0;
    /// <summary>
    /// コライダーの終了処理
    /// </summary>
    virtual void Finalize()                                      = 0;

    /// <summary>
    /// デバッグ用GUIでのパラメータ編集
    /// </summary>
    virtual void Edit(Scene* _scene, const EntityHandle& _handle, const std::string& _parentLabel);

    /// <summary>
    /// ローカル形状とTransformからワールド空間の形状を再計算する
    /// </summary>
    virtual void CalculateWorldShape() = 0;

    /// <summary>
    /// 衝突判定開始時の状態更新（前フレームの状態を保存し、ワールド形状を更新する）
    /// </summary>
    virtual void StartCollision();
    /// <summary>
    /// 衝突判定終了時の状態更新（このフレームで衝突が無くなった相手をExit状態にする）
    /// </summary>
    virtual void EndCollision();

    /// <summary>
    /// 衝突可能か判定（Manager経由でマトリクス参照）
    /// </summary>
    /// <param name="_other">相手のコライダー</param>
    bool CanCollideWith(const ICollider& _other) const;

    /// <summary>
    /// ワールドAABBを形状から生成、取得
    /// </summary>
    virtual Bounds::AABB ToWorldAABB() const = 0;

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
    void Initialize(Scene* /*_scene*/, const EntityHandle& /*_entity*/) override {}
    void Finalize() override {
        this->collisionStateMap_.clear();
        this->preCollisionStateMap_.clear();
    }

    virtual void Edit(Scene* _scene, const EntityHandle& _handle, const std::string& _parentLabel) = 0;

    virtual void CalculateWorldShape() = 0;

    virtual Bounds::AABB ToWorldAABB() const = 0;

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
