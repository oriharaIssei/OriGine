#pragma once

/// external
#include <nlohmann/json.hpp>
#include <uuid/uuid.h>

namespace OriGine {
/// <summary>
/// エンティティのハンドル構造体
/// </summary>
struct EntityHandle {
    EntityHandle() = default;
    EntityHandle(const uuids::uuid& _uuid) : uuid(_uuid) {}

    uuids::uuid uuid{};

    bool operator==(const EntityHandle& rhs) const {
        return uuid == rhs.uuid;
    }
    /// <summary>
    /// 有効なハンドルかどうか
    /// </summary>
    /// <returns>true = 有効 / false = 無効</returns>
    bool IsValid() const {
        return !uuid.is_nil();
    }
};

void to_json(nlohmann::json& j, const EntityHandle& handle);
void from_json(const nlohmann::json& j, EntityHandle& handle);

} // namespace OriGine

namespace std {

template <>
struct hash<OriGine::EntityHandle> {
    std::size_t operator()(const OriGine::EntityHandle& h) const noexcept {
        return std::hash<uuids::uuid>{}(h.uuid);
    }
};

} // namespace std
