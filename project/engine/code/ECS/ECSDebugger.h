#pragma once

#ifdef _DEBUG

/// parent
#include "module/Debugger/IDebugger.h"

/// stl
#include <deque>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

/// ECS
#include "component/IComponent.h"
#include "ECSManager.h"
#include "Entity.h"
// util
#include "util/nameof.h"

class EntityDebugger
    : public IDebugger {
public:
    EntityDebugger();
    ~EntityDebugger();

    void Initialize() override;
    void Update() override;
    void Finalize() override;

private:
    ECSManager* ecsManager_  = nullptr;
    GameEntity* debugEntity_ = nullptr;
    std::map<std::string, std::deque<IComponent*>> debugEntityComponents_;
};

class SystemDebugger
    : public IDebugger {
public:
    SystemDebugger();
    ~SystemDebugger();
    void Initialize() override;
    void Update() override;
    void Finalize() override;

private:
    void SortPriorityOrderFromECSManager();
    void SortPriorityOrderSystems(int32_t _systemTypeIndex);

private:
    ECSManager* ecsManager_ = nullptr;

    std::array<std::vector<std::pair<std::string, ISystem*>>, int32_t(SystemType::Count)> workSystemList_;
};

#endif //_DEBUG
