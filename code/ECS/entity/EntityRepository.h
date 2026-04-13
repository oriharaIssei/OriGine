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
    /// <summary>
    /// コンストラクタ
    /// </summary>
    EntityRepository();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~EntityRepository();

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// Entity 作成
    /// </summary>
    /// <param name="_dataType">エンティティのデータタイプ（モデル名や種類など）</param>
    /// <param name="_unique">ユニークなエンティティとして登録するか</param>
    /// <returns>作成されたエンティティのハンドル</returns>
    EntityHandle CreateEntity(const std::string& _dataType, bool _unique = false);

    /// <summary>
    /// EntityHandleを指定して Entityを 作成
    /// 失敗したら、Handleを上書きして生成する
    /// </summary>
    /// <param name="_handle">指定するハンドル</param>
    /// <param name="_dataType">エンティティのデータタイプ</param>
    /// <param name="_unique">ユニークなエンティティとして登録するか</param>
    /// <returns>作成されたエンティティのハンドル</returns>
    EntityHandle CreateEntity(EntityHandle _handle, const std::string& _dataType, bool _unique = false);

    /// <summary>
    /// Unique Entity 取得
    /// </summary>
    /// <param name="_dataType">UniqueEntityの名前</param>
    /// <returns>UniqueEntityにアクセスするハンドル</returns>
    EntityHandle GetUniqueEntity(const std::string& _dataType);

    /// <summary>
    /// Unique Entity 登録
    /// </summary>
    /// <param name="_entity">登録するエンティティ</param>
    /// <returns>登録に成功すればtrue</returns>
    bool RegisterUniqueEntity(Entity* _entity);

    /// <summary>
    /// Unique Entity 登録解除
    /// </summary>
    /// <param name="_entity">登録解除するエンティティ</param>
    /// <returns>解除に成功すればtrue</returns>
    bool UnregisterUniqueEntity(Entity* _entity);

    /// <summary>
    /// Entity 削除
    /// </summary>
    /// <param name="_handle">削除するエンティティのハンドル</param>
    /// <returns>削除に成功すればtrue</returns>
    bool RemoveEntity(EntityHandle _handle);

    /// <summary>
    /// Entity 取得
    /// </summary>
    /// <param name="_handle">取得したいエンティティのハンドル</param>
    /// <returns>エンティティのポインタ。見つからなければnullptr</returns>
    Entity* GetEntity(EntityHandle _handle);

    /// <summary>
    /// Entity 取得 (const)
    /// </summary>
    /// <param name="_handle">取得したいエンティティのハンドル</param>
    /// <returns>エンティティのポインタ。見つからなければnullptr</returns>
    const Entity* GetEntity(EntityHandle _handle) const;

    /// <summary>
    /// 生存チェック
    /// </summary>
    /// <param name="_handle">確認したいエンティティのハンドル</param>
    /// <returns>生存していればtrue</returns>
    bool IsAlive(EntityHandle _handle) const;

    /// <summary>
    /// 全エンティティを削除する
    /// </summary>
    void Clear();

private:
    // --- 内部 ---

    /// <summary>
    /// EntityIndex の 確保
    /// </summary>
    /// <returns>確保されたEntityIndex</returns>
    int32_t AllocateIndex();

    /// <summary>
    /// UUID から EntityIndex を探す
    /// </summary>
    /// <param name="_uuid">検索するUUID</param>
    /// <returns>見つかったEntityIndex。見つからなければ-1</returns>
    int32_t FindIndex(const uuids::uuid& _uuid) const;

private:
    uint32_t size_ = 10000;

    std::vector<Entity> entities_;
    BitArray<uint64_t> entityActiveBits_;

    std::unordered_map<uuids::uuid, int32_t> uuidToIndex_;
    std::unordered_map<std::string, uuids::uuid> uniqueEntities_;

public:
    /// <summary>
    /// 生存しているエンティティ数を取得
    /// </summary>
    /// <returns>生存エンティティ数</returns>
    size_t GetEntityCount() const { return entityActiveBits_.GetTrueCount(); }

    /// <summary>
    /// 収容可能なエンティティの最大数を取得
    /// </summary>
    /// <returns>最大エンティティ数</returns>
    uint32_t GetCapacity() const { return static_cast<uint32_t>(entities_.size()); }

    /// <summary>
    /// 全エンティティ配列を取得
    /// </summary>
    /// <returns>エンティティ配列</returns>
    const std::vector<Entity>& GetEntities() const { return entities_; }

    /// <summary>
    /// 全エンティティ配列をリファレンスで取得
    /// </summary>
    /// <returns>エンティティ配列のリファレンス</returns>
    std::vector<Entity>& GetEntitiesRef() { return entities_; }
};

} // namespace OriGine
