#pragma once

/// externals
#include "nlohmann/json.hpp"
#include "uuid/uuid.h"

namespace OriGine {

struct ComponentHandle {
    friend void to_json(nlohmann::json& _j, const ComponentHandle& _c);
    friend void from_json(const nlohmann::json& _j, ComponentHandle& _c);

    ComponentHandle() : uuid() {}
    ComponentHandle(const uuids::uuid& _uuid) : uuid(_uuid) {}

    uuids::uuid uuid;
    bool operator==(const ComponentHandle& _other) const {
        return uuid == _other.uuid;
    }
    bool operator!=(const ComponentHandle& _other) const {
        return !(*this == _other);
    }

    bool IsValid() const {
        return !uuid.is_nil();
    }
};

} // namespace OriGine

namespace std {

template <>
struct hash<OriGine::ComponentHandle> {
    std::size_t operator()(const OriGine::ComponentHandle& _h) const noexcept {
        return std::hash<uuids::uuid>{}(_h.uuid);
    }
};

} // namespace std
