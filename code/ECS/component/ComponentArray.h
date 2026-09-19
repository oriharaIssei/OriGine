#pragma once

/// stl
#include <cassert>
#include <unordered_map>
#include <vector>

/// util
#include "util/container/DenseSlotMap.h"

/// ECS
// entity
#include "entity/EntityHandle.h"
// component
#include "ComponentHandle.h"
#include "ECS/HandleAssignMode.h"
#include "IComponent.h"
#include "IComponentArray.h"

/// profiler
#include "profiler/CallCounter.h"

/// externals
#include "logger/Logger.h"
#include "uuidGenerator/UuidGenerator.h"
#include <uuid/uuid.h>

namespace OriGine {

/// <summary>
/// ComponentType 1つ分を JSON へ書き出す(Phase 3 D-1/D-4)。`kUsesDescriptorSerialization<ComponentType>`
/// が true の型は、型ID→ディスクリプタを引いて表経由(ToJsonViaDescriptor)で書く。それ以外は
/// 従来どおり ADL の to_json に任せる。
/// D-4 で表経由の型の手書き to_json/from_json を削除したため、表経由の型は ADL へフォールバック
/// できない(そもそも定義が無くコンパイルが通らない)。型IDが未採番/ディスクリプタ未登録という
/// 想定外の順序で呼ばれた場合でも、黙って壊れたデータを返さないよう LOG_ERROR を出したうえで
/// 空オブジェクトを返す(計測器と同じ原則: 捨てたことを数えて出す)。
/// </summary>
template <IsComponent ComponentType>
inline nlohmann::json SerializeComponentValue(const ComponentType& _comp) {
    if constexpr (kUsesDescriptorSerialization<ComponentType>) {
        const uint32_t typeId = ComponentTypeIdStorage<ComponentType>::id_;
        const TypeDesc* desc  = (typeId != kInvalidComponentTypeId) ? GetTypeDescriptor(typeId) : nullptr;
        if (desc) {
            nlohmann::json json = nlohmann::json::object();
            ToJsonViaDescriptor(json, &_comp, *desc);
            return json;
        }
        LOG_ERROR("SerializeComponentValue<{}>: ディスクリプタが未登録のため空オブジェクトを書き出す(手書きto_jsonはD-4で削除済み)", nameof<ComponentType>());
        return nlohmann::json::object();
    } else {
        return nlohmann::json(_comp);
    }
}

/// <summary>SerializeComponentValue の読み込み版。既定構築済みの _outComp に、JSON にあるフィールドだけ上書きする。</summary>
template <IsComponent ComponentType>
inline void DeserializeComponentValue(const nlohmann::json& _json, ComponentType& _outComp) {
    if constexpr (kUsesDescriptorSerialization<ComponentType>) {
        const uint32_t typeId = ComponentTypeIdStorage<ComponentType>::id_;
        const TypeDesc* desc  = (typeId != kInvalidComponentTypeId) ? GetTypeDescriptor(typeId) : nullptr;
        if (desc) {
            FromJsonViaDescriptor(_json, &_outComp, *desc);
            return;
        }
        LOG_ERROR("DeserializeComponentValue<{}>: ディスクリプタが未登録のため読み込みを行わない(手書きfrom_jsonはD-4で削除済み)", nameof<ComponentType>());
    } else {
        _outComp = _json.get<ComponentType>();
    }
}

/// <summary>
/// コンポーネント配列。
/// ComponentType ごとに実体化されるテンプレートクラスで、実データ(vector&lt;ComponentType&gt;)を型付きのまま保持する。
/// 外部(ComponentRepositoryなど)からは基底の IComponentArray インターフェース越しにしか触れないため、
/// 呼び出し側は具体的な型を知らなくても AddComponent/GetIComponent 等で扱える(=型消去)。
/// 型が必要な箇所(GetComponent等)だけこのテンプレートを直接使い、静的にComponentType*を返す。
/// </summary>
/// <typeparam name="ComponentType"></typeparam>
template <IsComponent ComponentType>
class ComponentArray final
    : public IComponentArray {
public:
    ComponentArray()           = default;
    ~ComponentArray() override = default;

    // ────────────────────────────────
    //  lifecycle
    // ────────────────────────────────
    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="_reserveSize">初期Arrayサイズ</param>
    void Initialize(uint32_t _reserveSize = kDefaultComponentArraySize) override;
    /// <summary>
    /// 終了化処理
    /// </summary>
    void Finalize() override;

    // ────────────────────────────────
    //  entity
    // ────────────────────────────────
    /// <summary>
    /// Entity登録
    /// </summary>
    void RegisterEntity(const EntityHandle& _entity) override;
    /// <summary>
    /// Entity登録解除
    /// </summary>
    /// <param name="_scene"></param>
    /// <param name="_entity"></param>
    void UnregisterEntity(const EntityHandle& _entity) override;

    /// <summary>
    /// Entityが登録されているか
    /// </summary>
    /// <param name="_entity"></param>
    /// <returns></returns>
    bool HasEntity(const EntityHandle& _entity) const override;

    // ────────────────────────────────
    //  component
    // ────────────────────────────────
    /// <summary>
    /// Componentの追加
    /// </summary>
    ComponentHandle AddComponent(Scene* _scene, const EntityHandle& _entity) override;

    /// <summary>
    /// Componentの挿入追加 (indexがsize以上なら最後尾に追加)
    /// </summary>
    /// <param name="_entity"></param>
    /// <param name="_compIndex"></param>
    /// <returns></returns>
    ComponentHandle InsertComponent(Scene* _scene, const EntityHandle& _entity, uint32_t _compIndex) override;

    /// <summary>
    /// Componentの削除
    /// </summary>
    /// <param name="_component"></param>
    void RemoveComponent(ComponentHandle _handle) override;
    /// <summary>
    /// Componentの削除(非推奨)
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_compIndex"></param>
    void RemoveComponent(const EntityHandle& _handle, uint32_t _compIndex = 0) override;

    /// <summary>
    /// Entityが所有するComponent全ての削除
    /// </summary>
    /// <param name="_handle"></param>
    void RemoveAllComponents(const EntityHandle& _handle) override;

    // ────────────────────────────────
    //  serialization
    // ────────────────────────────────
    /// <summary>
    /// 指定したComponentを保存する
    /// </summary>
    /// <param name="_compHandle"></param>
    /// <param name="_outJson">保存先</param>
    bool SaveComponent(ComponentHandle _compHandle, nlohmann::json& _outJson) override;
    /// <summary>
    /// 指定したComponentを保存する
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_compIndex"></param>
    /// <param name="_outJson">保存先</param>
    bool SaveComponent(const EntityHandle& _handle, uint32_t _compIndex, nlohmann::json& _outJson) override;

    /// <summary>
    /// 指定されたEntityが持つComponent全てを保存する
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_outJson">保存先</param>
    bool SaveComponents(const EntityHandle& _handle, nlohmann::json& _outJson) override;

    /// <summary>
    /// JsonからComponentを復元し、Entityに追加する
    /// </summary>
    /// <param name="_handle">追加さき</param>
    /// <param name="_inJson">復元もと</param>
    /// <param name="_handleMode">Handleの割り当て方法 (デフォルト: UseSaved)</param>
    /// <returns>復元されたComponentのHandle</returns>
    ComponentHandle LoadComponent(
        const EntityHandle& _handle,
        const nlohmann::json& _inJson,
        HandleAssignMode _handleMode = HandleAssignMode::UseSaved) override;

    /// <summary>
    /// JsonからComponentを復元し、Entityに挿入する
    /// </summary>
    /// <param name="_handle">追加さき</param>
    /// <param name="_compIndex">挿入先</param>
    /// <param name="_inJson">復元もと</param>
    /// <param name="_handleMode">Handleの割り当て方法 (デフォルト: UseSaved)</param>
    /// <returns>復元されたComponentのHandle</returns>
    ComponentHandle LoadComponent(
        const EntityHandle& _handle,
        uint32_t _compIndex,
        const nlohmann::json& _inJson,
        HandleAssignMode _handleMode = HandleAssignMode::UseSaved) override;

    /// <summary>
    /// Jsonから全てのComponentを復元し、Entityに追加する。
    /// </summary>
    /// <param name="_handle"></param>
    /// <param name="_inJson"></param>
    /// <param name="_handleMode">Handleの割り当て方法 (デフォルト: UseSaved)</param>
    void LoadComponents(
        const EntityHandle& _handle,
        const nlohmann::json& _inJson,
        HandleAssignMode _handleMode = HandleAssignMode::UseSaved) override;

    /// <summary>
    /// 指定したEntityが所有する、この型の全てのComponentを初期化する
    /// (要素は具象型ComponentTypeのまま扱うため、IComponent*経由の仮想呼び出しは発生しない)
    /// </summary>
    /// <param name="_scene"></param>
    /// <param name="_handle"></param>
    void InitializeComponents(Scene* _scene, const EntityHandle& _handle) override;

    // ────────────────────────────────
    //  getters
    // ────────────────────────────────
    /// <summary>
    /// Componentの取得
    /// </summary>
    /// <param name="_component"></param>
    /// <returns></returns>
    ComponentType* GetComponent(ComponentHandle _handle);
    /// <summary>
    /// Componentの取得
    /// </summary>
    /// <param name="_component"></param>
    /// <returns></returns>
    ComponentType* GetComponent(const EntityHandle& _handle, uint32_t _compIndex = 0);

    /// <summary>
    /// Entityが所有するComponent全ての取得
    /// </summary>
    /// <param name="_handle"></param>
    /// <returns></returns>
    std::vector<ComponentType>& GetComponents(const EntityHandle& _handle);

    /// <summary>
    /// Componentの取得 (IComponent版)
    /// </summary>
    /// <param name="_component"></param>
    /// <returns></returns>
    IComponent* GetIComponent(ComponentHandle _handle) override;
    /// <summary>
    /// Componentの取得 (IComponent版)
    /// </summary>
    /// <param name="_component"></param>
    /// <returns></returns>
    IComponent* GetIComponent(const EntityHandle& _handle, uint32_t _compIndex = 0) override;

    /// <summary>
    /// 指定したEntityが所有する全てのIComponentを取得する
    /// </summary>
    /// <param name="_handle"></param>
    /// <returns></returns>
    std::vector<IComponent*> GetIComponents(const EntityHandle& _handle) override;

    /// <summary>
    /// 指定したEntityが所有するComponent数を取得する
    /// </summary>
    /// <param name="_handle"></param>
    /// <returns></returns>
    uint32_t GetComponentCount(const EntityHandle& _handle) const;

public:

    /// <summary>
    /// コンポーネントの位置情報。
    /// ComponentHandle(uuid)から実データへ辿り着くための間接参照で、componentLocationMap_の値として使う。
    /// </summary>
    struct ComponentLocation {
        uint32_t entitySlot; // 所属するEntitySlotのDenseSlotMap上の安定ID
        uint32_t componentIndex; // EntitySlot::components内でのインデックス
    };
    /// <summary>
    /// コンポーネントのスロット内インデックス。
    /// 1つのEntityが同種コンポーネントを複数持てる設計のため、Entity単位でまとめて配列(vector)に格納する。
    /// </summary>
    struct EntitySlot {
        EntityHandle owner{}; // このスロットを所有するEntity
        std::vector<ComponentType> components; // 所有するComponent本体の配列
    };

private:
    // Entity単位でComponent群を保持する実データ本体。
    // DenseSlotMapを使うことで、削除時に発生する要素の詰め替え(swap-and-pop等)後も
    // 「安定ID」経由であれば同じ要素を指し続けられる(生配列のインデックスをそのままキーにはできないため)
    DenseSlotMap<EntitySlot> slots_;

    // entity uuid -> DenseSlotMap stable ID
    // EntityHandleから該当EntitySlotを定数時間で引くための逆引きテーブル
    std::unordered_map<uuids::uuid, uint32_t> entitySlotMap_;
    // component uuid -> (stable ID, component index)
    // ComponentHandleから実データ(EntitySlot::components内の要素)を定数時間で引くための逆引きテーブル
    std::unordered_map<uuids::uuid, ComponentLocation> componentLocationMap_;

	std::int32_t indexOnRepository_ = -1; // ComponentRepository上でのインデックス。登録時に設定される

public:
    const DenseSlotMap<EntitySlot>& GetSlots() const { return slots_; }
    DenseSlotMap<EntitySlot>& GetSlotsRef() { return slots_; }

    const std::unordered_map<uuids::uuid, uint32_t>& GetEntitySlotMap() const { return entitySlotMap_; }
    const std::unordered_map<uuids::uuid, ComponentLocation>& GetComponentLocationMap() const { return componentLocationMap_; }
    bool IsEmpty() const { return entitySlotMap_.empty(); }
};

} // namespace OriGine

// テンプレート実装のインクルード
#include "ComponentArray.inl"
