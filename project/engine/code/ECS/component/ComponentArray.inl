#pragma once
// このファイルは ComponentArray.h からインクルードされます。
// 単独でインクルードしないでください。

namespace OriGine {

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
    auto itr = entitySlotMap_.find(_entity.uuid);
    if (itr == entitySlotMap_.end()) {
        return false;
    }

    uint32_t slotIndex     = itr->second;
    const EntitySlot& slot = slots_[slotIndex];

    if (!slot.alive || slot.components.empty()) {
        return false;
    }

    return true;
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
inline ComponentHandle ComponentArray<ComponentType>::InsertComponent(Scene* _scene, EntityHandle _entity, uint32_t _compIndex) {
    auto entIt = entitySlotMap_.find(_entity.uuid);
    if (entIt == entitySlotMap_.end()) {
        RegisterEntity(_entity);
        entIt = entitySlotMap_.find(_entity.uuid);
    }

    uint32_t slotIndex = entIt->second;
    EntitySlot& slot   = slots_[slotIndex];

    ComponentType comp{};
    ComponentHandle compHandle = ComponentHandle(UuidGenerator::RandomGenerate());
    comp.SetHandle(compHandle);
    comp.Initialize(_scene, _entity);

    // index調整
    _compIndex = std::clamp<uint32_t>(static_cast<uint32_t>(_compIndex), 0, static_cast<uint32_t>(slot.components.size()));
    // コンポーネント挿入
    slot.components.insert(slot.components.begin() + _compIndex, std::move(comp));

    // handleの再配置
    for (uint32_t i = static_cast<uint32_t>(_compIndex); i < static_cast<uint32_t>(slot.components.size()); ++i) {
        componentLocationMap_[slot.components[i].GetHandle().uuid] =
            {slotIndex, i};
    }

    return compHandle;
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
inline void ComponentArray<ComponentType>::RemoveComponent(EntityHandle _handle, uint32_t _compIndex) {
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
inline bool ComponentArray<ComponentType>::SaveComponent(EntityHandle _handle, uint32_t _compIndex, nlohmann::json& _outJson) {
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
        compVecJson.emplace_back(compJson);
    }

    _outJson[nameof<ComponentType>()] = compVecJson;
    return true;
}

template <IsComponent ComponentType>
inline ComponentHandle ComponentArray<ComponentType>::LoadComponent(
    EntityHandle _handle,
    const nlohmann::json& _inJson,
    HandleAssignMode _handleMode) {
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
    if (_handleMode == HandleAssignMode::UseSaved && _inJson.contains("Handle")) {
        _inJson["Handle"].get_to<ComponentHandle>(compHandle);
    } else {
        // 新規Handle生成
        compHandle = ComponentHandle(UuidGenerator::RandomGenerate());
    }
    comp.SetHandle(compHandle);

    slot.components.push_back(comp);

    return compHandle;
}

template <IsComponent ComponentType>
inline ComponentHandle ComponentArray<ComponentType>::LoadComponent(
    EntityHandle _handle,
    uint32_t _compIndex,
    const nlohmann::json& _inJson,
    HandleAssignMode _handleMode) {
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
    // コンポーネントを読み込み
    ComponentType comp         = _inJson.get<ComponentType>();
    ComponentHandle compHandle = ComponentHandle();
    if (_handleMode == HandleAssignMode::UseSaved && _inJson.contains("Handle")) {
        _inJson["Handle"].get_to<ComponentHandle>(compHandle);
    } else {
        compHandle = ComponentHandle(UuidGenerator::RandomGenerate());
    }
    comp.SetHandle(compHandle);

    // index調整
    _compIndex = std::clamp<uint32_t>(_compIndex, 0, static_cast<uint32_t>(slot.components.size()));
    // コンポーネント挿入
    slot.components.insert(slot.components.begin() + _compIndex, std::move(comp));

    // handleの再配置
    for (uint32_t i = _compIndex; i < static_cast<uint32_t>(slot.components.size()); ++i) {
        componentLocationMap_[slot.components[i].GetHandle().uuid] =
            {slotIndex, i};
    }

    return compHandle;
}

template <IsComponent ComponentType>
inline void ComponentArray<ComponentType>::LoadComponents(
    EntityHandle _handle,
    const nlohmann::json& _inJson,
    HandleAssignMode _handleMode) {
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
        if (_handleMode == HandleAssignMode::UseSaved && compJson.contains("Handle")) {
            compJson["Handle"].get_to<ComponentHandle>(compHandle);
        } else {
            compHandle = ComponentHandle(UuidGenerator::RandomGenerate());
        }
        comp.SetHandle(compHandle);

        slot.components.emplace_back(comp);

        componentLocationMap_[comp.GetHandle().uuid] =
            {slotIndex, static_cast<uint32_t>(slot.components.size() - 1)};
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
inline ComponentType* ComponentArray<ComponentType>::GetComponent(EntityHandle _handle, uint32_t _compIndex) {
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
inline IComponent* ComponentArray<ComponentType>::GetIComponent(EntityHandle _handle, uint32_t _compIndex) {
    return GetComponent(_handle, _compIndex);
}

template <IsComponent ComponentType>
inline std::vector<IComponent*> ComponentArray<ComponentType>::GetIComponents(EntityHandle _handle) {
    auto entIt = entitySlotMap_.find(_handle.uuid);
    if (entIt == entitySlotMap_.end()) {
        static std::vector<IComponent*> emptyIComponents;
        return emptyIComponents;
    }

    uint32_t slotIndex = entIt->second;
    EntitySlot& slot   = slots_[slotIndex];

    static std::vector<IComponent*> iComponents;
    iComponents.clear();
    for (auto& comp : slot.components) {
        iComponents.push_back(&comp);
    }
    return iComponents;
}

template <IsComponent ComponentType>
inline uint32_t ComponentArray<ComponentType>::GetComponentCount(EntityHandle _handle) const {
    auto entIt = entitySlotMap_.find(_handle.uuid);
    if (entIt == entitySlotMap_.end()) {
        return 0;
    }

    uint32_t slotIndex     = entIt->second;
    const EntitySlot& slot = slots_[slotIndex];
    return static_cast<uint32_t>(slot.components.size());
}

} // namespace OriGine
