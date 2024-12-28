#include "PlayerWeakAttackBehavior.h"

#include "Engine.h"

#include "../Player.h"
#include "PlayerRootBehavior.h"

PlayerWeakAttackBehavior::PlayerWeakAttackBehavior(Player* _player,int32_t _currentCombo)
	:IPlayerBehavior(_player),
	maxCombo_{"Game","PlayerWeakAttack","maxCombo"},
	startUpTime_{"Game","PlayerWeakAttack" + std::to_string(_currentCombo),"startUpTime"},
	actionTime_{"Game","PlayerWeakAttack" + std::to_string(_currentCombo),"actionTime"},
	endLagTime_{"Game","PlayerWeakAttack" + std::to_string(_currentCombo),"endLagTime"}{
	currentCombo_ = _currentCombo;
}

PlayerWeakAttackBehavior::~PlayerWeakAttackBehavior(){}

void PlayerWeakAttackBehavior::Init(){
	if(currentCombo_ > maxCombo_){
		player_->ChangeBehavior(new PlayerRootBehavior(player_));
		return;
	}

	nextBehavior_ = new PlayerRootBehavior(player_);

	currentUpdate_ = [this](){StartUp(); };
}

void PlayerWeakAttackBehavior::Update(){
	currentUpdate_();
}

void PlayerWeakAttackBehavior::StartUp(){
	currentTimer_ += Engine::getInstance()->getDeltaTime();

	if(currentTimer_ >= startUpTime_){
		currentTimer_ = 0.0f;
		currentUpdate_ = [this](){this->Action(); };
	}
}

void PlayerWeakAttackBehavior::Action(){
	currentTimer_ += Engine::getInstance()->getDeltaTime();

	if(input->isTriggerKey(DIK_SPACE)){
		nextBehavior_ = new PlayerWeakAttackBehavior(player_,currentCombo_ + 1);
	}

	if(currentTimer_ >= actionTime_){
		currentTimer_ = 0.0f;
		currentUpdate_ = [this](){this->EndLag(); };
	}
}

void PlayerWeakAttackBehavior::EndLag(){
	currentTimer_ += Engine::getInstance()->getDeltaTime();

	if(input->isTriggerKey(DIK_SPACE)){
		nextBehavior_ = new PlayerWeakAttackBehavior(player_,currentCombo_ + 1);
	}

	if(currentTimer_ >= endLagTime_){
		currentTimer_ = 0.0f;
		player_->ChangeBehavior(nextBehavior_);
	}
}
