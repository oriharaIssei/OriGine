#pragma once

/// stl
#include <algorithm>
#include <vector>

/// ECS
// entity
#include "entity/Entity.h"
#include "entity/EntityHandle.h"
// component
#include "component/ComponentArray.h"
#include "component/ComponentHandle.h"
#include "component/ComponentRepository.h"
// system
#include "system/SystemCategory.h"

/// util
#include "deltaTime/DeltaTimer.h"

namespace OriGine {

class Scene;
class EntityRepository;

/// <summary>
/// システムの基底インターフェース
/// </summary>
class ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="_category">システムの実行カテゴリ</param>
    /// <param name="_priority">同一カテゴリ内での実行優先順位（昇順）</param>
    ISystem(SystemCategory _category, int32_t _priority = 0)
        : category_(_category), priority_(_priority) {}

    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~ISystem() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    virtual void Initialize() = 0;
    /// <summary>
    /// 終了処理
    /// </summary>
    virtual void Finalize() = 0;

    /// <summary>
    /// 更新処理
    /// </summary>
    virtual void Run();
    /// <summary>
    /// 編集処理
    /// </summary>
    virtual void Edit();

    /// <summary>
    /// 無効Entityの削除処理
    /// </summary>
    void EraseDeadEntity();

public:
    /// <summary>
    /// Run() から呼ばれる処理
    /// </summary>
    virtual void Update();
    /// <summary>
    /// 更新処理 (Entity単位)
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    virtual void UpdateEntity([[maybe_unused]] EntityHandle _handle) {}

    //==========================================
    // ECS Accessors (Handle based)
    //==========================================
    /// <summary>
    /// エンティティを生成する
    /// </summary>
    /// <param name="_dataType">エンティティのデータタイプ</param>
    /// <param name="_isUnique">ユニークなエンティティとして登録するか</param>
    /// <returns>作成されたエンティティのハンドル</returns>
    EntityHandle CreateEntity(const std::string& _dataType, bool _isUnique = false);

    /// <summary>
    /// エンティティを取得する
    /// </summary>
    /// <param name="_entity">対象のエンティティハンドル</param>
    /// <returns>エンティティのポインタ (存在しない場合はnullptr)</returns>
    Entity* GetEntity(EntityHandle _entity);

    /// <summary>
    /// ユニークエンティティを取得する (存在しない場合は空のハンドルを返す)
    /// </summary>
    /// <param name="_dataType">取得するエンティティのタイプ名</param>
    /// <returns>取得したエンティティのハンドル</returns>
    EntityHandle GetUniqueEntity(const std::string& _dataType);

    /// <summary>
    /// コンポーネント配列を取得する
    /// </summary>
    /// <param name="_typeName">コンポーネントのタイプ名</param>
    /// <returns>コンポーネント配列のインターフェース</returns>
    IComponentArray* GetComponentArray(const std::string& _typeName);

    /// <summary>
    /// コンポーネントを取得する
    /// </summary>
    /// <typeparam name="ComponentType">コンポーネントの型</typeparam>
    /// <param name="_handle">コンポーネントハンドル</param>
    /// <returns>コンポーネントのポインタ</returns>
    template <IsComponent ComponentType>
    ComponentType* GetComponent(ComponentHandle _handle);

    /// <summary>
    /// コンポーネントを取得する
    /// </summary>
    /// <typeparam name="ComponentType">コンポーネントの型</typeparam>
    /// <param name="_handle">対象のエンティティハンドル</param>
    /// <param name="_index">同種のコンポーネントが複数ある場合のインデックス</param>
    /// <returns>コンポーネントのポインタ</returns>
    template <IsComponent ComponentType>
    ComponentType* GetComponent(EntityHandle _handle, int32_t _index = 0);

    /// <summary>
    /// エンティティに属する特定の型のコンポーネントリストを取得する
    /// </summary>
    /// <typeparam name="ComponentType">コンポーネントの型</typeparam>
    /// <param name="_entity">対象のエンティティハンドル</param>
    /// <returns>コンポーネントのリストへの参照</returns>
    template <IsComponent ComponentType>
    std::vector<ComponentType>& GetComponents(EntityHandle _entity);

    /// <summary>
    /// コンポーネント配列を取得する
    /// </summary>
    /// <typeparam name="ComponentType">コンポーネントの型</typeparam>
    /// <returns>特定の型のコンポーネント配列へのポインタ</returns>
    template <IsComponent ComponentType>
    ComponentArray<ComponentType>* GetComponentArray();

    /// <summary>
    /// コンポーネントを追加する
    /// </summary>
    /// <typeparam name="ComponentType">コンポーネントの型</typeparam>
    /// <param name="_entity">対象のエンティティハンドル</param>
    /// <returns>追加されたコンポーネントのハンドル</returns>
    template <IsComponent ComponentType>
    ComponentHandle AddComponent(EntityHandle _entity);

    /// <summary>
    /// コンポーネントを追加する
    /// </summary>
    /// <param name="_entity">対象のエンティティハンドル</param>
    /// <param name="_typeName">型名</param>
    /// <returns>追加されたコンポーネントのハンドル</returns>
    ComponentHandle AddComponent(EntityHandle _entity, const ::std::string& _typeName);

protected:
    std::vector<EntityHandle> entities_;

#ifndef _RELEASE
    DeltaTimer deltaTimer_;
#endif

private:
    Scene* scene_                             = nullptr;
    EntityRepository* entityRepository_       = nullptr;
    ComponentRepository* componentRepository_ = nullptr;

    SystemCategory category_;
    int32_t priority_ = 0;
    bool isActive_    = false;

public:
    //==========================================
    // accessor
    //==========================================
    /// <summary>
    /// 所属するSceneを取得する
    /// </summary>
    /// <returns>Sceneへのポインタ</returns>
    Scene* GetScene() const { return scene_; }
    /// <summary>
    /// 所属するSceneを設定する
    /// </summary>
    /// <param name="_scene">所属させるScene</param>
    void SetScene(Scene* _scene);

    /// <summary>
    /// カテゴリを取得する
    /// </summary>
    /// <returns>SystemCategoryの値</returns>
    SystemCategory GetCategory() const { return category_; }

    /// <summary>
    /// 登録されているEntityHandleリストを取得する
    /// </summary>
    /// <returns>EntityHandleのリストへの参照</returns>
    const std::vector<EntityHandle>& GetEntities() const { return entities_; }
    /// <summary>
    /// 登録されているEntity数を取得する
    /// </summary>
    /// <returns>Entity数</returns>
    int32_t GetEntityCount() const { return static_cast<int32_t>(entities_.size()); }

    /// <summary>
    /// 特定のEntityがシステムに登録されているか
    /// </summary>
    /// <param name="_entity">対象のエンティティハンドル</param>
    /// <returns>登録されていればtrue</returns>
    bool HasEntity(EntityHandle _entity) const {
        return std::find_if(
                   entities_.begin(),
                   entities_.end(),
                   [&](const EntityHandle& e) { return e.uuid == _entity.uuid; })
               != entities_.end();
    }

    /// <summary>
    /// Entityをシステムに登録する
    /// </summary>
    /// <param name="_entity">登録するエンティティハンドル</param>
    void AddEntity(EntityHandle _entity) {
        if (!HasEntity(_entity)) {
            entities_.push_back(_entity);
        }
    }

    /// <summary>
    /// Entityをシステムから除外する
    /// </summary>
    /// <param name="_entity">除外するエンティティハンドル</param>
    void RemoveEntity(EntityHandle _entity) {
        entities_.erase(
            std::remove_if(
                entities_.begin(),
                entities_.end(),
                [&](const EntityHandle& e) { return e.uuid == _entity.uuid; }),
            entities_.end());
    }

    /// <summary>
    /// 全てのEntityをシステムから除外する
    /// </summary>
    void ClearEntities() {
        entities_.clear();
    }

    /// <summary>
    /// 更新優先度を設定する
    /// </summary>
    /// <param name="_priority">優先度の値 (昇順に実行)</param>
    void SetPriority(int32_t _priority) { priority_ = _priority; }
    /// <summary>
    /// 更新優先度を取得する
    /// </summary>
    /// <returns>優先度の値</returns>
    int32_t GetPriority() const { return priority_; }

    /// <summary>
    /// システムがアクティブかどうかを取得する
    /// </summary>
    /// <returns>アクティブならtrue</returns>
    bool IsActive() const { return isActive_; }
    /// <summary>
    /// システムのアクティブ状態を設定する
    /// </summary>
    /// <param name="_isActive">アクティブにするならtrue</param>
    void SetIsActive(bool _isActive) { isActive_ = _isActive; }
};

/// <summary>
/// コンポーネントを取得する
/// </summary>
/// <typeparam name="ComponentType">コンポーネントの型</typeparam>
/// <param name="_handle">コンポーネントハンドル</param>
/// <returns>コンポーネントのポインタ</returns>
template <IsComponent ComponentType>
inline ComponentType* ISystem::GetComponent(ComponentHandle _handle) {
    if (!componentRepository_) {
        LOG_ERROR("ComponentRepository is not set.");
        return nullptr;
    }
    return componentRepository_->GetComponent<ComponentType>(_handle);
}

/// <summary>
/// コンポーネントを取得する (非推奨 ComponentHandleの使用を推奨します)
/// </summary>
/// <typeparam name="ComponentType">コンポーネントの型</typeparam>
/// <param name="_handle">対象のエンティティハンドル</param>
/// <param name="_index">インデックス</param>
/// <returns>コンポーネントのポインタ</returns>
template <IsComponent ComponentType>
inline ComponentType* ISystem::GetComponent(EntityHandle _handle, int32_t _index) {
    auto* componentArray = GetComponentArray<ComponentType>();
    if (!componentArray) {
        LOG_ERROR("ComponentArray is not found.");
        return nullptr;
    }
    return componentArray->GetComponent(_handle, _index);
}

/// <summary>
/// コンポーネントのリストを取得する
/// </summary>
/// <typeparam name="ComponentType">コンポーネントの型</typeparam>
/// <param name="_entity">対象のエンティティハンドル</param>
/// <returns>コンポーネントリストの参照</returns>
template <IsComponent ComponentType>
inline std::vector<ComponentType>& ISystem::GetComponents(EntityHandle _entity) {
    auto* componentArray = GetComponentArray<ComponentType>();
    if (!componentArray) {
        LOG_ERROR("ComponentArray is not found.");
        // ダミーの空配列を返す
        static std::vector<ComponentType> emptyComponents;
        return emptyComponents;
    }
    return componentArray->GetComponents(_entity);
}

/// <summary>
/// コンポーネント配列を取得する
/// </summary>
/// <typeparam name="ComponentType">コンポーネントの型</typeparam>
/// <returns>コンポーネント配列へのポインタ</returns>
template <IsComponent ComponentType>
inline ComponentArray<ComponentType>* ISystem::GetComponentArray() {
    if (!componentRepository_) {
        LOG_ERROR("ComponentRepository is not set.");
        return nullptr;
    }
    return componentRepository_->GetComponentArray<ComponentType>();
}

/// <summary>
/// コンポーネントを追加する
/// </summary>
/// <typeparam name="ComponentType">コンポーネントの型</typeparam>
/// <param name="_entity">対象のエンティティハンドル</param>
/// <returns>追加されたコンポーネントのハンドル</returns>
template <IsComponent ComponentType>
inline ComponentHandle ISystem::AddComponent(EntityHandle _entity) {
    return GetComponentArray<ComponentType>()->AddComponent(scene_, _entity);
}

// Systemを継承しているかどうか
template <typename T>
concept IsSystem = ::std::is_base_of<ISystem, T>::value;

} // namespace OriGine
