#include "PlayerHpBar.h"

#include "../Player/Player.h"
#include "sprite/Sprite.h"

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
    hpBar_->setAnchorPoint(Vector2(0.0f, 0.5f));
    hpBar_->setSize(hpBarSize_);
    hpBar_->setPosition(hpBarPos_);
    currentWidth_ = hpBarSize_->x;
    currentColor_ = maxHpColor_;
}

void PlayerHpBar::Update() {
    if (player_) {
        float t = player_->getHP() / player_->getMaxHp();

        t             = (std::max)(0.0f, t);
        currentWidth_ = hpBarSize_->x * t;
        currentColor_ = Lerp(minHpColor_, maxHpColor_, t);
    }
    hpBar_->setSize(Vector2(currentWidth_, hpBarSize_->y));
    hpBar_->setColor(currentColor_);
    hpBar_->setPosition(hpBarPos_);
    hpBar_->Update();
}

void PlayerHpBar::Draw() {
    hpBar_->Draw();
}
