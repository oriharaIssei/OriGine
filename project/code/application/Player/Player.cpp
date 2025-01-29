#include "Player.h"

///enigne
#include "Engine.h"
//assets
#include "animation/Animation.h"
//component
#include "../Collision/Collider.h"
#include "object3d/Object3d.h"
//lib
#include "input/Input.h"
#include "deltaTime/GameDeltaTime.h"

/// application
//component
#include "PlayerBehaviors/PlayerRootBehavior.h"
//object
#include "../Enemy/IEnemy.h"

Player::Player()
    : GameObject("Player"),
      hp_("Game", "Player", "hp"),
      power_("Game", "Player", "power"),
      maxMoveLength_("Game", "Player", "maxMoveLength") {
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

    // DeltaTime
    deltaTime_ = std::make_unique<GameDeltaTime>();
    deltaTime_->Init();
}

void Player::Update() {
    if (currentHp_ <= 0.0f) {
        isAlive_ = false;
        return;
    }
    if (!isAlive_) {
        return;
    }

    deltaTime_->Update();

    float deltaTime = deltaTime_->getDeltaTime();

    Transform& transform = drawObject3d_->transform_;
    jumpForce_ -= 9.8f * deltaTime;
    transform.translate[Y] += jumpForce_ * deltaTime;
    if (transform.translate[Y] < 0.0f) {
        transform.translate[Y] = 0.0f;
        jumpForce_             = 0.0f;
        onGround_              = true;
    } else {
        onGround_ = false;
    }

    currentBehavior_->Update();

    { // Transform Update
        if (drawObject3d_->transform_.translate.lengthSq() >= maxMoveLength_ * maxMoveLength_) {
            drawObject3d_->transform_.translate = drawObject3d_->transform_.translate.normalize() * maxMoveLength_;
        }

        drawObject3d_->Update(deltaTime);
        hitCollider_->UpdateMatrix();
    }

    if (invisibleTime_ >= 0.0f) {
        invisibleTime_ -= deltaTime;
        if (invisibleTime_ < 0.0f) {
            isInvisible_ = false;

            for (auto& material :
                 drawObject3d_->getModel()->materialData_) {
                material.material = Engine::getInstance()->getMaterialManager()->getMaterial("white");
            }
        }
    }

    if (effectAnimationObject_) {
        effectAnimationObject_->Update(deltaTime);
        if (effectAnimationObject_->getAnimation()->isEnd()) {
            effectAnimationObject_.reset();
        }
    }

    // Shadow
    {
        shadowObject_->transform_.translate = (Vec3f(drawObject3d_->transform_.worldMat[3][X], -0.03f, drawObject3d_->transform_.worldMat[3][Z]));
        shadowObject_->UpdateTransform();
    }
    /*{
        auto playerModel    = drawObject3d_->getModel();
        int32_t bodyIndex   = playerModel->meshData_->meshIndexes["body"];
        auto& bodyMesh      = playerModel->meshData_->mesh_[bodyIndex];
        auto& bodyTransform = playerModel->transformBuff_[&bodyMesh].openData_;

        shadowObject_->transform_.translate = (Vec3f(bodyTransform.translate[X], -0.03f, bodyTransform.translate[Z]));
        shadowObject_->UpdateTransform();
    }*/
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

Vector3f Player::RotateUpdateByStick(float interpolation) {
    if (interpolation * interpolation <= 0.0000001f) {
        return axisZ * MakeMatrix::RotateQuaternion(getRotate());
    }
    Vector3f directionXZ;
    // 入力 に 応じた 方向を 取得，計算
    Input* input = Input::getInstance();

    if (input->isPadActive()) {
        directionXZ = {
            input->getLStickVelocity()[X],
            0.0f, // 上方向には 移動しない
            input->getLStickVelocity()[Y]};
    }

    CameraTransform* cameraTransform = cameraTransform_;
    if (cameraTransform) {
        directionXZ = TransformVector(directionXZ, MakeMatrix::RotateY(cameraTransform->rotate[Y]));
    }

    directionXZ = directionXZ.normalize();

    if (directionXZ.lengthSq() <= 0.0000001f) {
        return axisZ * MakeMatrix::RotateQuaternion(getRotate());
    }


    Quaternion currentPlayerRotate = getRotate();
    { // Player を 入力方向 へ 回転
        Quaternion inputDirectionRotate = Quaternion::RotateAxisAngle({0.0f, 1.0f, 0.0f}, atan2(directionXZ[X], directionXZ[Z]));
        inputDirectionRotate            = inputDirectionRotate.normalize();
        setRotate(Slerp(currentPlayerRotate, inputDirectionRotate, interpolation).normalize());

        if (std::isnan(getRotate().x)) {
            setRotate(inputDirectionRotate);
        }
    }
    return axisZ * MakeMatrix::RotateQuaternion(getRotate());
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
        material.material = Engine::getInstance()->getMaterialManager()->Create("Player_Invisible");
    }
}
