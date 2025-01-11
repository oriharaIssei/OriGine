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

    SerializedField<Vec2f> hpBarPos_;
    SerializedField<Vec2f> hpBarSize_;
    float currentWidth_;
    SerializedField<Vec4f> maxHpColor_;
    SerializedField<Vec4f> minHpColor_;
    Vec4f currentColor_;

public:
    void setPlayer(Player* player) { player_ = player; }
};
