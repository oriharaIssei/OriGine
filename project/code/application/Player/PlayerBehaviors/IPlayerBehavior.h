#pragma once

#include <functional>

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

	// 上記3つの Update の 内 どれか一つを currentUpdate にセットして Update 内で使用する
	std::function<void()> currentUpdate_;

	Player* player_;

	Input* input;
};