#pragma once

#include "IPlayerBehavior.h"

#include "globalVariables/SerializedField.h"

class PlayerRootBehavior
	:public IPlayerBehavior{
public:
	PlayerRootBehavior(Player* _player);
	~PlayerRootBehavior();

	void Init()override;
	void Update()override;
protected:
	void StartUp()override; //使用しない
	void Action()override;
	void EndLag()override; //使用しない
private:
	SerializedField<float> speed_;

	Vector3 lastDir_ = {0.0f,0.0f,1.0f};
};

