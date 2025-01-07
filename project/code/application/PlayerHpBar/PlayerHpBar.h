#pragma once

/// stl
#include <memory>

/// engine
// component
class Sprite;
//lib
#include "globalVariables/SerializedField.h"
/// app
//object
class Player;

class PlayerHpBar {
public:
    PlayerHpBar();
    ~PlayerHpBar();
    void Init();
    void Update();
    void Draw();

private:
    Player* player_ = nullptr;
    std::unique_ptr<Sprite> hpBar_;
        
    SerializedField<Vector2> hpBarPos_;
    SerializedField<Vector2> hpBarSize_;
    float currentWidth_;
    SerializedField<Vector4> maxHpColor_;
    SerializedField<Vector4> minHpColor_;
    Vector4 currentColor_;

public:
    void setPlayer(Player* player) { player_ = player; }
};
