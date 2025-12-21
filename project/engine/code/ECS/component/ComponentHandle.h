#pragma once

/// externals
#include "uuid/uuid.h"
#include "nlohmann/json.hpp"

struct ComponentHandle {
    friend void to_json(nlohmann::json& j, const ComponentHandle& c);
    friend void from_json(const nlohmann::json& j, ComponentHandle& c);

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
