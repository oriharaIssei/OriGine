#pragma once

/// stl
#include <string>

/// ECS
// system
#include "ISystem.h"
#include "SystemCategory.h"

namespace OriGine {

/// ECS
class Scene;

/// <summary>
/// System Runner
/// Systemを実際に実行するクラス.
/// System Registry から システムを借りてくる.
/// </summary>
class SystemRunner final {
public:
    SystemRunner(Scene* _scene);
    ~SystemRunner();

    /// <summary>
    /// SystemのActive状態関係なく,全てのSystemを初期化する.
    /// </summary>
    void InitializeAllCategory();
    /// <summary>
    /// SystemのActive状態関係なく,指定したカテゴリのSystemを初期化する.
    /// </summary>
    void InitializeCategory(SystemCategory _category);
    /// <summary>
    /// SystemのActive状態関係なく,指定したカテゴリのSystemを初期化する.
    /// </summary>
    /// <typeparam name="Category">対象のカテゴリ</typeparam>
    template <SystemCategory Category>
    void InitializeCategory();

    /// <summary>
    /// !!!非推奨!!!
    /// ActiveなSystemをすべて初期化する
    /// </summary>
    void InitializeActiveSystems();

    /// <summary>
    /// !!!非推奨!!!
    /// 指定したカテゴリのActiveなSystemを初期化する
    /// </summary>
    /// <param name="_category">対象のカテゴリ</param>
    void InitializeActiveCategory(SystemCategory _category);
    /// <summary>
    /// !!!非推奨!!!
    /// 指定したカテゴリのActiveなSystemを初期化する
    /// </summary>
    /// <typeparam name="Category">対象のカテゴリ</typeparam>
    template <SystemCategory Category>
    void InitializeActiveCategory();

    /// <summary>
    /// SystemのActive状態に関わらず,全てのSystemを終了する.
    /// </summary>
    void FinalizeAllCategory();
    /// <summary>
    /// SystemのActive状態に関わらず,指定したカテゴリのSystemを終了する.
    /// </summary>
    /// <param name="_category">対象のカテゴリ</param>
    void FinalizeCategory(SystemCategory _category);
    /// <summary>
    /// SystemのActive状態に関わらず,指定したカテゴリのSystemを終了する.
    /// </summary>
    /// <typeparam name="Category">対象のカテゴリ</typeparam>
    template <SystemCategory Category>
    void FinalizeCategory();

    /// <summary>
    /// !!!非推奨!!!
    /// ActiveなSystemをすべて終了する
    /// </summary>
    void FinalizeActiveSystems();
    /// <summary>
    /// !!!非推奨!!!
    /// 指定したカテゴリのActiveなSystemを終了する
    /// </summary>
    /// <param name="_category">対象のカテゴリ</param>
    void FinalizeActiveCategory(SystemCategory _category);
    /// <summary>
    /// !!!非推奨!!!
    /// 指定したカテゴリのActiveなSystemを終了する
    /// </summary>
    /// <typeparam name="Category">対象의カテゴリ</typeparam>
    template <SystemCategory Category>
    void FinalizeActiveCategory();

    /// <summary>
    /// 全てのSystemの登録を解除する
    /// </summary>
    /// <param name="_isFinalize">解除時に終了処理を行うか</param>
    void AllUnregisterSystem(bool _isFinalize = false);

    /// <summary>
    /// 指定したカテゴリのSystemを更新する
    /// </summary>
    /// <param name="_category">対象のカテゴリ</param>
    void UpdateCategory(SystemCategory _category);

    /// <summary>
    /// 指定したカテゴリのSystemを更新する
    /// </summary>
    /// <typeparam name="Category">対象のカテゴリ</typeparam>
    template <SystemCategory Category>
    void UpdateCategory();

    /// <summary>
    /// システムを登録する
    /// </summary>
    /// <param name="_systemName">登録するシステム名</param>
    /// <param name="_priority">実行優先順位</param>
    /// <param name="_isActivate">Activeにするかどうか</param>
    void RegisterSystem(const ::std::string& _systemName, int32_t _priority = 0, bool _isActivate = true);
    /// <summary>
    /// システムを登録する
    /// </summary>
    /// <typeparam name="SystemClass">登録するシステムクラス</typeparam>
    /// <param name="_priority">実行優先順位</param>
    /// <param name="_activate">Activeにするかどうか</param>
    template <IsSystem SystemClass>
    void RegisterSystem(int32_t _priority = 0, bool _activate = true);

    /// <summary>
    /// 登録を解除する
    /// </summary>
    /// <param name="_systemName">登録解除するシステム名</param>
    void UnregisterSystem(const ::std::string& _systemName);
    /// <summary>
    /// 登録を解除する
    /// </summary>
    /// <typeparam name="SystemClass">解除するシステムクラス</typeparam>
    template <IsSystem SystemClass>
    void UnregisterSystem();

    /// <summary>
    /// 指定したシステムをアクティブにする
    /// </summary>
    /// <param name="_systemName">アクティブにするシステムの名前</param>
    void ActivateSystem(const ::std::string& _systemName);

    /// <summary>
    /// 指定したシステムをアクティブにする
    /// </summary>
    /// <typeparam name="SystemClass">アクティブにするシステムクラス</typeparam>
    template <IsSystem SystemClass>
    void ActivateSystem();

    /// <summary>
    /// 指定されたシステムを非アクティブ化します。
    /// </summary>
    /// <param name="_systemName">非アクティブ化するシステムの名前。</param>
    void DeactivateSystem(const ::std::string& _systemName);
    /// <summary>
    /// 指定されたシステムを非アクティブ化します。
    /// </summary>
    /// <typeparam name="SystemClass">非アクティブ化するシステムクラス</typeparam>
    template <IsSystem SystemClass>
    void DeactivateSystem();

    /// <summary>
    /// 指定したシステムにエンティティを登録する
    /// </summary>
    /// <typeparam name="SystemClass">エンティティを追加するシステムクラスたち</typeparam>
    /// <param name="_handle">対象のエンティティハンドル</param>
    template <IsSystem... SystemClass>
    void RegisterEntity(EntityHandle _handle);
    /// <summary>
    /// 指定したシステムにエンティティを登録する
    /// </summary>
    /// <param name="_systemTypeName">エンティティを追加するシステムの名前</param>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void RegisterEntity(const ::std::string& _systemTypeName, EntityHandle _handle);

    /// <summary>
    /// 指定されたシステムからエンティティを削除します。
    /// </summary>
    /// <param name="_systemTypeName">エンティティを削除する対象のシステム名。</param>
    /// <param name="_handle">削除するEntityハンドル</param>
    void RemoveEntity(const ::std::string& _systemTypeName, EntityHandle _handle);

    /// <summary>
    /// すべてのシステムから指定されたエンティティを削除します。
    /// </summary>
    /// <param name="_handle">削除するEntityハンドル</param>
    void RemoveEntityFromAllSystems(EntityHandle _handle);

private:
    Scene* scene_ = nullptr; // 所属するシーン

    ::std::array<bool, static_cast<size_t>(SystemCategory::Count)> categoryActivity = {true, true, true, true, true, true, true, true};

    ::std::unordered_map<::std::string, ::std::shared_ptr<ISystem>> systems_;
    ::std::array<::std::vector<::std::shared_ptr<ISystem>>, size_t(SystemCategory::Count)> activeSystems_;

public:
    /// <summary>
    /// カテゴリごとのアクティビティ状態を取得する
    /// </summary>
    /// <returns>アクティビティ状態の配列参照</returns>
    const ::std::array<bool, static_cast<size_t>(SystemCategory::Count)>& GetCategoryActivity() const {
        return categoryActivity;
    }
    /// <summary>
    /// カテゴリごとのアクティビティ状態を取得する
    /// </summary>
    /// <returns>アクティビティ状態の配列への参照</returns>
    ::std::array<bool, static_cast<size_t>(SystemCategory::Count)>& GetCategoryActivityRef() {
        return categoryActivity;
    }
    /// <summary>
    /// 指定されたカテゴリがアクティブかどうかを取得する
    /// </summary>
    /// <param name="_category">対象のカテゴリ</param>
    /// <returns>アクティブならtrue</returns>
    bool GetCategoryActivity(SystemCategory _category) const {
        return categoryActivity[static_cast<size_t>(_category)];
    }
    /// <summary>
    /// カテゴリのアクティビティ状態を設定する
    /// </summary>
    /// <param name="_category">対象のカテゴリ</param>
    /// <param name="_isActive">アクティブにするならtrue</param>
    void SetCategoryActivity(SystemCategory _category, bool _isActive) {
        categoryActivity[static_cast<size_t>(_category)] = _isActive;
    }

    /// <summary>
    /// 登録されている全てのシステムを取得する
    /// </summary>
    /// <returns>システム名とシステム本体のマップ参照</returns>
    const ::std::unordered_map<::std::string, ::std::shared_ptr<ISystem>>& GetSystems() const {
        return systems_;
    }
    /// <summary>
    /// 登録されている全てのシステムを取得する
    /// </summary>
    /// <returns>システム名とシステム本体のマップへの参照</returns>
    ::std::unordered_map<::std::string, ::std::shared_ptr<ISystem>>& GetSystemsRef() {
        return systems_;
    }

    /// <summary>
    /// カテゴリ分けされたアクティブシステムを取得する
    /// </summary>
    /// <returns>カテゴリごとのシステムリストの配列参照</returns>
    const ::std::array<::std::vector<::std::shared_ptr<ISystem>>, size_t(SystemCategory::Count)>& GetActiveSystems() const {
        return activeSystems_;
    }
    /// <summary>
    /// カテゴリ分けされたアクティブシステムを取得する
    /// </summary>
    /// <returns>カテゴリごとのシステムリストの配列への参照</returns>
    ::std::array<::std::vector<::std::shared_ptr<ISystem>>, size_t(SystemCategory::Count)>& GetActiveSystemsRef() {
        return activeSystems_;
    }

    /// <summary>
    /// 指定されたカテゴリのアクティブシステムリストを取得する
    /// </summary>
    /// <param name="_category">対象のカテゴリ</param>
    /// <returns>システムのリストへの参照</returns>
    ::std::vector<::std::shared_ptr<ISystem>>& GetActiveSystemsRef(SystemCategory _category) {
        return activeSystems_[static_cast<size_t>(_category)];
    }
    /// <summary>
    /// 指定されたカテゴリのアクティブシステムリストを取得する
    /// </summary>
    /// <param name="_category">対象のカテゴリ</param>
    /// <returns>システムのリストへの参照</returns>
    const ::std::vector<::std::shared_ptr<ISystem>>& GetActiveSystems(SystemCategory _category) const {
        return activeSystems_[static_cast<size_t>(_category)];
    }
    /// <summary>
    /// システム名からシステムを取得する
    /// </summary>
    /// <param name="_systemName">システム名</param>
    /// <returns>システムの共有ポインタ (存在しない場合はnullptr)</returns>
    ::std::shared_ptr<ISystem> GetSystem(const ::std::string& _systemName) const;

    /// <summary>
    /// 型からシステムを取得する
    /// </summary>
    /// <typeparam name="SystemClass">取得するシステムの型</typeparam>
    /// <returns>システムのポインタ (存在しない場合はnullptr)</returns>
    template <IsSystem SystemClass>
    SystemClass* GetSystem() const;

    /// <summary>
    /// システム名からシステムの共有ポインタを取得する
    /// </summary>
    /// <param name="_systemName">システム名</param>
    /// <returns>システムの共有ポインタ</returns>
    ::std::shared_ptr<ISystem> GetSystemRef(const ::std::string& _systemName);

    /// <summary>
    /// 型からシステムを取得する
    /// </summary>
    /// <typeparam name="SystemClass">取得するシステムの型</typeparam>
    /// <returns>システムのポインタ</returns>
    template <IsSystem SystemClass>
    SystemClass* GetSystemRef();
};

/// <summary>
/// カテゴリを指定して初期化する
/// </summary>
/// <typeparam name="Category">対象のカテゴリ</typeparam>
template <SystemCategory Category>
inline void SystemRunner::InitializeCategory() {
    InitializeCategory(Category);
}

/// <summary>
/// カテゴリを指定してアクティブなシステムのみ初期化する (非推奨)
/// </summary>
/// <typeparam name="Category">対象のカテゴリ</typeparam>
template <SystemCategory Category>
inline void SystemRunner::InitializeActiveCategory() {
    InitializeActiveCategory(Category);
}

/// <summary>
/// カテゴリを指定して終了処理を行う
/// </summary>
/// <typeparam name="Category">対象のカテゴリ</typeparam>
template <SystemCategory Category>
inline void SystemRunner::FinalizeCategory() {
    FinalizeCategory(Category);
}

/// <summary>
/// カテゴリを指定してアクティブなシステムの終了処理を行う (非推奨)
/// </summary>
/// <typeparam name="Category">対象のカテゴリ</typeparam>
template <SystemCategory Category>
inline void SystemRunner::FinalizeActiveCategory() {
    FinalizeActiveCategory(Category);
}

/// <summary>
/// カテゴリを指定して更新処理を行う
/// </summary>
/// <typeparam name="Category">対象のカテゴリ</typeparam>
template <SystemCategory Category>
inline void SystemRunner::UpdateCategory() {
    UpdateCategory(Category);
}

/// <summary>
/// 型を指定してシステムを登録する
/// </summary>
/// <typeparam name="SystemClass">システムクラスの型</typeparam>
/// <param name="_priority">優先度</param>
/// <param name="_activate">初期状態でアクティブにするか</param>
template <IsSystem SystemClass>
inline void SystemRunner::RegisterSystem(int32_t _priority, bool _activate) {
    RegisterSystem(nameof<SystemClass>(), _priority, _activate);
}

/// <summary>
/// システムの登録を解除する
/// </summary>
/// <typeparam name="SystemClass">解除するシステムの型</typeparam>
template <IsSystem SystemClass>
inline void SystemRunner::UnregisterSystem() {
    UnregisterSystem(nameof<SystemClass>());
}
/// <summary>
/// 型を指定してシステムをアクティブにする
/// </summary>
/// <typeparam name="SystemClass">システムクラスの型</typeparam>
template <IsSystem SystemClass>
inline void SystemRunner::ActivateSystem() {
    ActivateSystem(nameof<SystemClass>());
}

/// <summary>
/// 型を指定してシステムを非アクティブにする
/// </summary>
/// <typeparam name="SystemClass">システムクラスの型</typeparam>
template <IsSystem SystemClass>
inline void SystemRunner::DeactivateSystem() {
    DeactivateSystem(nameof<SystemClass>());
}

/// <summary>
/// 指定したシステム群にエンティティを登録する
/// </summary>
/// <typeparam name="SystemClass">登録先のシステムクラス群</typeparam>
/// <param name="_handle">対象のエンティティハンドル</param>
template <IsSystem... SystemClass>
inline void SystemRunner::RegisterEntity(EntityHandle _handle) {
    // 各システムにエンティティを登録
    (GetSystem<SystemClass>()->AddEntity(_handle), ...);
}

/// <summary>
/// 型を指定してシステムを取得する
/// </summary>
/// <typeparam name="SystemClass">システムクラスの型</typeparam>
/// <returns>システムのポインタ</returns>
template <IsSystem SystemClass>
inline SystemClass* SystemRunner::GetSystem() const {
    ::std::string systemName = nameof<SystemClass>();
    auto itr                 = systems_.find(systemName);
    if (itr == systems_.end()) {
        LOG_ERROR("SystemRunner: System '{}' not found .", systemName);
        return nullptr;
    }
    return dynamic_cast<SystemClass*>(itr->second.get());
}

/// <summary>
/// 型を指定してシステムを取得する
/// </summary>
/// <typeparam name="SystemClass">システムクラスの型</typeparam>
/// <returns>システムのポインタ</returns>
template <IsSystem SystemClass>
inline SystemClass* SystemRunner::GetSystemRef() {
    ::std::string systemName = nameof<SystemClass>();
    auto itr                 = systems_.find(systemName);
    if (itr == systems_.end()) {
        LOG_ERROR("SystemRunner: System '{}' not found .", systemName);
        return nullptr;
    }
    return dynamic_cast<SystemClass*>(itr->second.get());
}

} // namespace OriGine
