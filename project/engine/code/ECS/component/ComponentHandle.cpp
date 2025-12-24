#include "ComponentHandle.h"

using namespace OriGine;

void OriGine::to_json(nlohmann::json& j, const ComponentHandle& c) {
    j = uuids::to_string(c.uuid);
}

void OriGine::from_json(const nlohmann::json& j, ComponentHandle& c) {
    std::string uuidStr;
    j.at("uuid").get_to(uuidStr);
    auto uuid = uuids::uuid::from_string(uuidStr);
    if (uuid.has_value()) {
        c.uuid = uuid.value();
    }
}
