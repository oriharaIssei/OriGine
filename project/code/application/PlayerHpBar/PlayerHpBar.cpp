#include "PlayerHpBar.h"

///stl
//algorithm
#include <algorithm>

///engine
//component
#include "sprite/Sprite.h"
///application
//object
#include "../Player/Player.h"

PlayerHpBar::PlayerHpBar()
    : hpBarPos_{"Game", "HpBarUI", "hpBarPos"},
      hpBarSize_{"Game", "HpBarUI", "hpBarSize"},
      maxHpColor_{"Game", "HpBarUI", "maxHpColor"},
      minHpColor_{"Game", "HpBarUI", "minHpColor"} {
}

PlayerHpBar::~PlayerHpBar() {
}

void PlayerHpBar::Init() {
    hpBar_ = std::make_unique<Sprite>();
    hpBar_->Init("resource/Texture/white1x1.png");
    hpBar_->setAnchorPoint(Vec2f(0.0f, 0.5f));
    hpBar_->setSize(hpBarSize_);
    hpBar_->setTranslate(hpBarPos_);
    currentWidth_ = hpBarSize_->v[X];
    currentColor_ = maxHpColor_;
}

void PlayerHpBar::Update() {
    if (player_) {
        float t = player_->getHP() / player_->getMaxHp();
        t       = std::clamp(t, 0.0f, 1.0f); // Ensure t is within [0, 1]

        currentWidth_ = hpBarSize_->v[X] * t;
        currentColor_ = Vector<4, float>::Lerp(minHpColor_, maxHpColor_, t);
    }
    hpBar_->setSize(Vec2f(currentWidth_, hpBarSize_->v[Y]));
    hpBar_->setColor(currentColor_);
    hpBar_->setTranslate(hpBarPos_);
    hpBar_->Update();
}

void PlayerHpBar::Draw() {
    hpBar_->Draw();
}
