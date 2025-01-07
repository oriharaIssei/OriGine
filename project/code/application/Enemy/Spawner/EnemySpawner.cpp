#include "EnemySpawner.h"

///engine
#include "Engine.h"
//assetes
#include "animation/Animation.h"
///applicatoin
#include "../Manager/EnemyManager.h"
#include "application/Collision/Collider.h"

EnemySpawner::EnemySpawner(IEnemy* _enemy, int index)
    : GameObject("EnemySpawner"),
      cloneOrigine_(_enemy),
      spawnCoolTime_("Game", "EnemySpawner", "spawnCoolTime"),
      maxHp_("Game", "EnemySpawner", "maxHp"),
      position_("Game", "EnemySpawner" + std::to_string(index), "position") {}

EnemySpawner::~EnemySpawner() {
}

void EnemySpawner::Init() {
    isAlive_ = true;

    leftCoolTime_ = spawnCoolTime_;
    hp_           = maxHp_;

    // DrawObject
    drawObject3d_ = std::make_unique<AnimationObject3d>();
    drawObject3d_->Init(AnimationSetting("EnemySpawner"));
    drawObject3d_->transform_.translate = position_;
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
    shadowObject_->transform_.scale = Vector3(3.5f, 3.5f, 3.5f);

    // Collider
    hitCollider_ = std::make_unique<Collider>("EnemySpawner");
    hitCollider_->Init();
    hitCollider_->setHostObject(this);
    hitCollider_->setParent(&drawObject3d_->transform_);
}

void EnemySpawner::Update() {
    if (hp_ <= 0.0f) {
        isAlive_ = false;
        return;
    }
    if (!isAlive_) {
        return;
    }

    if (isInvisible_) {
        invisibleTime_ -= Engine::getInstance()->getDeltaTime();
        if (invisibleTime_ < 0.0f) {
            isInvisible_ = false;
        }
    }

    leftCoolTime_ -= Engine::getInstance()->getDeltaTime();
    if (leftCoolTime_ <= 0.0f && !enemyManager_->isMaxEnemy()) {
        leftCoolTime_ = spawnCoolTime_;
        // Spawn
        enemyManager_->addEnemy(std::move(Spawn()));
    }

    setTranslate(position_);
    drawObject3d_->Update(Engine::getInstance()->getDeltaTime());

    // Shadow
    {
        shadowObject_->transform_.translate = (Vector3(drawObject3d_->transform_.translate.x, -0.03f, drawObject3d_->transform_.translate.z));
        shadowObject_->UpdateTransform();
    }
}

void EnemySpawner::Draw() {
    drawObject3d_->Draw();
    // Shadow
    Object3d::setBlendMode(BlendMode::Sub);
    shadowObject_->Draw();
    Object3d::setBlendMode(BlendMode::Alpha);
}

std::unique_ptr<IEnemy> EnemySpawner::Spawn() {
    std::unique_ptr<IEnemy> clone = std::move(cloneOrigine_->Clone());
    clone->Init();
    clone->setTranslate(position_);
    clone->setPlayer(enemyManager_->getPlayer());
    return clone;
}
