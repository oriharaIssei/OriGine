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

    bool operator==(const EntityHandle& _rhs) const {
        return uuid == _rhs.uuid;
    }
    bool operator!=(const EntityHandle& _rhs) const {
        return !(*this == _rhs);
    }

    bool operator<(const EntityHandle& _rhs) const {
        return uuid < _rhs.uuid;
    }
    bool operator<=(const EntityHandle& _rhs) const {
        return (*this < _rhs) || (*this == _rhs);
    }

    /// <summary>
    /// 有効なハンドルかどうか
    /// </summary>
    /// <returns>true = 有効 / false = 無効</returns>
    bool IsValid() const {
        return !uuid.is_nil();
    }
};

/// <summary>
/// エンティティハンドルのシリアライズ
/// </summary>
/// <param name="_j">jsonオブジェクト</param>
/// <param name="_handle">シリアライズするエンティティハンドル</param>
void to_json(nlohmann::json& _j, const EntityHandle& _handle);

/// <summary>
/// エンティティハンドルのデシリアライズ
/// </summary>
/// <param name="_j">jsonオブジェクト</param>
/// <param name="_handle">デシリアライズ先のエンティティハンドル</param>
void from_json(const nlohmann::json& _j, EntityHandle& _handle);

} // namespace OriGine

namespace std {

template <>
struct hash<OriGine::EntityHandle> {
    /// <summary>
    /// ハッシュ関数のオーバーロード
    /// </summary>
    /// <param name="_h">エンティティハンドル</param>
    /// <returns>ハッシュ値</returns>
    std::size_t operator()(const OriGine::EntityHandle& _h) const noexcept {
        return std::hash<uuids::uuid>{}(_h.uuid);
    }
};

} // namespace std
