#pragma once

#include "IPlayerBehavior.h"

#include "globalVariables/SerializedField.h"

#include "Vector3.h"

class PlayerRootBehavior
	:public IPlayerBehavior{
public:
	PlayerRootBehavior(Player* _player);
	~PlayerRootBehavior();

	void Init()override;
	void Update()override;
protected:
	void StartUp()override; //使用しない
	void Action()override; //使用しない
	void EndLag()override; //使用しない
private:
	SerializedField<float> speed_;
};

