#pragma once

#include "input/Input.h"

class Player;

class IPlayerBehavior{
public:
	IPlayerBehavior(Player* _player);
	~IPlayerBehavior();

	virtual void Init() = 0;
	virtual void Update() = 0;
protected:
	virtual void StartUp() = 0;
	virtual void Action() = 0;
	virtual void EndLag() = 0;
	Player* player_;

	Input* input;
};