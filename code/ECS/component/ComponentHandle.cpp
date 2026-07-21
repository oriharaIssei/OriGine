#include "ComponentHandle.h"

using namespace OriGine;

/// <summary>
/// ComponentHandleを文字列化したuuidとしてjsonへ書き出す
/// </summary>
void OriGine::to_json(nlohmann::json& _j, const ComponentHandle& _c) {
    _j = uuids::to_string(_c.uuid);
}

/// <summary>
/// jsonの文字列からuuidを復元してComponentHandleへ書き戻す
/// </summary>
void OriGine::from_json(const nlohmann::json& _j, ComponentHandle& _c) {
    std::string uuidStr;
    _j.get_to(uuidStr);
    auto uuid = uuids::uuid::from_string(uuidStr);
    if (uuid.has_value()) {
        // 変換失敗時(不正な文字列など)はuuidを更新せず既定値のままにする
        _c.uuid = uuid.value();
    }
}
