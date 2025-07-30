#pragma once

/// stl
#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/// engine
#include "scene/Scene.h"
// ECS
#include "component/ComponentArray.h"
#include "component/IComponent.h"
#include "ECS/Entity.h"

/// lib
#include "lib/deltaTime/DeltaTime.h"
#include "logger/Logger.h"

/// <summary>
/// システムの種類(この種類によって処理順序が決定する)
/// </summary>
enum class SystemCategory {
    Initialize, // 初期化処理

    Input, // Userによる入力に対する処理
    StateTransition, // 入力等による状態遷移の処理
    Movement, // 移動や行動の処理
    Collision, // 衝突判定処理

    Effect, // エフェクト処理

    Render, // 更新処理のあとに 描画処理
    PostRender, // 描画処理のあとに 後処理

    Count
};

static const std::array<std::string, static_cast<int>(SystemCategory::Count)> SystemCategoryString = {
    "Initialize",
    "Input",
    "StateTransition",
    "Movement",
    "Collision",
    "Effect",
    "Render",
    "PostRender"};

class ISystem {
public:
    ISystem(SystemCategory _category, int32_t _priority = 0) : category_(_category), priority_(_priority) {};
    virtual ~ISystem() = default;

    virtual void Initialize() = 0;
    virtual void Update();
    virtual void Edit();
    virtual void Finalize() = 0;

    void eraseDeadEntity();

protected:
    virtual void UpdateEntity([[maybe_unused]] GameEntity* _entity) {}

    /// ==========================================
    // システム内で使用するであろう 便利関数群
    /// ==========================================

    GameEntity* getEntity(int32_t _entityID);
    GameEntity* getUniqueEntity(const std::string& _dataTypeName);

    IComponentArray* getComponentArray(const std::string& _typeName);
    template <IsComponent ComponentType>
    ComponentArray<ComponentType>* getComponentArray() {
        if (scene_ == nullptr) {
            LOG_ERROR("ComponentRepository is not set.");
            return nullptr;
        }
        return scene_->getComponentRepositoryRef()->getComponentArray<ComponentType>();
    }

    template <IsComponent ComponentType>
    ComponentType* getComponent(GameEntity* _entity, uint32_t _index = 0) {
        if (scene_ == nullptr) {
            LOG_ERROR("ComponentRepository is not set.");
            return nullptr;
        }
        return scene_->getComponentRepositoryRef()->getComponent<ComponentType>(_entity, _index);
    }
    template <IsComponent ComponentType>
    std::vector<ComponentType>* getComponents(GameEntity* _entity) {
        if (scene_ == nullptr) {
            LOG_ERROR("ComponentRepository is not set.");
            return nullptr;
        }
        return scene_->getComponentRepositoryRef()->getComponents<ComponentType>(_entity);
    }

    void addComponent(GameEntity* _entity, const std::string& _typeName, IComponent* _component, bool _doInitialize = true);
    template <IsComponent ComponentType>
    void addComponent(GameEntity* _entity, ComponentType _component, bool _doInitialize = true) {
        if (scene_ == nullptr) {
            LOG_ERROR("ComponentRepository is not set.");
            return;
        }
        getComponentArray<ComponentType>()->add(_entity, _component, _doInitialize);
    }

protected:
    std::vector<int32_t> entityIDs_;

#ifdef _DEBUG
    DeltaTime deltaTimer_;
    float runningTime_ = 0.0f; // deltaTime
#endif // _DEBUG

private:
    Scene* scene_  = nullptr;
    bool isActive_ = false;
    SystemCategory category_;

    int32_t priority_ = 0;

public: // ========================================== accessor ========================================== //
    Scene* getScene() const {
        return scene_;
    }
    void setScene(Scene* _scene) {
        scene_ = _scene;
    }
    SystemCategory getCategory() const {
        return category_;
    }
    const std::vector<int32_t>& getEntityIDs() const {
        return entityIDs_;
    }
    int32_t getEntityCount() const {
        return static_cast<int32_t>(entityIDs_.size());
    }

    bool hasEntity(const GameEntity* _entity) const {
        return std::find(entityIDs_.begin(), entityIDs_.end(), _entity->getID()) != entityIDs_.end();
    }

    void addEntity(GameEntity* _entity) {
        // 重複登録を防ぐ
        if (std::find(entityIDs_.begin(), entityIDs_.end(), _entity->getID()) != entityIDs_.end()) {
            return;
        }
        entityIDs_.push_back(_entity->getID());
    }
    void removeEntity(GameEntity* _entity) {
        entityIDs_.erase(std::remove(entityIDs_.begin(), entityIDs_.end(), _entity->getID()), entityIDs_.end());
    }
    void clearEntities() {
        entityIDs_.clear();
    }

    void setPriority(int32_t _priority) {
        priority_ = _priority;
    }
    int32_t getPriority() const {
        return priority_;
    }

#ifdef _DEBUG
    float getRunningTime() const { return runningTime_; }
#endif // _DEBUG

    bool isActive() const {
        return isActive_;
    }
    void setIsActive(bool _isActive) {
        isActive_ = _isActive;
    }
};

// Systemを継承しているかどうか
template <typename T>
concept IsSystem = std::is_base_of<ISystem, T>::value;

/// <summary>
/// System Registry
/// SystemRegistryはシステムの登録, 削除, 取得などを行う.
/// </summary>
class SystemRegistry final {
public:
    static SystemRegistry* getInstance() {
        static SystemRegistry instance;
        return &instance;
    }

    void clearAll() {
        systems_.clear();
    }

    template <IsSystem SystemClass>
    void registerSystem() {
        auto addedSystem = std::make_unique<SystemClass>();

        if (addedSystem->getCategory() == SystemCategory::Count) {
            LOG_ERROR("SystemRegistry: Invalid SystemCategory.");
            return;
        }
        std::string systemName = nameof<SystemClass>();
        if (systems_.find(systemName) != systems_.end()) {
            LOG_WARN("SystemRegistry: System already registered with name: {}", systemName);
            return;
        }

        systems_[systemName] = std::move(addedSystem);
    }

    ISystem* getSystem(const std::string& _systemName) const {
        auto itr = systems_.find(_systemName);
        if (itr == systems_.end()) {
            LOG_ERROR("SystemRegistry: System not found with name: {}", _systemName);
            return nullptr;
        }
        return itr->second.get();
    }
    template <IsSystem SystemClass>
    SystemClass* getSystem() const {
        std::string systemName = nameof<SystemClass>();
        auto itr               = systems_.find(systemName);
        if (itr == systems_.end()) {
            LOG_ERROR("SystemRegistry: System not found with name: {}", systemName);
            return nullptr;
        }
        return dynamic_cast<SystemClass*>(itr->second.get());
    }

private:
    std::unordered_map<std::string, std::unique_ptr<ISystem>> systems_;

public:
    const std::unordered_map<std::string, std::unique_ptr<ISystem>>& getSystems() const {
        return systems_;
    }
    std::unordered_map<std::string, std::unique_ptr<ISystem>>& getSystemsRef() {
        return systems_;
    }
};

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
    void InitializeCategory() {
        InitializeCategory(Category);
    }

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
    void InitializeActiveCategory() {
        InitializeActiveCategory(Category);
    }

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
    void FinalizeCategory() {
        FinalizeCategory(Category);
    }

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
    void FinalizeActiveCategory() {
        FinalizeActiveCategory(Category);
    }

    void AllUnregisterSystem(bool _isFinalize = false);

    void UpdateCategory(SystemCategory _category);
    template <SystemCategory Category>
    void UpdateCategory() {
        UpdateCategory(Category);
    }

    void registerSystem(const std::string& _systemName, int32_t _priority = 0, bool _isInitalize = true, bool _activate = true);
    template <IsSystem SystemClass>
    void registerSystem(int32_t _priority = 0, bool _isInitalize = true, bool _activate = true) {
        registerSystem(nameof<SystemClass>(), _priority, _isInitalize, _activate);
    }
    void unregisterSystem(const std::string& _systemName, bool _isFinalize = false);
    template <IsSystem SystemCategory>
    void unregisterSystem(bool _isFinalize = false) {
        unregisterSystem(nameof<SystemCategory>());
    }

    void ActivateSystem(const std::string& _systemName);
    template <IsSystem SystemCategory>
    void ActivateSystem() {
        ActivateSystem(nameof<SystemCategory>());
    }
    void ActivateSystem(SystemCategory _category, const std::string& _systemName);
    template <IsSystem SystemCategory>
    void ActivateSystem(SystemCategory _category) {
        ActivateSystem(_category, nameof<SystemCategory>());
    }
    void DeactivateSystem(SystemCategory _category, const std::string& _systemName);
    template <IsSystem SystemCategory>
    void DeactivateSystem(SystemCategory _category) {
        DeactivateSystem(nameof<SystemCategory>());
    }

    template <IsSystem... SystemClass>
    void registerEntity(GameEntity* _entity) {
        // 各システムにエンティティを登録
        (getSystem<SystemClass>()->addEntity(_entity), ...);
    }
    void registerEntity(const std::string& _systemTypeName, GameEntity* _entity) {
        // システム名からシステムを取得し、エンティティを登録
        ISystem* system = SystemRegistry::getInstance()->getSystem(_systemTypeName);
        if (system) {
            system->addEntity(_entity);
        } else {
            LOG_ERROR("SystemRunner: System '{}' not found.", _systemTypeName);
        }
    }
    void removeEntity(const std::string& _systemTypeName, GameEntity* _entity) {
        // システム名からシステムを取得し、エンティティを削除
        ISystem* system = SystemRegistry::getInstance()->getSystem(_systemTypeName);
        if (system) {
            system->removeEntity(_entity);
        } else {
            LOG_ERROR("SystemRunner: System '{}' not found.", _systemTypeName);
        }
    }

    void removeEntityFromAllSystems(GameEntity* _entity) {
        // 各システムからエンティティを削除
        for (auto& systemByCategory : systems_) {
            for (auto& [name, system] : systemByCategory) {
                if (!system->hasEntity(_entity)) {
                    continue;
                }
                system->removeEntity(_entity);
            }
        }
    }

private:
    Scene* scene_ = nullptr; // 所属するシーン

    std::array<bool, static_cast<size_t>(SystemCategory::Count)> categoryActivity = {true, true, true, true, true, true, true, true};

    std::array<std::unordered_map<std::string, ISystem*>, size_t(SystemCategory::Count)> systems_;
    std::array<std::vector<ISystem*>, size_t(SystemCategory::Count)> activeSystems_;

public:
    const std::array<bool, static_cast<size_t>(SystemCategory::Count)>& getCategoryActivity() const {
        return categoryActivity;
    }
    std::array<bool, static_cast<size_t>(SystemCategory::Count)>& getCategoryActivityRef() {
        return categoryActivity;
    }
    template <SystemCategory Category>
    bool getCategoryActivity() const {
        return categoryActivity[static_cast<size_t>(Category)];
    }
    bool getCategoryActivity(SystemCategory Category) const {
        return categoryActivity[static_cast<size_t>(Category)];
    }
    template <>
    bool getCategoryActivity<SystemCategory::Count>() const {
        LOG_ERROR("SystemRunner: getCategoryActivity() called with invalid SystemCategory::Count.");
        return false;
    }
    template <SystemCategory Category>
    void setCategoryActivity(bool _isActive) {
        categoryActivity[static_cast<size_t>(Category)] = _isActive;
    }
    void setCategoryActivity(SystemCategory _category, bool _isActive) {
        categoryActivity[static_cast<size_t>(_category)] = _isActive;
    }
    template <>
    void setCategoryActivity<SystemCategory::Count>(bool /*_isActive*/) {
        LOG_ERROR("SystemRunner: setCategoryActivity() called with invalid SystemCategory::Count.");
    }

    const std::array<std::unordered_map<std::string, ISystem*>, size_t(SystemCategory::Count)>& getSystems() const {
        return systems_;
    }
    std::array<std::unordered_map<std::string, ISystem*>, size_t(SystemCategory::Count)>& getSystemsRef() {
        return systems_;
    }

    const std::array<std::vector<ISystem*>, size_t(SystemCategory::Count)>& getActiveSystems() const {
        return activeSystems_;
    }
    std::array<std::vector<ISystem*>, size_t(SystemCategory::Count)>& getActiveSystemsRef() {
        return activeSystems_;
    }

    const std::vector<ISystem*> getActiveSystems(SystemCategory _category) const {
        return activeSystems_[static_cast<size_t>(_category)];
    }
    template <SystemCategory Category>
    const std::vector<ISystem**>& getActiveSystems() const {
        return activeSystems_[static_cast<size_t>(Category)];
    }
    template <SystemCategory Category>
    std::vector<ISystem**>& getActiveSystemsRef() {
        return activeSystems_[static_cast<size_t>(Category)];
    }
    ISystem* getSystem(const std::string& _systemName, SystemCategory _category) const {
        auto itr = systems_[static_cast<size_t>(_category)].find(_systemName);
        if (itr == systems_[static_cast<size_t>(_category)].end()) {
            LOG_ERROR("SystemRunner: System '{}' not found in category '{}'.", _systemName, SystemCategoryString[static_cast<int>(_category)]);
            return nullptr;
        }
        return itr->second;
    }
    ISystem* getSystem(const std::string& _systemName) const {
        for (const auto& systemCategory : systems_) {
            auto itr = systemCategory.find(_systemName);
            if (itr != systemCategory.end()) {
                return itr->second;
            }
        }
        LOG_ERROR("SystemRunner: System '{}' not found in any category.", _systemName);
        return nullptr;
    }
    template <IsSystem SystemClass>
    SystemClass* getSystem(SystemCategory _category) const {
        std::string systemName = nameof<SystemClass>();
        auto itr               = systems_[static_cast<size_t>(_category)].find(systemName);
        if (itr == systems_[static_cast<size_t>(_category)].end()) {
            LOG_ERROR("SystemRunner: System '{}' not found in category '{}'.", systemName, SystemCategoryString[static_cast<int>(_category)]);
            return nullptr;
        }
        return dynamic_cast<SystemClass*>(itr->second);
    }
    template <IsSystem SystemClass>
    SystemClass* getSystem() const {
        std::string systemName = nameof<SystemClass>();
        for (const auto& systemCategory : systems_) {
            auto itr = systemCategory.find(systemName);
            if (itr != systemCategory.end()) {
                return dynamic_cast<SystemClass*>(itr->second);
            }
        }
        LOG_ERROR("SystemRunner: System '{}' not found in any category.", systemName);
        return nullptr;
    }
};
