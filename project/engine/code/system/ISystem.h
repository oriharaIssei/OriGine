#pragma once

/// stl
#include <array>
#include <vector>

/// engine
// ECS
#include "ECS/Entity.h"
// lib
#include "lib/deltaTime/DeltaTime.h"

/// <summary>
/// システムの種類(この種類によって処理順序が決定する)
/// </summary>
enum class SystemType {
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

static const std::array<std::string, static_cast<int>(SystemType::Count)> SystemTypeString = {
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
    ISystem(SystemType _systemType, int32_t _priority = 0) : systemType_(_systemType), priority_(_priority) {};
    virtual ~ISystem() = default;

    virtual void Initialize() = 0;
    virtual void Update();
    virtual void Finalize() = 0;

    void eraseDeadEntity() {
        std::erase_if(entities_, [](GameEntity* _entity) {
            return !_entity || !_entity->isAlive();
        });
    }

protected:
    virtual void UpdateEntity([[maybe_unused]] GameEntity* _entity) {}

protected:
    std::vector<GameEntity*> entities_;

#ifdef _DEBUG
    DeltaTime deltaTimer_;
    float deltaTime_ = 0.0f; // deltaTime
#endif // _DEBUG

private:
    bool isActive_ = true;
    SystemType systemType_;

    int32_t priority_ = 0;

public: // ========================================== accessor ========================================== //
    SystemType getSystemType() const {
        return systemType_;
    }
    const std::vector<GameEntity*>& getEntities() const {
        return entities_;
    }
    int32_t getEntityCount() const {
        return static_cast<int32_t>(entities_.size());
    }

    bool hasEntity(const GameEntity* _entity) const {
        return std::find(entities_.begin(), entities_.end(), _entity) != entities_.end();
    }

    void addEntity(GameEntity* _entity) {
        // 重複登録を防ぐ
        if (std::find(entities_.begin(), entities_.end(), _entity) != entities_.end()) {
            return;
        }
        entities_.push_back(_entity);
    }
    void removeEntity(GameEntity* _entity) {
        entities_.erase(std::remove(entities_.begin(), entities_.end(), _entity), entities_.end());
    }
    void clearEntities() {
        entities_.clear();
    }

    void setPriority(int32_t _priority) {
        priority_ = _priority;
    }
    int32_t getPriority() const {
        return priority_;
    }

#ifdef _DEBUG
    float getDeltaTime() const { return deltaTime_; }
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
