#pragma once

/// stl
#include <vector>

/// ECS
// entity
#include "entity/EntityHandle.h"
// component
#include "ComponentHandle.h"
#include "ECS/HandleAssignMode.h"

/// externals
#include <nlohmann/json_fwd.hpp>

namespace OriGine {

class Scene;
class IComponent;

static constexpr uint32_t kDefaultComponentArraySize = 128;

//============================================================
// IComponentArray
//============================================================
/// <summary>
/// コンポーネント配列インターフェース
/// </summary>
class IComponentArray {
public:
    virtual ~IComponentArray() = default;

    virtual void Initialize(uint32_t _reserveSize = kDefaultComponentArraySize) = 0;
    virtual void Finalize()                                                     = 0;

    /// <summary>
    /// Entity登録
    /// </summary>
    /// <param name="_entity"></param>
    virtual void RegisterEntity(EntityHandle _entity) = 0;
    /// <summary>
    /// Entity登録解除
    /// </summary>
    /// <param name="_entity"></param>
    virtual void UnregisterEntity(EntityHandle _entity) = 0;
    /// <summary>
    /// Entityが登録されたいるか
    /// </summary>
    /// <param name="_entity"></param>
    /// <returns></returns>
    virtual bool HasEntity(EntityHandle _entity) const = 0;

    /// <summary>
    /// Componentの追加
    /// </summary>
    /// <param name="_scene"></param>
    /// <param name="_entity"></param>
    /// <returns></returns>
    virtual ComponentHandle AddComponent(Scene* _scene, EntityHandle _entity) = 0;

    /// <summary>
    /// Componentの挿入追加 (indexがsize以上なら最後尾に追加)
    /// </summary>
    /// <param name="_entity"></param>
    /// <param name="_compIndex"></param>
    /// <returns></returns>
    virtual ComponentHandle InsertComponent(Scene* _scene, EntityHandle _entity, uint32_t _compIndex) = 0;

    /// <summary>
    /// Componentの削除
    /// </summary>
    /// <param name="_component"></param>
    virtual void RemoveComponent(ComponentHandle _handle) = 0;
    /// <summary>
    /// Componentの削除
    /// </summary>
    /// <param name="_component"></param>
    virtual void RemoveComponent(EntityHandle _handle, uint32_t _compIndex) = 0;

    /// <summary>
    /// Entityが所有するComponent全ての削除
    /// </summary>
    /// <param name="_handle"></param>
    virtual void RemoveAllComponents(EntityHandle _handle) = 0;

    /// <summary>
    /// 指定したComponentを保存する
    /// </summary>
    /// <param name="_compHandle"></param>
    /// <param name="_outJson">保存先</param>
    virtual bool SaveComponent(ComponentHandle _compHandle, nlohmann::json& _outJson) = 0;
    /// <summary>
    /// 指定したComponentを保存する
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_compIndex"></param>
    /// <param name="_outJson">保存先</param>
    virtual bool SaveComponent(EntityHandle _handle, uint32_t _compIndex, nlohmann::json& _outJson) = 0;

    /// <summary>
    /// 指定されたEntityが持つComponent全てを保存する
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_outJson">保存先</param>
    virtual bool SaveComponents(EntityHandle _handle, nlohmann::json& _outJson) = 0;

    /// <summary>
    /// JsonからComponentを復元し、Entityに追加する
    /// (初期化はしない)
    /// </summary>
    /// <param name="_handle">追加さき</param>
    /// <param name="_inJson">復元もと</param>
    /// <param name="_handleMode">Handleの割り当て方法 (デフォルト: UseSaved)</param>
    /// <returns>復元されたComponentのHandle</returns>
    virtual ComponentHandle LoadComponent(
        EntityHandle _handle,
        const nlohmann::json& _inJson,
        HandleAssignMode _handleMode = HandleAssignMode::UseSaved) = 0;

    /// <summary>
    /// JsonからComponentを復元し、Entityに挿入する
    /// </summary>
    /// <param name="_handle">追加さき</param>
    /// <param name="_compIndex">挿入先</param>
    /// <param name="_inJson">復元もと</param>
    /// <param name="_handleMode">Handleの割り当て方法 (デフォルト: UseSaved)</param>
    /// <returns>復元されたComponentのHandle</returns>
    virtual ComponentHandle LoadComponent(
        EntityHandle _handle,
        uint32_t _compIndex,
        const nlohmann::json& _inJson,
        HandleAssignMode _handleMode = HandleAssignMode::UseSaved) = 0;

    /// <summary>
    /// Jsonから全てのComponentを復元し、Entityに追加する。
    /// </summary>
    /// <param name="_handle">追加さき</param>
    /// <param name="_inJson">復元もと</param>
    /// <param name="_handleMode">Handleの割り当て方法 (デフォルト: UseSaved)</param>
    virtual void LoadComponents(
        EntityHandle _handle,
        const nlohmann::json& _inJson,
        HandleAssignMode _handleMode = HandleAssignMode::UseSaved) = 0;

    /// <summary>
    /// Componentの取得 (IComponent)
    /// </summary>
    /// <param name="_component"></param>
    /// <returns></returns>
    virtual IComponent* GetIComponent(ComponentHandle _handle) = 0;
    /// <summary>
    /// Componentの取得 (IComponent)
    /// </summary>
    /// <param name="_component"></param>
    /// <returns></returns>
    virtual IComponent* GetIComponent(EntityHandle _handle, uint32_t _compIndex) = 0;

    /// <summary>
    /// 指定したEntityが所有する全てのIComponentを取得する
    /// </summary>
    /// <param name="_handle"></param>
    /// <returns></returns>
    virtual std::vector<IComponent*> GetIComponents(EntityHandle _handle) = 0;

    /// <summary>
    /// 指定したEntityが所有するComponent数を取得する
    /// </summary>
    /// <param name="_handle"></param>
    /// <returns></returns>
    virtual uint32_t GetComponentCount(EntityHandle _handle) const = 0;
};

} // namespace OriGine
