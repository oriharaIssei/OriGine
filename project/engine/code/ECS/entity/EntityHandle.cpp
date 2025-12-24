#include "EntityHandle.h"

void OriGine::to_json(nlohmann::json& j, const EntityHandle& handle) {
    j["uuid"] = uuids::to_string(handle.uuid);
}

void OriGine::from_json(const nlohmann::json& j, EntityHandle& handle) {
    std::string uuidStr;
    j.get_to(uuidStr);
    auto uuid = uuids::uuid::from_string(uuidStr);
    if (uuid.has_value()) {
        handle.uuid = uuid.value();
    }
}
