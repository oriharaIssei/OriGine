#pragma once

/// stl
#include <memory>
#include <string>

/// engine
// directX12
#include "directX12/raytracing/RaytracingScene.h"

/// ECS
// entity
#include "entity/EntityRepository.h"
// component
#include "component/ComponentArray.h"
#include "component/ComponentRepository.h"

/// logger
#include <logger/Logger.h>

namespace OriGine {

/// engine
class SceneManager;
// directX12
class RenderTexture;
// input
class KeyboardInput;
class MouseInput;
class GamepadInput;

/// ECS
// system
class SystemRunner;
class ISystem;

/// <summary>
/// ゲーム内の 1 つの場面 (シーン) を表すクラス.
/// エンティティ、コンポーネント、システムなどの ECS リソースを所有し、そのライフサイクルと入出力を管理する.
/// </summary>
class Scene final {
    friend class SceneFactory;

public:
    Scene(const ::std::string& _name);
    ~Scene();

    /// <summary>
    /// シーンの初期化を行う. 初期エンティティの生成などが含まれる.
    /// </summary>
    void Initialize();

    /// <summary>
    /// シーンの毎フレームの更新処理を行う. システムの実行やエンティティの削除予約処理が含まれる.
    /// </summary>
    void Update();

    /// <summary>
    /// シーンの描画処理を行う.
    /// </summary>
    void Render();

    /// <summary>
    /// シーンの終了処理を行い、ECS リソースをクリーンアップする.
    /// </summary>
    void Finalize();

    /// <summary>
    /// ECS (Entity, Component, System) 関連のストレージを初期化する.
    /// </summary>
    void InitializeECS();

    /// <summary>
    /// 描画結果を格納するメインのレンダーターゲット (SceneView) を初期化する.
    /// </summary>
    void InitializeSceneView();

    /// <summary>
    /// レイトレーシングシーンを初期化する.
    /// </summary>
    void InitializeRaytracingScene();

    /// <summary>
    /// 前フレームまでに削除予約されたエンティティを物理的に削除する.
    /// </summary>
    void ExecuteDeleteEntities();

    /// <summary>
    /// レイトレーシングシーンの更新を行う.
    /// 1フレーム中に一度だけ呼ばれる.
    /// </summary>
    void UpdateRaytracingScene();

protected:
    /// <summary>
    /// レイトレーシングで使用するメッシュを登録する.
    /// </summary>
    void DispatchMeshForRaytracing();

protected:
    /// <summary>このシーンを管理しているシーンマネージャーへのポインタ</summary>
    SceneManager* sceneManager_ = nullptr;

    /// <summary>シーンの識別名</summary>
    ::std::string name_ = "NULL";
    /// <summary>シーン固有の描画バッファ</summary>
    ::std::unique_ptr<RenderTexture> sceneView_ = nullptr;

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;

    /// <summary>エンティティのストレージ</summary>
    ::std::unique_ptr<EntityRepository> entityRepository_ = nullptr;
    /// <summary>コンポーネントのストレージ</summary>
    ::std::unique_ptr<ComponentRepository> componentRepository_ = nullptr;
    /// <summary>システムの実行管理</summary>
    ::std::unique_ptr<SystemRunner> systemRunner_ = nullptr;

    /// <summary>レイトレーシング用シーン情報の管理オブジェクト</summary>
    std::unique_ptr<RaytracingScene> raytracingScene_ = nullptr;

    std::vector<RaytracingMeshEntry> meshForRaytracing_{}; // レイトレーシング用メッシュのエントリ
    std::vector<RayTracingInstance> rayTracingInstances_{}; // レイトレーシングインスタンス

    // --- Input Devices (Engine から供給される) ---
    KeyboardInput* keyInput_ = nullptr;
    MouseInput* mouseInput_  = nullptr;
    GamepadInput* padInput_  = nullptr;

    /// <summary>現在のフレーム末尾で削除される予定のエンティティハンドルリスト</summary>
    ::std::list<EntityHandle> deleteEntities_;

    /// <summary>シーンがアクティブ (動作中) かどうか</summary>
    bool isActive_ = false;

public:
    /// <summary>シーンがアクティブ状態か取得する.</summary>
    bool IsActive() const { return isActive_; }
    /// <summary>シーンのアクティブ状態を設定する.</summary>
    void SetActive(bool _isActive) { isActive_ = _isActive; }

    const RaytracingScene* GetRaytracingScene() const { return raytracingScene_.get(); }
    RaytracingScene* GetRaytracingSceneRef() { return raytracingScene_.get(); }

    /// <summary>シーンマネージャーを取得する.</summary>
    SceneManager* GetSceneManager() const { return sceneManager_; }
    /// <summary>シーンマネージャーを設定する.</summary>
    void SetSceneManager(SceneManager* _sceneManager) { sceneManager_ = _sceneManager; }

    /// <summary>キーボード入力オブジェクトを取得する.</summary>
    KeyboardInput* GetKeyboardInput() const { return keyInput_; }
    /// <summary>マウス入力オブジェクトを取得する.</summary>
    MouseInput* GetMouseInput() const { return mouseInput_; }
    /// <summary>ゲームパッド入力オブジェクトを取得する.</summary>
    GamepadInput* GetGamepadInput() const { return padInput_; }

    /// <summary>シーンで使用する入力デバイスを一括設定する.</summary>
    void SetInputDevices(KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput) {
        keyInput_   = _keyInput;
        mouseInput_ = _mouseInput;
        padInput_   = _padInput;
    }

    /// <summary>シーン名を取得する.</summary>
    const ::std::string& GetName() const { return name_; }
    /// <summary>シーンの描画結果を保持するレンダーターゲットを取得する.</summary>
    RenderTexture* GetSceneView() const { return sceneView_.get(); }

    /// <summary>エンティティリポジトリを取得する.</summary>
    const EntityRepository* GetEntityRepository() const;
    /// <summary>エンティティリポジトリを取得する (非 const 版).</summary>
    EntityRepository* GetEntityRepositoryRef();

    /// <summary>コンポーネントリポジトリを取得する.</summary>
    const ComponentRepository* GetComponentRepository() const;
    /// <summary>コンポーネントリポジトリを取得する (非 const 版).</summary>
    ComponentRepository* GetComponentRepositoryRef();

    /// <summary>システムランナーを取得する.</summary>
    const SystemRunner* GetSystemRunner() const;
    /// <summary>システムランナーを取得する (非 const 版).</summary>
    SystemRunner* GetSystemRunnerRef();

    /// <summary>
    /// 指定したエンティティを削除予約リストに加える. 実際の削除はフレームの最後に行われる.
    /// </summary>
    /// <param name="_entityId">削除するエンティティのハンドル</param>
    void AddDeleteEntity(EntityHandle _entityId);

    // --- Entity Operation Helpers ---

    /// <summary>
    /// ハンドルからエンティティの実体を取得する.
    /// </summary>
    Entity* GetEntity(EntityHandle _handle) const;

    /// <summary>
    /// 指定したデータ型を持つユニークエンティティ (シーン内に一つだけ存在することが保証されるもの) を取得する.
    /// </summary>
    EntityHandle GetUniqueEntity(const ::std::string& _dataType) const;

    /// <summary>
    /// 新しいエンティティをシーンに生成する.
    /// </summary>
    /// <param name="_dataType">生成するエンティティの種類 (文字列識別子)</param>
    /// <param name="_isUnique">ユニークエンティティとして登録するかどうか</param>
    /// <returns>生成されたエンティティのハンドル</returns>
    EntityHandle CreateEntity(const ::std::string& _dataType, bool _isUnique = false);

    /// <summary>
    /// 既存のエンティティをユニークエンティティとしてシーンに登録する.
    /// </summary>
    bool RegisterUniqueEntity(Entity* _entity);
    /// <summary>
    /// ユニークエンティティの登録を解除する.
    /// </summary>
    bool UnregisterUniqueEntity(Entity* _entity);

    // --- Component Operation Helpers ---

    /// <summary>
    /// 指定したエンティティから特定の型を持つコンポーネントを取得する.
    /// </summary>
    /// <typeparam name="ComponentType">取得したいコンポーネントの型</typeparam>
    /// <param name="_handle">エンティティハンドル</param>
    /// <param name="_index">複数ある場合のインデックス (デフォルトは 0)</param>
    /// <returns>コンポーネントへのポインタ. 存在しない場合は nullptr</returns>
    template <IsComponent ComponentType>
    ComponentType* GetComponent(EntityHandle _handle, uint32_t _index = 0) const;

    template <IsComponent ComponentType>
    ComponentType* GetComponent(ComponentHandle _handle) const;

    /// <summary>
    /// 指定したエンティティが持つ、特定の型のコンポーネントリストをすべて取得する.
    /// </summary>
    template <IsComponent ComponentType>
    ::std::vector<ComponentType>& GetComponents(EntityHandle _handle) {
        return componentRepository_->GetComponents<ComponentType>(_handle);
    }

    /// <summary>
    /// 型名を指定してエンティティにコンポーネントを追加する.
    /// </summary>
    bool AddComponent(const ::std::string& _compTypeName, EntityHandle _handle);

    /// <summary>
    /// テンプレート引数で指定した型のコンポーネントをエンティティに追加する.
    /// </summary>
    template <IsComponent ComponentType>
    bool AddComponent(EntityHandle _handle);

    /// <summary>
    /// エンティティから指定したコンポーネントを削除する.
    /// </summary>
    bool RemoveComponent(const ::std::string& _compTypeName, EntityHandle _handle, int32_t _componentIndex = 0);

    /// <summary>
    /// 指定した型のコンポーネントを格納している内部配列を取得する.
    /// </summary>
    IComponentArray* GetComponentArray(const ::std::string& _componentTypeName) const {
        return componentRepository_->GetComponentArray(_componentTypeName);
    }
    /// <summary>
    /// 指定した型のコンポーネントを格納している内部配列を取得する (テンプレート版).
    /// </summary>
    template <IsComponent ComponentType>
    ComponentArray<ComponentType>* GetComponentArray() const {
        return componentRepository_->GetComponentArray<ComponentType>();
    }

    // --- System Operation Helpers ---

    /// <summary>
    /// 型名を指定して、シーンに登録されているシステムを取得する.
    /// </summary>
    ::std::shared_ptr<ISystem> GetSystem(const ::std::string& _systemTypeName) const;

    /// <summary>
    /// 新しいシステムをシーンに登録する.
    /// </summary>
    /// <param name="_systemTypeName">登録するシステムの型名</param>
    /// <param name="_priority">更新の優先順位 (数値が大きいほど後で実行)</param>
    /// <param name="_activity">初期状態でアクティブかどうか</param>
    bool RegisterSystem(const ::std::string& _systemTypeName, int32_t _priority = 0, bool _activity = true);

    /// <summary>
    /// 登録済みのシステムをシーンから解除する.
    /// </summary>
    bool UnregisterSystem(const ::std::string& _systemTypeName);
};

template <IsComponent ComponentType>
/// <summary>
/// コンポーネントを取得する
/// </summary>
/// <typeparam name="ComponentType">コンポーネント型</typeparam>
/// <param name="_handle">エンティティハンドル</param>
/// <param name="_index">インデックス</param>
/// <returns>コンポーネントのポインタ</returns>
inline ComponentType* Scene::GetComponent(EntityHandle _handle, uint32_t _index) const {
    if (!_handle.IsValid()) {
        LOG_ERROR("Entity is null. EntityName :{}", nameof<ComponentType>());
        return nullptr;
    }
    return componentRepository_->GetComponent<ComponentType>(_handle, _index);
}
template <IsComponent ComponentType>
inline ComponentType* Scene::GetComponent(ComponentHandle _handle) const {
    if (!_handle.IsValid()) {
        LOG_ERROR("Entity is null. EntityName :{}", nameof<ComponentType>());
        return nullptr;
    }
    return componentRepository_->GetComponent<ComponentType>(_handle);
}

template <IsComponent ComponentType>
inline bool Scene::AddComponent(EntityHandle _handle) {
    if (!_handle.IsValid()) {
        LOG_ERROR("Entity with ID '{}' not found.", uuids::to_string(_handle.uuid));
        return false;
    }
    componentRepository_->AddComponent<ComponentType>(this, _handle);
    return true;
}
} // namespace OriGine
