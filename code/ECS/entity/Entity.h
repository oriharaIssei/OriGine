#pragma once

/// stl
#include <string>

/// ECS
// entity
#include "entity/EntityHandle.h"

/// externals
#include <uuid/uuid.h>

/// math
#include <stdint.h>

namespace OriGine {

/// engine
// ECS
class EntityRepository;

static constexpr int32_t kInvalidEntityID = -1; // 無効なEntity IDを表す値

/// <summary>
/// 実体を表すクラス (実際にはIDでしか無い)
/// </summary>
class Entity {
    friend class EntityRepository;

public:
    Entity(const ::std::string& _dataType, int32_t _id, const EntityHandle& _handle, bool _isUnique = false)
        : dataType_(_dataType), id_(_id), handle_(_handle), isAlive_(true), isUnique_(_isUnique) {}
    Entity() = default;

    ~Entity() = default;

private:
    ::std::string dataType_ = "UNKNOWN"; // Entityの種別を表す文字列

    int32_t id_          = kInvalidEntityID; // 同一dataType_内での識別番号
    EntityHandle handle_ = EntityHandle(); // このEntityを一意に識別するHandle
    bool isAlive_        = false; // 生存フラグ
    bool isUnique_       = false; // シーン内で唯一の存在かどうか
    bool shouldSave_     = true; // シーン保存時に書き出す対象かどうか

public:
    /// <summary>
    /// エンティティハンドルを取得
    /// </summary>
    /// <returns>エンティティハンドル</returns>
    EntityHandle GetHandle() const {
        return handle_;
    }

    /// <summary>
    /// エンティティが生存しているか
    /// </summary>
    /// <returns>生存していればtrue</returns>
    bool IsAlive() const {
        return isAlive_;
    }

    /// <summary>
    /// ユニークなエンティティか
    /// </summary>
    /// <returns>ユニークであればtrue</returns>
    bool IsUnique() const {
        return isUnique_;
    }

    /// <summary>
    /// 保存対象か
    /// </summary>
    /// <returns>保存対象であればtrue</returns>
    bool ShouldSave() const {
        return shouldSave_;
    }

    /// <summary>
    /// 保存対象かどうかを設定
    /// </summary>
    /// <param name="_ShouldSave">保存対象にするか</param>
    void SetShouldSave(bool _ShouldSave) {
        shouldSave_ = _ShouldSave;
    }

    /// <summary>
    /// エンティティのデータタイプを取得する
    /// </summary>
    const ::std::string& GetDataType() const {
        return dataType_;
    }

    /// <summary>
    /// エンティティのデータタイプを設定する
    /// </summary>
    void SetDataType(const ::std::string& _dataType) {
        dataType_ = _dataType;
    }

    /// <summary>
    /// エンティティのユニークIDを取得する
    /// </summary>
    ::std::string GetUniqueID() const {
        return dataType_ + ::std::to_string(id_);
    }
};

} // namespace OriGine
