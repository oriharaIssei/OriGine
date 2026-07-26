#pragma once

/// external
#include <nlohmann/json.hpp>
#include <uuid/uuid.h>

namespace OriGine {
/// <summary>
/// エンティティのハンドル構造体。
/// 生ポインタや配列インデックスではなく、生成時に割り当てたランダムUUIDだけでEntityを指す。
/// 「インデックス + 世代カウンタ」方式の代わりにUUIDを採用しているのは、
/// EntityRepository側が uuid -> プールインデックス の対応表を持ち、
/// Entity削除時にその対応表からエントリを消すだけで済むため。
/// 削除後に古いHandleを使ってアクセスしようとしても対応表に見つからずnullptr/falseが返るので、
/// 世代カウンタを別途インクリメント管理しなくても「削除済みエンティティへのダングリング参照」を検出できる。
/// </summary>
struct EntityHandle {
    EntityHandle() = default;
    EntityHandle(const uuids::uuid& _uuid) : uuid(_uuid) {}

    uuids::uuid uuid{}; // このHandleが指すEntityの一意なID

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
