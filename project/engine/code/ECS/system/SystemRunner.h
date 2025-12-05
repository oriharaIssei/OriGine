#pragma once

class Scene;
#include "ISystem.h"
#include "SystemCategory.h"

namespace OriGine {

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
    void InitializeActiveCategory(SystemCategory _category);
    /// <summary>
    /// !!!非推奨!!!
    /// 指定したカテゴリのActiveなSystemを初期化する
    /// </summary>
    template <SystemCategory Category>
    void InitializeActiveCategory();

    /// <summary>
    /// SystemのActive状態に関わらず,全てのSystemを終了する.
    /// </summary>
    void FinalizeAllCategory();
    /// <summary>
    /// SystemのActive状態に関わらず,指定したカテゴリのSystemを終了する.
    /// / </summary>
    void FinalizeCategory(SystemCategory _category);
    /// <summary>
    /// SystemのActive状態に関わらず,指定したカテゴリのSystemを終了する.
    /// </summary>
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
    void FinalizeActiveCategory(SystemCategory _category);
    /// <summary>
    /// !!!非推奨!!!
    /// 指定したカテゴリのActiveなSystemを終了する
    /// </summary>
    template <SystemCategory Category>
    void FinalizeActiveCategory();

    /// <summary>
    /// 全てのSystemの登録を解除する
    /// </summary>
    void AllUnregisterSystem(bool _isFinalize = false);

    /// <summary>
    /// 指定したカテゴリのSystemを更新する
    /// </summary>
    void UpdateCategory(SystemCategory _category);

    /// <summary>
    /// 指定したカテゴリのSystemを更新する
    /// </summary>
    template <SystemCategory Category>
    void UpdateCategory();

    /// <summary>
    /// システムを登録する
    /// </summary>
    /// <param name="_systemName">登録するシステム名</param>
    /// <param name="_priority">実行優先順位</param>
    /// <param name="_isActivate">Activeにするかどうか</param>
    void RegisterSystem(const std::string& _systemName, int32_t _priority = 0, bool _isActivate = true);
    /// <summary>
    /// システムを登録する
    /// </summary>
    /// <typeparam name="SystemClass">登録するシステムクラス</typeparam>
    /// <param name="_priority">実行優先順位</param>
    /// <param name="_isActivate">Activeにするかどうか</param>
    template <IsSystem SystemClass>
    void RegisterSystem(int32_t _priority = 0, bool _activate = true);

    /// <summary>
    /// 登録を解除する
    /// </summary>
    /// <param name="_systemName">登録解除するシステム名</param>
    void UnregisterSystem(const std::string& _systemName);
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
    void ActivateSystem(const std::string& _systemName);

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
    void DeactivateSystem(const std::string& _systemName);
    /// <summary>
    /// 指定されたシステムを非アクティブ化します。
    /// </summary>
    /// <typeparam name="SystemClass">非アクティブ化するシステムクラス</typeparam>
    template <IsSystem SystemClass>
    void DeactivateSystem();

    /// <summary>
    /// 指定したシステムにエンティティを登録する
    /// </summary>
    /// <typeparam name="...SystemClass">エンティティを追加するシステムクラスたち</typeparam>
    /// <param name="_entity"></param>
    template <IsSystem... SystemClass>
    void RegisterEntity(Entity* _entity);
    /// <summary>
    /// 指定したシステムにエンティティを登録する
    /// </summary>
    /// <param name="_systemTypeName">エンティティを追加するシステムの名前</param>
    /// <param name="_entity"></param>
    void RegisterEntity(const std::string& _systemTypeName, Entity* _entity);

    /// <summary>
    /// 指定されたシステムからエンティティを削除します。
    /// </summary>
    /// <param name="_systemTypeName">エンティティを削除する対象のシステム名。</param>
    /// <param name="_entity">削除するEntityオブジェクトへのポインタ。</param>
    void RemoveEntity(const std::string& _systemTypeName, Entity* _entity);

    void RemoveEntityFromAllSystems(Entity* _entity);

private:
    Scene* scene_ = nullptr; // 所属するシーン

    std::array<bool, static_cast<size_t>(SystemCategory::Count)> categoryActivity = {true, true, true, true, true, true, true, true};

    std::unordered_map<std::string, std::unique_ptr<ISystem>> systems_;
    std::array<std::vector<ISystem*>, size_t(SystemCategory::Count)> activeSystems_;

public:
    const std::array<bool, static_cast<size_t>(SystemCategory::Count)>& GetCategoryActivity() const {
        return categoryActivity;
    }
    std::array<bool, static_cast<size_t>(SystemCategory::Count)>& GetCategoryActivityRef() {
        return categoryActivity;
    }
    bool GetCategoryActivity(SystemCategory category) const {
        return categoryActivity[static_cast<size_t>(category)];
    }
    void SetCategoryActivity(SystemCategory _category, bool _isActive) {
        categoryActivity[static_cast<size_t>(_category)] = _isActive;
    }

    const std::unordered_map<std::string, std::unique_ptr<ISystem>>& GetSystems() const {
        return systems_;
    }
    std::unordered_map<std::string, std::unique_ptr<ISystem>>& GetSystemsRef() {
        return systems_;
    }

    const std::array<std::vector<ISystem*>, size_t(SystemCategory::Count)>& GetActiveSystems() const {
        return activeSystems_;
    }
    std::array<std::vector<ISystem*>, size_t(SystemCategory::Count)>& GetActiveSystemsRef() {
        return activeSystems_;
    }

    std::vector<ISystem*>& GetActiveSystemsRef(SystemCategory _category) {
        return activeSystems_[static_cast<size_t>(_category)];
    }
    const std::vector<ISystem*>& GetActiveSystems(SystemCategory category) const {
        return activeSystems_[static_cast<size_t>(category)];
    }
    ISystem* GetSystem(const std::string& _systemName) const;

    template <IsSystem SystemClass>
    SystemClass* GetSystem() const;

    ISystem* GetSystemRef(const std::string& _systemName);

    template <IsSystem SystemClass>
    SystemClass* GetSystemRef();
};

template <SystemCategory Category>
inline void SystemRunner::InitializeCategory() {
    InitializeCategory(Category);
}

template <SystemCategory Category>
inline void SystemRunner::InitializeActiveCategory() {
    InitializeActiveCategory(Category);
}

template <SystemCategory Category>
inline void SystemRunner::FinalizeCategory() {
    FinalizeCategory(Category);
}

template <SystemCategory Category>
inline void SystemRunner::FinalizeActiveCategory() {
    FinalizeActiveCategory(Category);
}

template <SystemCategory Category>
inline void SystemRunner::UpdateCategory() {
    UpdateCategory(Category);
}

template <IsSystem SystemClass>
inline void SystemRunner::RegisterSystem(int32_t _priority, bool _activate) {
    RegisterSystem(nameof<SystemClass>(), _priority, _activate);
}

template <IsSystem SystemCategory>
inline void SystemRunner::UnregisterSystem() {
    UnregisterSystem(nameof<SystemCategory>());
}

template <IsSystem SystemClass>
inline void SystemRunner::ActivateSystem() {
    ActivateSystem(nameof<SystemClass>());
}

template <IsSystem SystemClass>
inline void SystemRunner::DeactivateSystem() {
    DeactivateSystem(nameof<SystemClass>());
}

template <IsSystem... SystemClass>
inline void SystemRunner::RegisterEntity(Entity* _entity) {
    // 各システムにエンティティを登録
    (GetSystem<SystemClass>()->AddEntity(_entity), ...);
}

template <IsSystem SystemClass>
inline SystemClass* SystemRunner::GetSystem() const {
    std::string systemName = nameof<SystemClass>();
    auto itr               = systems_.find(systemName);
    if (itr == systems_.end()) {
        LOG_ERROR("SystemRunner: System '{}' not found .", systemName);
        return nullptr;
    }
    return dynamic_cast<SystemClass*>(itr->second.get());
}

template <IsSystem SystemClass>
inline SystemClass* SystemRunner::GetSystemRef() {
    std::string systemName = nameof<SystemClass>();
    auto itr               = systems_.find(systemName);
    if (itr == systems_.end()) {
        LOG_ERROR("SystemRunner: System '{}' not found .", systemName);
        return nullptr;
    }
    return dynamic_cast<SystemClass*>(itr->second.get());
}

} // namespace OriGine
