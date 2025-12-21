#pragma once

/// external
#include <uuid/uuid.h>

namespace OriGine {

/// <summary>
/// エンティティのハンドル構造体
/// </summary>
struct EntityHandle {
    uuids::uuid uuid{};

    bool operator==(const EntityHandle& rhs) const {
        return uuid == rhs.uuid;
    }
    bool IsValid() const {
        return !uuid.is_nil();
    }
};

} // namespace OriGine
