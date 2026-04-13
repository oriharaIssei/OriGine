#include "ComponentHandle.h"

using namespace OriGine;

void OriGine::to_json(nlohmann::json& _j, const ComponentHandle& _c) {
    _j = uuids::to_string(_c.uuid);
}

void OriGine::from_json(const nlohmann::json& _j, ComponentHandle& _c) {
    std::string uuidStr;
    _j.get_to(uuidStr);
    auto uuid = uuids::uuid::from_string(uuidStr);
    if (uuid.has_value()) {
        _c.uuid = uuid.value();
    }
}
