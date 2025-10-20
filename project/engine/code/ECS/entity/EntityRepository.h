#pragma once

class Entity;

/// stl
#include <string>
#include <unordered_map>
#include <vector>

/// util
#include "util/BitArray.h"

/// math
#include <stdint.h>

/// <summary>
/// Entity Repository(登録, 削除, 取得などを行う)
/// </summary>
/// <summary>
/// Entity Repository(登録, 削除, 取得などを行う)
/// </summary>
class EntityRepository final {
public:
    EntityRepository();
    ~EntityRepository();

    void Initialize();
    void Finalize();

    void resize(uint32_t _newSize);

    /// <summary>
    /// Entity の取得
    /// </summary>
    Entity* getEntity(int32_t _entityIndex);
    /// <summary>
    /// UniqueEntity の取得
    /// </summary>
    Entity* getUniqueEntity(const std::string& _dataTypeName);

    /// <summary>
    /// EntityIndex の 確保
    /// </summary>
    int32_t allocateEntity();

    /// <summary>
    /// UniqueEntity に 登録する
    /// </summary>
    bool registerUniqueEntity(Entity* _entity);
    /// <summary>
    /// UniqueEntity を削除する
    /// </summary>
    bool unregisterUniqueEntity(const std::string& _dataTypeName);

    /// <summary>
    /// Entity を登録する
    /// </summary>
    int32_t CreateEntity(const std::string& _dataType, bool _isUnique = false);
    /// <summary>
    /// Entity を指定したIndexに登録する
    /// </summary>
    int32_t CreateEntity(int32_t _id, const std::string& _dataType, bool _isUnique = false);

    /// <summary>
    /// Entity を削除する
    /// </summary>
    bool removeEntity(int32_t _entityIndex);

    uint32_t getSize() const;
    uint32_t getActiveEntityCount() const;
    uint32_t getInactiveEntityCount() const;
    void clear();

    const std::vector<Entity>& getEntities() const;
    std::vector<Entity>& getEntitiesRef();

private:
    uint32_t size_ = 10000;
    std::vector<Entity> entities_;
    std::unordered_map<std::string, int32_t> uniqueEntityIDs_;
    BitArray<uint64_t> entityActiveBits_;
};
