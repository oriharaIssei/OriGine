#include "EntityHandle.h"

/// <summary>
/// エンティティハンドルのシリアライズ
/// </summary>
void OriGine::to_json(nlohmann::json& _j, const EntityHandle& _handle) {
    _j["uuid"] = uuids::to_string(_handle.uuid);
}

/// <summary>
/// エンティティハンドルのデシリアライズ
/// </summary>
void OriGine::from_json(const nlohmann::json& _j, EntityHandle& _handle) {
    std::string uuidStr;
    _j.at("uuid").get_to(uuidStr);

    auto uuid = uuids::uuid::from_string(uuidStr);
    if (uuid.has_value()) {
        _handle.uuid = uuid.value();
    }
}
