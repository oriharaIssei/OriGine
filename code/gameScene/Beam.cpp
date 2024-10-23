#include "Beam.h"

#include <algorithm>

#include "DeltaTime/DeltaTime.h"
#include "globalVariables/GlobalVariables.h"
#include "input/Input.h"
#include "System.h"

#include "directX12/buffer/CameraBuffer.h"
#include "directX12/buffer/TransformBuffer.h"
#include "primitiveDrawer/PrimitiveDrawer.h"

void Beam::Initialize()
{
	GlobalVariables* variables = GlobalVariables::getInstance();
	variables->addValue("Game","Beam","lostEnergyPerSeconds_",lostEnergyPerSeconds_);
	variables->addValue("Game","Beam","healingEnergyPerSeconds_",healingEnergyPerSeconds_);
	variables->addValue("Game","Beam","maxEnergy_",maxEnergy_);
	variables->addValue("Game","Beam","length_",length_);

	leftEnergy_ = maxEnergy_;

	transform_.Init();
}

void Beam::Update(Input* input)
{
	isActive_ = input->isPressKey(DIK_SPACE) || leftEnergy_ > 0.0f;

	Vector2 mouseVelo = input->getMouseVelocity();

	direction_.y *= -1.0f;
	direction_ += {mouseVelo * 0.01f,0.0f};
	direction_ = direction_.Normalize();

	end_ = -(direction_.Normalize() * length_);

	const float& deltaTime = DeltaTime::getInstance()->getDeltaTime();

	if(isActive_)
	{
		leftEnergy_ -= lostEnergyPerSeconds_ * deltaTime;
	} else
	{
		leftEnergy_ += healingEnergyPerSeconds_ * deltaTime;
	}
	leftEnergy_ = std::clamp(leftEnergy_,0.0f,maxEnergy_);

	transform_.UpdateMatrix();
}

void Beam::Draw(const CameraBuffer& cameraBuff)
{
	if(!isActive_)
	{
		return;
	}
	PrimitiveDrawer::Line({0.0f,0.0f,0.0f},end_,transform_,cameraBuff,System::getInstance()->getMaterialManager()->getMaterial("white"));
}
