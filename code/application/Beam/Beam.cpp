#include "Beam.h"

#include <algorithm>

#include "globalVariables/GlobalVariables.h"
#include "input/Input.h"
#include "object3d/ModelManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "SLerp.h"
#include "sprite/SpriteCommon.h"
#include "System.h"
#include "transform/CameraTransform.h"

#include "../railCamera/RailCamera.h"

void Beam::Initialize(){
	///===========================================================
	/// GlobalVariables
	///===========================================================
	GlobalVariables* variables = GlobalVariables::getInstance();

	///===========================================================
	/// Energy
	///===========================================================
	variables->addValue("Game","Beam","lostEnergyPerSeconds_",lostEnergyPerSeconds_);
	variables->addValue("Game","Beam","healingEnergyPerSeconds_",healingEnergyPerSeconds_);
	variables->addValue("Game","Beam","maxEnergy_",maxEnergy_);
	leftEnergy_ = maxEnergy_;

	///===========================================================
	/// Sprite
	///===========================================================


	///===========================================================
	/// Object
	///===========================================================
	// Left Beam
	leftObject_ = std::make_unique<Object3d>();
	leftObject_->SetModel(ModelManager::getInstance()->Create("resource","Beam.obj"));
	leftObject_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	variables->addValue("Game","Beam","leftOffset",leftOffset_);
	// right Beam
	rightObject_ = std::make_unique<Object3d>();
	rightObject_->SetModel(ModelManager::getInstance()->Create("resource","Beam.obj"));
	rightObject_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	variables->addValue("Game","Beam","rightOffset",rightOffset_);

	//// reticle 
	//reticleObject_ = std::make_unique<Object3d>();
	//reticleObject_->SetModel(ModelManager::getInstance()->Create("resource","teapot.obj"));
	//reticleObject_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());

}

void Beam::Update(const Reticle* reticle,Input* input){
	isActive_ = input->isPressKey(DIK_SPACE) || leftEnergy_ > 0.0f;

	if(!isActive_){
		return;
	}

	{ // Energy Update
	
	}

	{ // Rotate Update

	}

	{ // Objects Update
		leftObject_->transform_.openData_.UpdateMatrix();
		leftObject_->transform_.ConvertToBuffer();

		rightObject_->transform_.openData_.UpdateMatrix();
		rightObject_->transform_.ConvertToBuffer();
	}
}

void Beam::Draw(const IConstantBuffer<CameraTransform>& cameraBuff){
	leftObject_->Draw(cameraBuff);
	rightObject_->Draw(cameraBuff);

	//PrimitiveDrawer::Line({0.0f,0.0f,0.0f},end_,transform_,cameraBuff,System::getInstance()->getMaterialManager()->getMaterial("white"));
}