#pragma once

/// stl
#include <cassert>
#include <queue>
#include <unordered_map>
#include <vector>

/// ECS
// entity
#include "entity/EntityHandle.h"
// component
#include "ComponentHandle.h"
#include "ECS/HandleAssignMode.h"
#include "IComponent.h"
#include "IComponentArray.h"

/// externals
#include "logger/Logger.h"
#include "uuidGenerator/UuidGenerator.h"
#include <uuid/uuid.h>

namespace OriGine {

/// <summary>
/// コンポーネント配列
/// </summary>
/// <typeparam name="ComponentType"></typeparam>
template <IsComponent ComponentType>
class ComponentArray final
    : public IComponentArray {
public:
    ComponentArray()           = default;
    ~ComponentArray() override = default;

    // ────────────────────────────────
    //  lifecycle
    // ────────────────────────────────
    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="_reserveSize">初期Arrayサイズ</param>
    void Initialize(uint32_t _reserveSize = kDefaultComponentArraySize) override;
    /// <summary>
    /// 終了化処理
    /// </summary>
    void Finalize() override;

    // ────────────────────────────────
    //  entity
    // ────────────────────────────────
    /// <summary>
    /// Entity登録
    /// </summary>
    void RegisterEntity(EntityHandle _entity) override;
    /// <summary>
    /// Entity登録解除
    /// </summary>
    /// <param name="_scene"></param>
    /// <param name="_entity"></param>
    void UnregisterEntity(EntityHandle _entity) override;

    /// <summary>
    /// Entityが登録されているか
    /// </summary>
    /// <param name="_entity"></param>
    /// <returns></returns>
    bool HasEntity(EntityHandle _entity) const override;

    // ────────────────────────────────
    //  component
    // ────────────────────────────────
    /// <summary>
    /// Componentの追加
    /// </summary>
    ComponentHandle AddComponent(Scene* _scene, EntityHandle _entity) override;

    /// <summary>
    /// Componentの挿入追加 (indexがsize以上なら最後尾に追加)
    /// </summary>
    /// <param name="_entity"></param>
    /// <param name="_compIndex"></param>
    /// <returns></returns>
    ComponentHandle InsertComponent(Scene* _scene, EntityHandle _entity, uint32_t _compIndex) override;

    /// <summary>
    /// Componentの削除
    /// </summary>
    /// <param name="_component"></param>
    void RemoveComponent(ComponentHandle _handle) override;
    /// <summary>
    /// Componentの削除(非推奨)
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_compIndex"></param>
    void RemoveComponent(EntityHandle _handle, uint32_t _compIndex = 0) override;

    /// <summary>
    /// Entityが所有するComponent全ての削除
    /// </summary>
    /// <param name="_handle"></param>
    void RemoveAllComponents(EntityHandle _handle) override;

    // ────────────────────────────────
    //  serialization
    // ────────────────────────────────
    /// <summary>
    /// 指定したComponentを保存する
    /// </summary>
    /// <param name="_compHandle"></param>
    /// <param name="_outJson">保存先</param>
    bool SaveComponent(ComponentHandle _compHandle, nlohmann::json& _outJson) override;
    /// <summary>
    /// 指定したComponentを保存する
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_compIndex"></param>
    /// <param name="_outJson">保存先</param>
    bool SaveComponent(EntityHandle _handle, uint32_t _compIndex, nlohmann::json& _outJson) override;

    /// <summary>
    /// 指定されたEntityが持つComponent全てを保存する
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_outJson">保存先</param>
    bool SaveComponents(EntityHandle _handle, nlohmann::json& _outJson) override;

    /// <summary>
    /// JsonからComponentを復元し、Entityに追加する
    /// </summary>
    /// <param name="_handle">追加さき</param>
    /// <param name="_inJson">復元もと</param>
    /// <param name="_handleMode">Handleの割り当て方法 (デフォルト: UseSaved)</param>
    /// <returns>復元されたComponentのHandle</returns>
    ComponentHandle LoadComponent(
        EntityHandle _handle,
        const nlohmann::json& _inJson,
        HandleAssignMode _handleMode = HandleAssignMode::UseSaved) override;

    /// <summary>
    /// JsonからComponentを復元し、Entityに挿入する
    /// </summary>
    /// <param name="_handle">追加さき</param>
    /// <param name="_compIndex">挿入先</param>
    /// <param name="_inJson">復元もと</param>
    /// <param name="_handleMode">Handleの割り当て方法 (デフォルト: UseSaved)</param>
    /// <returns>復元されたComponentのHandle</returns>
    ComponentHandle LoadComponent(
        EntityHandle _handle,
        uint32_t _compIndex,
        const nlohmann::json& _inJson,
        HandleAssignMode _handleMode = HandleAssignMode::UseSaved) override;

    /// <summary>
    /// Jsonから全てのComponentを復元し、Entityに追加する。
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_inJson"></param>
    /// <param name="_handleMode">Handleの割り当て方法 (デフォルト: UseSaved)</param>
    void LoadComponents(
        EntityHandle _handle,
        const nlohmann::json& _inJson,
        HandleAssignMode _handleMode = HandleAssignMode::UseSaved) override;

    // ────────────────────────────────
    //  getters
    // ────────────────────────────────
    /// <summary>
    /// Componentの取得
    /// </summary>
    /// <param name="_component"></param>
    /// <returns></returns>
    ComponentType* GetComponent(ComponentHandle _handle);
    /// <summary>
    /// Componentの取得
    /// </summary>
    /// <param name="_component"></param>
    /// <returns></returns>
    ComponentType* GetComponent(EntityHandle _handle, uint32_t _compIndex = 0);

    /// <summary>
    /// Entityが所有するComponent全ての取得
    /// </summary>
    /// <param name="_handle"></param>
    /// <returns></returns>
    std::vector<ComponentType>& GetComponents(EntityHandle _handle);

    /// <summary>
    /// Componentの取得 (IComponent版)
    /// </summary>
    /// <param name="_component"></param>
    /// <returns></returns>
    IComponent* GetIComponent(ComponentHandle _handle) override;
    /// <summary>
    /// Componentの取得 (IComponent版)
    /// </summary>
    /// <param name="_component"></param>
    /// <returns></returns>
    IComponent* GetIComponent(EntityHandle _handle, uint32_t _compIndex = 0) override;

    /// <summary>
    /// 指定したEntityが所有する全てのIComponentを取得する
    /// </summary>
    /// <param name="_handle"></param>
    /// <returns></returns>
    std::vector<IComponent*> GetIComponents(EntityHandle _handle) override;

    /// <summary>
    /// 指定したEntityが所有するComponent数を取得する
    /// </summary>
    /// <param name="_handle"></param>
    /// <returns></returns>
    uint32_t GetComponentCount(EntityHandle _handle) const;

public:
    /// <summary>
    /// コンポーネントの位置情報
    /// </summary>
    struct ComponentLocation {
        uint32_t entitySlot;
        uint32_t componentIndex;
    };
    /// <summary>
    /// コンポーネントのスロット内インデックス
    /// </summary>
    struct EntitySlot {
        bool alive = false;
        EntityHandle owner{};
        std::vector<ComponentType> components;
    };

private:
    std::vector<EntitySlot> slots_;
    std::queue<uint32_t> freeSlots_;

    // entity uuid -> slot index
    std::unordered_map<uuids::uuid, uint32_t> entitySlotMap_;
    // component uuid -> (slot index, component index)
    std::unordered_map<uuids::uuid, ComponentLocation> componentLocationMap_;

public:
    const std::vector<EntitySlot>& GetSlots() const { return slots_; }
    std::vector<EntitySlot>& GetSlotsRef() { return slots_; }

    const std::unordered_map<uuids::uuid, uint32_t>& GetEntitySlotMap() const { return entitySlotMap_; }
    const std::unordered_map<uuids::uuid, ComponentLocation>& GetComponentLocationMap() const { return componentLocationMap_; }
    bool IsEmpty() const { return entitySlotMap_.empty(); }
};

} // namespace OriGine

// テンプレート実装のインクルード
#include "ComponentArray.inl"
