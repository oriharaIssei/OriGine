#include "PlayerRootBehavior.h"

#include "../Player.h"
#include "../PlayerBehaviors/PlayerDodgeBehavior.h"
#include "../PlayerBehaviors/PlayerWeakAttackBehavior.h"

#include "Engine.h"
#include "input/Input.h"

PlayerRootBehavior::PlayerRootBehavior(Player* _player)
	:IPlayerBehavior(_player),
	speed_{"Game","PlayerRoot","speed"}{}

PlayerRootBehavior::~PlayerRootBehavior(){}

void PlayerRootBehavior::Init(){
	currentUpdate_ = [this](){this->Action(); };
}

void PlayerRootBehavior::Update(){
	currentUpdate_();

	if(input->isTriggerKey(DIK_LSHIFT)){
		player_->ChangeBehavior(new PlayerDodgeBehavior(player_,lastDir_));
	} else if(input->isTriggerKey(DIK_SPACE)){
		player_->ChangeBehavior(new PlayerWeakAttackBehavior(player_,0));
	}
}

void PlayerRootBehavior::StartUp(){}

void PlayerRootBehavior::Action(){
	Vector3 directionXZ;
	{ // 入力 に 応じた 方向を 取得，計算
		directionXZ = {
			static_cast<float>(input->isPressKey(DIK_D) - input->isPressKey(DIK_A)),
			0.0f, // 上方向には 移動しない
			static_cast<float>(input->isPressKey(DIK_W) - input->isPressKey(DIK_S))
		};
	}
	if(directionXZ.lengthSq() == 0.0f){
		return; // skip
	}
	directionXZ = directionXZ.normalize();
	lastDir_ = directionXZ;

	const Quaternion& currentPlayerRotate = player_->getRotate();
	{ // Player を 入力方向 へ 回転
		Quaternion inputDirectionRotate = Quaternion::RotateAxisAngle({0.0f,1.0f,0.0f},acosf(directionXZ.dot()));
		player_->setRotate(inputDirectionRotate);
	}

	{// 方向と速度を 使って 次の座標を計算
		// 速度を 秒単位に
		float speedPerSecond = speed_ * Engine::getInstance()->getDeltaTime();

		// 現在の 座標
		const Vector3& playerPos = player_->getTranslate();
		player_->setTranslate(playerPos + directionXZ * speedPerSecond);
	}
}

void PlayerRootBehavior::EndLag(){}
