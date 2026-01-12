#include "EntityHandle.h"

/// <summary>
/// エンティティハンドルのシリアライズ
/// </summary>
void OriGine::to_json(nlohmann::json& j, const EntityHandle& handle) {
    j["uuid"] = uuids::to_string(handle.uuid);
}

/// <summary>
/// エンティティハンドルのデシリアライズ
/// </summary>
void OriGine::from_json(const nlohmann::json& j, EntityHandle& handle) {
    std::string uuidStr;
    j.at("uuid").get_to(uuidStr);

    auto uuid = uuids::uuid::from_string(uuidStr);
    if (uuid.has_value()) {
        handle.uuid = uuid.value();
    }
}
