#include "IPlayerBehavior.h"

#include "../Player.h"

IPlayerBehavior::IPlayerBehavior(Player* _player)
	:player_(_player){
	input = Input::getInstance();
}

IPlayerBehavior::~IPlayerBehavior(){}
