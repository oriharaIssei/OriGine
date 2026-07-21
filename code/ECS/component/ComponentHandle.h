#pragma once

/// externals
#include "nlohmann/json.hpp"
#include "uuid/uuid.h"

namespace OriGine {

/// <summary>
/// Componentを一意に識別するためのハンドル。
/// 内部的にはuuidをラップし、Component検索・保存/復元のキーとして使用する。
/// </summary>
struct ComponentHandle {
    friend void to_json(nlohmann::json& _j, const ComponentHandle& _c);
    friend void from_json(const nlohmann::json& _j, ComponentHandle& _c);

    ComponentHandle() : uuid() {}
    ComponentHandle(const uuids::uuid& _uuid) : uuid(_uuid) {}

    uuids::uuid uuid; // このHandleが指すComponentの一意なID
    bool operator==(const ComponentHandle& _other) const {
        return uuid == _other.uuid;
    }
    bool operator!=(const ComponentHandle& _other) const {
        return !(*this == _other);
    }

    /// <summary>
    /// 有効なComponentを指しているか(nil uuidでないか)を判定する
    /// </summary>
    bool IsValid() const {
        return !uuid.is_nil();
    }
};

} // namespace OriGine

namespace std {

// ComponentHandleをunordered_map等のキーとして使用するためのハッシュ特殊化
template <>
struct hash<OriGine::ComponentHandle> {
    std::size_t operator()(const OriGine::ComponentHandle& _h) const noexcept {
        return std::hash<uuids::uuid>{}(_h.uuid);
    }
};

} // namespace std
