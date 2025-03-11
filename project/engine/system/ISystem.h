#pragma once

/// stl
#include <array>
#include <vector>

/// engine
// ECS
#include "ECS/Entity.h"

/// <summary>
/// システムの種類(この種類によって処理順序が決定する)
/// </summary>
enum class SystemType {
    Input, // Userによる入力に対する処理
    StateTransition, // 入力等による状態遷移の処理
    Movement, // 移動や行動の処理
    Physics, // Movementの結果に対する物理演算処理
    Collision, // 衝突判定処理

    Render, // 更新処理のあとに 描画処理
    PostRender, // 描画処理のあとに 後処理

    Count
};

static const std::array<std::string, static_cast<int>(SystemType::Count)> SystemTypeString = {
    "Input",
    "StateTransition",
    "Movement",
    "Physics",
    "Collision",
    "Render",
    "PostRender"};

class ISystem {
public:
    ISystem(SystemType _systemType) : systemType_(_systemType) {};
    virtual ~ISystem() = default;

    virtual void Init() = 0;
    virtual void Update();
    virtual void Finalize() = 0;

protected:
    virtual void UpdateEntity(GameEntity* _entity) = 0;

protected:
    std::vector<GameEntity*> entities_;

private:
    SystemType systemType_;

    int32_t priority_ = 0;

public: // ========================================== accessor ========================================== //
    SystemType getSystemType() const {
        return systemType_;
    }
    const std::vector<GameEntity*>& getEntities() const {
        return entities_;
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

};

// Systemを継承しているかどうか
template <typename T>
concept IsSystem = std::is_base_of<ISystem, T>::value;
