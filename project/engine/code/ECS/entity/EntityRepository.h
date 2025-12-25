#pragma once

/// stl
#include <string>
#include <unordered_map>
#include <vector>

/// ECS
// entity
#include "Entity.h"
#include "EntityHandle.h"

/// util
#include "util/BitArray.h"

/// external
#include <uuid/uuid.h>

/// math
#include <stdint.h>

namespace OriGine {
/// <summary>
/// Entity Repository(登録, 削除, 取得などを行う)
/// </summary>
class EntityRepository final {
public:
    EntityRepository();
    ~EntityRepository();

    void Initialize();
    void Finalize();

    /// <summary>
    /// Entity 作成
    /// </summary>
    /// <param name="dataType">エンティティの名前</param>
    /// <param name="unique"></param>
    /// <returns></returns>
    EntityHandle CreateEntity(const std::string& _dataType, bool _unique = false);

    /// <summary>
    /// EntityHandleを指定して Entityを 作成
    /// 失敗したら、Handleを上書きして生成する
    /// </summary>
    /// <param name="dataType">エンティティの名前</param>
    /// <param name="unique"></param>
    /// <returns></returns>
    EntityHandle CreateEntity(EntityHandle _handle, const std::string& _dataType, bool _unique = false);

    /// <summary>
    /// Unique Entity 取得
    /// </summary>
    /// <param name="dataType">UniqueEntityの名前</param>
    /// <returns>UniqueEntityにアクセスするハンドル</returns>
    EntityHandle GetUniqueEntity(const std::string& _dataType);

    /// <summary>
    /// Unique Entity 登録
    /// </summary>
    /// <param name="_dataType">登録するEntityName</param>
    /// <param name="_handle"></param>
    /// <returns></returns>
    bool RegisterUniqueEntity(Entity* _entity);
    /// <summary>
    /// Unique Entity 登録解除
    /// </summary>
    /// <param name="_entity"></param>
    /// <returns></returns>
    bool UnregisterUniqueEntity(Entity* _entity);

    /// <summary>
    /// Entity 削除
    /// </summary>
    /// <param name="handle"></param>
    /// <returns></returns>
    bool RemoveEntity(EntityHandle _handle);

    /// <summary>
    /// Entity 取得
    /// </summary>
    /// <param name="handle"></param>
    /// <returns></returns>
    Entity* GetEntity(EntityHandle handle);
    /// <summary>
    /// Entity 取得 (const)
    /// </summary>
    /// <param name="handle"></param>
    /// <returns></returns>
    const Entity* GetEntity(EntityHandle handle) const;

    /// <summary>
    /// 生存チェック
    /// </summary>
    /// <param name="handle"></param>
    /// <returns></returns>
    bool IsAlive(EntityHandle handle) const;

    /// <summary>
    /// 全エンティティを削除する
    /// </summary>
    void Clear();

private:
    // --- 内部 ---

    /// <summary>
    /// EntityIndex の 確保
    /// </summary>
    /// <returns>EntityIndex</returns>
    int32_t AllocateIndex();
    /// <summary>
    /// UUID から EntityIndex を探す
    /// </summary>
    /// <param name="uuid"></param>
    /// <returns>EntityIndex</returns>
    int32_t FindIndex(const uuids::uuid& uuid) const;

private:
    uint32_t size_ = 10000;

    std::vector<Entity> entities_;
    BitArray<uint64_t> entityActiveBits_;

    std::unordered_map<uuids::uuid, int32_t> uuidToIndex_;
    std::unordered_map<std::string, uuids::uuid> uniqueEntities_;

public:
    size_t GetEntityCount() const { return entityActiveBits_.GetTrueCount(); }
    uint32_t GetCapacity() const { return static_cast<uint32_t>(entities_.size()); }

    const std::vector<Entity>& GetEntities() const { return entities_; }
    std::vector<Entity>& GetEntitiesRef() { return entities_; }
};

} // namespace OriGine
