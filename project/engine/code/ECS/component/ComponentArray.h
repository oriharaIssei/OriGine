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
#include "IComponent.h"

/// externals
#include "logger/Logger.h"
#include "uuidGenerator/UuidGenerator.h"
#include <uuid/uuid.h>

namespace OriGine {
constexpr uint32_t kDefaultComponentArraySize = 128;

//============================================================
// IComponentArray
//============================================================
/// <summary>
/// コンポーネント配列インターフェース
/// </summary>
class IComponentArray {
public:
    virtual ~IComponentArray() = default;

    virtual void Initialize(uint32_t _reserveSize = kDefaultComponentArraySize) = 0;
    virtual void Finalize()                                                     = 0;

    /// <summary>
    /// Entity登録
    /// </summary>
    /// <param name="_entity"></param>
    virtual void RegisterEntity(EntityHandle _entity) = 0;
    /// <summary>
    /// Entity登録解除
    /// </summary>
    /// <param name="_entity"></param>
    virtual void UnregisterEntity(EntityHandle _entity) = 0;
    /// <summary>
    /// Entityが登録されたいるか
    /// </summary>
    /// <param name="_entity"></param>
    /// <returns></returns>
    virtual bool HasEntity(EntityHandle _entity) const = 0;

    /// <summary>
    /// Componentの追加
    /// </summary>
    /// <param name="_scene"></param>
    /// <param name="_entity"></param>
    /// <returns></returns>
    virtual ComponentHandle AddComponent(Scene* _scene, EntityHandle _entity) = 0;

    /// <summary>
    /// Componentの削除
    /// </summary>
    /// <param name="_component"></param>
    virtual void RemoveComponent(ComponentHandle _handle) = 0;
    /// <summary>
    /// Componentの削除
    /// </summary>
    /// <param name="_component"></param>
    virtual void RemoveComponent(EntityHandle _handle, int32_t _compIndex) = 0;

    /// <summary>
    /// Entityが所有するComponent全ての削除
    /// </summary>
    /// <param name="_handle"></param>
    virtual void RemoveAllComponents(EntityHandle _handle) = 0;

    /// <summary>
    /// 指定したComponentを保存する
    /// </summary>
    /// <param name="_compHandle"></param>
    /// <param name="_outJson">保存先</param>
    virtual bool SaveComponent(ComponentHandle _compHandle, nlohmann::json& _outJson) = 0;
    /// <summary>
    /// 指定したComponentを保存する
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_compIndex"></param>
    /// <param name="_outJson">保存先</param>
    virtual bool SaveComponent(EntityHandle _handle, int32_t _compIndex, nlohmann::json& _outJson) = 0;

    /// <summary>
    /// 指定されたEntityが持つComponent全てを保存する
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_outJson">保存先</param>
    virtual bool SaveComponents(EntityHandle _handle, nlohmann::json& _outJson) = 0;

    /// <summary>
    /// JsonからComponentを復元し、Entityに追加する
    /// </summary>
    /// <param name="_handle">追加さき</param>
    /// <param name="_inJson">復元もと</param>
    /// <returns>復元されたComponentのHandle</returns>
    virtual ComponentHandle LoadComponent(EntityHandle _handle, const nlohmann::json& _inJson) = 0;

    /// <summary>
    /// Jsonから全てのComponentを復元し、Entityに追加する。
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_inJson"></param>
    virtual void LoadComponents(EntityHandle _handle, const nlohmann::json& _inJson) = 0;

    /// <summary>
    /// Componentの取得
    /// </summary>
    /// <param name="_component"></param>
    /// <returns></returns>
    virtual IComponent* GetIComponent(ComponentHandle _handle) = 0;
    /// <summary>
    /// Componentの取得
    /// </summary>
    /// <param name="_component"></param>
    /// <returns></returns>
    virtual IComponent* GetIComponent(EntityHandle _handle, int32_t _compIndex) = 0;
};

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
    /// Componentの削除
    /// </summary>
    /// <param name="_component"></param>
    void RemoveComponent(ComponentHandle _handle) override;
    /// <summary>
    /// Componentの削除(非推奨)
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_compIndex"></param>
    void RemoveComponent(EntityHandle _handle, int32_t _compIndex = 0) override;

    /// <summary>
    /// Entityが所有するComponent全ての削除
    /// </summary>
    /// <param name="_handle"></param>
    void RemoveAllComponents(EntityHandle _handle) override;

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
    bool SaveComponent(EntityHandle _handle, int32_t _compIndex, nlohmann::json& _outJson) override;

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
    /// <returns>復元されたComponentのHandle</returns>
    ComponentHandle LoadComponent(EntityHandle _handle, const nlohmann::json& _inJson) override;

    /// <summary>
    /// Jsonから全てのComponentを復元し、Entityに追加する。
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_inJson"></param>
    void LoadComponents(EntityHandle _handle, const nlohmann::json& _inJson) override;

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
    ComponentType* GetComponent(EntityHandle _handle, int32_t _compIndex = 0);

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
    IComponent* GetIComponent(EntityHandle _handle, int32_t _compIndex = 0) override;

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

template <IsComponent ComponentType>
inline void ComponentArray<ComponentType>::Initialize(uint32_t _reserveSize) {
    slots_.reserve(_reserveSize);
    entitySlotMap_.clear();
    componentLocationMap_.clear();
    if (!freeSlots_.empty()) {
        freeSlots_ = std::queue<uint32_t>();
    }
}

template <IsComponent ComponentType>
inline void ComponentArray<ComponentType>::Finalize() {
    for (auto& slot : slots_) {
        if (!slot.alive) {
            continue;
        }
        for (auto& comp : slot.components) {
            comp.Finalize();
        }
    }
    slots_.clear();
    entitySlotMap_.clear();
    componentLocationMap_.clear();
}

template <IsComponent ComponentType>
inline void ComponentArray<ComponentType>::RegisterEntity(EntityHandle _entity) {
    if (entitySlotMap_.contains(_entity.uuid)) {
        return;
    }

    uint32_t slotIndex;

    if (!freeSlots_.empty()) {
        slotIndex = freeSlots_.front();
        freeSlots_.pop();
    } else {
        slotIndex = static_cast<uint32_t>(slots_.size());
        slots_.emplace_back();
    }

    EntitySlot& slot = slots_[slotIndex];
    slot.alive       = true;
    slot.owner       = _entity;
    slot.components.clear();

    entitySlotMap_[_entity.uuid] = slotIndex;
}

template <IsComponent ComponentType>
inline void ComponentArray<ComponentType>::UnregisterEntity(EntityHandle _entity) {
    auto itr = entitySlotMap_.find(_entity.uuid);
    if (itr == entitySlotMap_.end()) {
        return;
    }

    uint32_t slotIndex = itr->second;
    EntitySlot& slot   = slots_[slotIndex];

    for (auto& comp : slot.components) {
        comp.Finalize();
        componentLocationMap_.erase(comp.GetHandle().uuid);
    }

    slot.components.clear();
    slot.alive = false;
    slot.owner = {};

    entitySlotMap_.erase(itr);
    freeSlots_.push(slotIndex);
}

template <IsComponent ComponentType>
inline bool ComponentArray<ComponentType>::HasEntity(EntityHandle _entity) const {
    return entitySlotMap_.contains(_entity.uuid);
}

template <IsComponent ComponentType>
inline ComponentHandle ComponentArray<ComponentType>::AddComponent(Scene* _scene, EntityHandle _entity) {
    auto entIt = entitySlotMap_.find(_entity.uuid);
    if (entIt == entitySlotMap_.end()) {
        RegisterEntity(_entity);
        entIt = entitySlotMap_.find(_entity.uuid);
    }

    uint32_t slotIndex = entIt->second;
    EntitySlot& slot   = slots_[slotIndex];

    ComponentType comp{};
    comp.SetHandle(ComponentHandle(UuidGenerator::RandomGenerate()));

    slot.components.emplace_back(std::move(comp));
    uint32_t compIndex = static_cast<uint32_t>(slot.components.size() - 1);

    componentLocationMap_[slot.components.back().GetHandle().uuid] =
        {slotIndex, compIndex};

    slot.components.back().Initialize(_scene, _entity);

    return slot.components.back().GetHandle();
}

template <IsComponent ComponentType>
inline void ComponentArray<ComponentType>::RemoveComponent(ComponentHandle _handle) {
    auto itr = componentLocationMap_.find(_handle.uuid);
    if (itr == componentLocationMap_.end()) {
        return;
    }

    auto [slotIndex, compIndex] = itr->second;
    EntitySlot& slot            = slots_[slotIndex];

    slot.components[compIndex].Finalize();
    slot.components.erase(slot.components.begin() + compIndex);
    componentLocationMap_.erase(itr);

    // index 再割当（同一 entity 内のみ）
    for (uint32_t i = compIndex; i < slot.components.size(); ++i) {
        componentLocationMap_[slot.components[i].GetHandle().uuid] =
            {slotIndex, i};
    }
}

template <IsComponent ComponentType>
inline void ComponentArray<ComponentType>::RemoveComponent(EntityHandle _handle, int32_t _compIndex) {
    auto entIt = entitySlotMap_.find(_handle.uuid);
    if (entIt == entitySlotMap_.end()) {
        return;
    }

    uint32_t slotIndex = entIt->second;
    EntitySlot& slot   = slots_[slotIndex];
    if (_compIndex < 0 || static_cast<size_t>(_compIndex) >= slot.components.size()) {
        return;
    }

    ComponentType& comp = slot.components[_compIndex];
    comp.Finalize();
    componentLocationMap_.erase(comp.GetHandle().uuid);
    slot.components.erase(slot.components.begin() + _compIndex);
    // index 再割当（同一 entity 内のみ）
    for (uint32_t i = _compIndex; i < slot.components.size(); ++i) {
        componentLocationMap_[slot.components[i].GetHandle().uuid] =
            {slotIndex, i};
    }
}

template <IsComponent ComponentType>
inline void ComponentArray<ComponentType>::RemoveAllComponents(EntityHandle _handle) {
    auto entIt = entitySlotMap_.find(_handle.uuid);
    if (entIt == entitySlotMap_.end()) {
        return;
    }
    uint32_t slotIndex = entIt->second;
    EntitySlot& slot   = slots_[slotIndex];
    for (auto& comp : slot.components) {
        comp.Finalize();
        componentLocationMap_.erase(comp.GetHandle().uuid);
    }
    slot.components.clear();
}

template <IsComponent ComponentType>
inline bool ComponentArray<ComponentType>::SaveComponent(ComponentHandle _compHandle, nlohmann::json& _outJson) {
    // エンティティが存在しない場合は失敗
    auto itr = componentLocationMap_.find(_compHandle.uuid);
    if (itr == componentLocationMap_.end()) {
        return false;
    }

    auto [slotIndex, compIndex] = itr->second;
    EntitySlot& slot            = slots_[slotIndex];

    _outJson[nameof<ComponentType>()]           = slot.components[compIndex];
    _outJson[nameof<ComponentType>()]["Handle"] = slot.components[compIndex].GetHandle();

    return true;
}

template <IsComponent ComponentType>
inline bool ComponentArray<ComponentType>::SaveComponent(EntityHandle _handle, int32_t _compIndex, nlohmann::json& _outJson) {
    // エンティティが存在しない場合は失敗
    auto entIt = entitySlotMap_.find(_handle.uuid);
    if (entIt == entitySlotMap_.end()) {
        return false;
    }

    uint32_t slotIndex = entIt->second;
    EntitySlot& slot   = slots_[slotIndex];
    // indexが無効なら 失敗
    if (_compIndex < 0 || static_cast<size_t>(_compIndex) >= slot.components.size()) {
        return false;
    }

    _outJson[nameof<ComponentType>()]           = slot.components[_compIndex];
    _outJson[nameof<ComponentType>()]["Handle"] = slot.components[_compIndex].GetHandle();

    return true;
}

template <IsComponent ComponentType>
inline bool ComponentArray<ComponentType>::SaveComponents(EntityHandle _handle, nlohmann::json& _outJson) {
    // エンティティが存在しない場合は何もしない
    auto entIt = entitySlotMap_.find(_handle.uuid);
    if (entIt == entitySlotMap_.end()) {
        return false;
    }

    // slotの取得
    uint32_t slotIndex = entIt->second;
    EntitySlot& slot   = slots_[slotIndex];

    // コンポーネントを保存
    nlohmann::json compVecJson = nlohmann::json::array();
    for (auto& comp : slot.components) {
        nlohmann::json compJson = comp;
        compJson["Handle"]      = comp.GetHandle();
        compVecJson.emplace_back(comp);
    }

    _outJson[nameof<ComponentType>()] = compVecJson;
}

template <IsComponent ComponentType>
inline ComponentHandle ComponentArray<ComponentType>::LoadComponent(EntityHandle _handle, const nlohmann::json& _inJson) {
    auto itr = entitySlotMap_.find(_handle.uuid);
    if (itr == entitySlotMap_.end()) {
        // エンティティが存在しない場合は何もしない
        LOG_ERROR("Entity not found for ID: {}", uuids::to_string(_handle.uuid));
        return ComponentHandle();
    }

    // slotの取得
    uint32_t slotIndex = itr->second;
    EntitySlot& slot   = slots_[slotIndex];

    // コンポーネントを読み込み
    ComponentType comp         = _inJson.get<ComponentType>();
    ComponentHandle compHandle = ComponentHandle();
    if (_inJson.contains("Handle")) {
        _inJson["Handle"].get<ComponentHandle>();
    } else {
        // Handleがない場合は新規作成
        compHandle = ComponentHandle(UuidGenerator::RandomGenerate());
    }
    comp.SetHandle(compHandle);

    slot.components.push_back(comp);

    return compHandle;
}

template <IsComponent ComponentType>
inline void ComponentArray<ComponentType>::LoadComponents(EntityHandle _handle, const nlohmann::json& _inJson) {
    auto entIt = entitySlotMap_.find(_handle.uuid);
    if (entIt == entitySlotMap_.end()) {
        // エンティティが見つからなかった 場合
        // 新たに登録する
        RegisterEntity(_handle);
        entIt = entitySlotMap_.find(_handle.uuid);
    }

    // slotの取得
    uint32_t slotIndex = entIt->second;
    EntitySlot& slot   = slots_[slotIndex];
    if (!slot.components.empty()) {
        slot.components.clear();
    }

    // コンポーネントを読み込み
    for (const auto& compJson : _inJson) {
        ComponentType comp         = compJson.get<ComponentType>();
        ComponentHandle compHandle = ComponentHandle();
        if (compJson.contains("Handle")) {
            compJson["Handle"].get<ComponentHandle>();
        } else {
            compHandle = ComponentHandle(UuidGenerator::RandomGenerate());
        }
        comp.SetHandle(compHandle);

        slot.components.emplace_back(comp);
    }
}

template <IsComponent ComponentType>
inline ComponentType* ComponentArray<ComponentType>::GetComponent(ComponentHandle _handle) {
    auto itr = componentLocationMap_.find(_handle.uuid);
    if (itr == componentLocationMap_.end()) {
        return nullptr;
    }

    auto [slotIndex, compIndex] = itr->second;
    return &slots_[slotIndex].components[compIndex];
}
template <IsComponent ComponentType>
inline ComponentType* ComponentArray<ComponentType>::GetComponent(EntityHandle _handle, int32_t _compIndex) {
    auto entIt = entitySlotMap_.find(_handle.uuid);
    if (entIt == entitySlotMap_.end()) {
        return nullptr;
    }

    uint32_t slotIndex = entIt->second;
    EntitySlot& slot   = slots_[slotIndex];
    if (_compIndex < 0 || static_cast<size_t>(_compIndex) >= slot.components.size()) {
        return nullptr;
    }

    return &slot.components[_compIndex];
}

template <IsComponent ComponentType>
inline std::vector<ComponentType>& ComponentArray<ComponentType>::GetComponents(EntityHandle _handle) {
    auto entIt = entitySlotMap_.find(_handle.uuid);
    if (entIt == entitySlotMap_.end()) {
        static std::vector<ComponentType> emptyComponents;
        return emptyComponents;
    }

    uint32_t slotIndex = entIt->second;
    EntitySlot& slot   = slots_[slotIndex];

    return slot.components;
}

template <IsComponent ComponentType>
inline IComponent* ComponentArray<ComponentType>::GetIComponent(ComponentHandle _handle) {
    return GetComponent(_handle);
}

template <IsComponent ComponentType>
inline IComponent* ComponentArray<ComponentType>::GetIComponent(EntityHandle _handle, int32_t _compIndex) {
    return GetComponent(_handle, _compIndex);
}

} // namespace OriGine
