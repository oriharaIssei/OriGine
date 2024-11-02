#include "Beam.h"

#include <algorithm>

#include "globalVariables/GlobalVariables.h"
#include "input/Input.h"
#include "System.h"
#include "transform/CameraTransform.h"
#include "transform/Transform.h"
#include "primitiveDrawer/PrimitiveDrawer.h"

#include "railCamera/RailCamera.h"

void Beam::Initialize(){
	GlobalVariables* variables = GlobalVariables::getInstance();
	variables->addValue("Game","Beam","lostEnergyPerSeconds_",lostEnergyPerSeconds_);
	variables->addValue("Game","Beam","healingEnergyPerSeconds_",healingEnergyPerSeconds_);
	variables->addValue("Game","Beam","maxEnergy_",maxEnergy_);
	variables->addValue("Game","Beam","length_",length_);

	leftEnergy_ = maxEnergy_;

	transform_.Init();
	WinApp* window = System::getInstance()->getWinApp();
	viewPortMat_ = MakeMatrix::Orthographic(0,0,(float)window->getWidth(),(float)window->getHeight(),0.0f,100.0f);
}

void Beam::Update(const RailCamera* camera,Input* input){
	{ // Direction Update
		Vector3 mousePos = {input->getCurrentMousePos(),0.0f};
		const CameraTransform& cameraBuff = camera->getCameraBuffer();
		Matrix4x4 inverseVpv = (cameraBuff.viewMat * cameraBuff.projectionMat * viewPortMat_).Inverse();
		Vector3 mouseOnNearClip = inverseVpv * mousePos;
		mousePos.z = 1.0f;
		Vector3 mouseOnFarClip = inverseVpv * mousePos;

		direction_ = (mouseOnFarClip - mouseOnNearClip).Normalize();
	}

	{ // Energy Update
		const float& deltaTime = System::getInstance()->getDeltaTime();

		isActive_ = input->isPressKey(DIK_SPACE) || leftEnergy_ > 0.0f;
		if(isActive_){
			leftEnergy_ -= lostEnergyPerSeconds_ * deltaTime;
		} else{
			leftEnergy_ += healingEnergyPerSeconds_ * deltaTime;
		}
		leftEnergy_ = std::clamp(leftEnergy_,0.0f,maxEnergy_);
	}

	end_ = MakeMatrix::RotateXYZ(direction_) * Vector3(0.0f,0.0f,length_);

	transform_.UpdateMatrix();
}

void Beam::Draw(const CameraTransform& cameraBuff){
	if(!isActive_){
		return;
	}
	//PrimitiveDrawer::Line({0.0f,0.0f,0.0f},end_,transform_,cameraBuff,System::getInstance()->getMaterialManager()->getMaterial("white"));
}
