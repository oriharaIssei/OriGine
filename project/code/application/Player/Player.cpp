#include "Player.h"

///enigne
#include "Engine.h"
//assets
#include "animation/Animation.h"
//component
#include "../Collision/Collider.h"
#include "object3d/Object3d.h"

/// application
//component
#include "PlayerBehaviors/PlayerRootBehavior.h"
//object
#include "../Enemy/IEnemy.h"

Player::Player()
    : GameObject("Player"),
      hp_("Game", "Player", "hp"),
      power_("Game", "Player", "power"),
      maxMoveLenght_("Game", "Player", "maxMoveLenght") {
    currentHp_ = hp_;
}

Player::~Player() {}

void Player::Init() {
    isAlive_ = true;
    // DrawObject
    drawObject3d_ = std::make_unique<AnimationObject3d>();
    drawObject3d_->Init(AnimationSetting("PlayerIdle"));

    // Shadow
    shadowObject_ = std::make_unique<Object3d>();
    shadowObject_->Init("resource/Models", "ShadowPlane.obj");
    {
        auto model = shadowObject_->getModel();
        while (true) {
            if (model->meshData_->currentState_ == LoadState::Loaded) {
                break;
            }
        }
        for (auto& material : model->materialData_) {
            material.material = Engine::getInstance()->getMaterialManager()->Create("Shadow");
        }
    }
    shadowObject_->transform_.scale = Vector3(2.5f, 2.5f, 2.5f);

    // Behavior
    currentBehavior_ = std::make_unique<PlayerRootBehavior>(this);
    currentBehavior_->Init();

    // Collider
    hitCollider_ = std::make_unique<Collider>("Player");
    hitCollider_->Init();
    hitCollider_->setHostObject(this);
    hitCollider_->setParent(&drawObject3d_->transform_);

    attackCollider_ = std::make_unique<AttackCollider>("NULL");
    attackCollider_->Init();
    attackCollider_->setIsAlive(false);
}

void Player::Update() {
    if (hp_ <= 0.0f) {
        isAlive_ = false;
        return;
    }

    Transform& transform = drawObject3d_->transform_;
    jampForce_ -= 9.8f * Engine::getInstance()->getDeltaTime();
    transform.translate.y += jampForce_ * Engine::getInstance()->getDeltaTime();
    if (transform.translate.y < 0.0f) {
        transform.translate.y = 0.0f;
        jampForce_            = 0.0f;
        onGround_             = true;
    } else {
        onGround_ = false;
    }

    currentBehavior_->Update();

    { // Transform Update
        if (drawObject3d_->transform_.translate.lengthSq() >= maxMoveLenght_ * maxMoveLenght_) {
            drawObject3d_->transform_.translate = drawObject3d_->transform_.translate.normalize() * maxMoveLenght_;
        }

        drawObject3d_->Update(Engine::getInstance()->getDeltaTime());
        hitCollider_->UpdateMatrix();
    }

    if (invisibleTime_ >= 0.0f) {
        invisibleTime_ -= Engine::getInstance()->getDeltaTime();
        if (invisibleTime_ < 0.0f) {
            isInvisible_ = false;

            for (auto& material :
                 drawObject3d_->getModel()->materialData_) {
                material.material = Engine::getInstance()->getMaterialManager()->getMaterial("white");
            }
        }
    }

    if (effectAnimationObject_) {
        effectAnimationObject_->Update(Engine::getInstance()->getDeltaTime());
        if (effectAnimationObject_->getAnimation()->isEnd()) {
            effectAnimationObject_.reset();
        }
    }

    // Shadow
    {
        shadowObject_->transform_.translate = (Vector3(drawObject3d_->transform_.translate.x, -0.03f, drawObject3d_->transform_.translate.z));
        shadowObject_->UpdateTransform();
    }
}

void Player::Draw() {
    drawObject3d_->Draw();

    Object3d::setBlendMode(BlendMode::Sub);
    shadowObject_->Draw();
    Object3d::setBlendMode(BlendMode::Alpha);
    if (effectAnimationObject_) {
        effectAnimationObject_->Draw();
    }
}

void Player::ChangeBehavior(IPlayerBehavior* next) {
    currentBehavior_.reset(next);
    currentBehavior_->Init();
}

void Player::ChangeBehavior(std::unique_ptr<IPlayerBehavior>& next) {
    currentBehavior_ = std::move(next);
    currentBehavior_->Init();
}

void Player::setInvisibleTime(float time) {
    isInvisible_   = true;
    invisibleTime_ = time;
    for (auto& material :
         drawObject3d_->getModel()->materialData_) {
        material.material = Engine::getInstance()->getMaterialManager()->Create("Player_Inbisible");
    }
}
