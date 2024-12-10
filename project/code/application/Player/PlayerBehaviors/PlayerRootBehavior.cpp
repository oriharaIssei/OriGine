#include "PlayerRootBehavior.h"

#include "../Player.h"

#include "Engine.h"
#include "input/Input.h"

PlayerRootBehavior::PlayerRootBehavior(Player* _player)
	:IPlayerBehavior(_player),
	speed_{"Game","Player","speed"}{}

PlayerRootBehavior::~PlayerRootBehavior(){}

void PlayerRootBehavior::Init(){}

void PlayerRootBehavior::Update(){
	Vector2 directionXZ;
	{ // 入力 に 応じた 方向を 取得，計算
		directionXZ = {
			static_cast<float>(input->isPressKey(DIK_D) - input->isPressKey(DIK_A)),
			static_cast<float>(input->isPressKey(DIK_W) - input->isPressKey(DIK_S))
		};
		if(directionXZ.lengthSq() != 0.0f){
			directionXZ = directionXZ.normalize();
		}
	}

	// 速度を 秒単位に
	float speedPerSecond = speed_ * Engine::getInstance()->getDeltaTime();

	// 現在の 座標
	const Vector3& playerPos = player_->getTranslate();

	// 方向と速度を 使って 次の座標を計算
	player_->setTranslate(playerPos + Vector3(directionXZ.x * speed_,0.0f,directionXZ.y * speed_));
}

void PlayerRootBehavior::StartUp(){}

void PlayerRootBehavior::Action(){}

void PlayerRootBehavior::EndLag(){}
