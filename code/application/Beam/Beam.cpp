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
#include "../reticle/Reticle.h"

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
	/// Object
	///===========================================================
	// Left Beam
	leftObject_ = std::make_unique<Object3d>();
	leftObject_->SetModel(ModelManager::getInstance()->Create("resource","Beam.obj"));
	leftObject_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	variables->addValue("Game","Beam","leftOffset",leftOffset_);
	leftObject_->transform_.openData_.translate = leftOffset_;
	// right Beam
	rightObject_ = std::make_unique<Object3d>();
	rightObject_->SetModel(ModelManager::getInstance()->Create("resource","Beam.obj"));
	rightObject_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	variables->addValue("Game","Beam","rightOffset",rightOffset_);
	rightObject_->transform_.openData_.translate = rightOffset_;

}

void Beam::Update(const RailCamera* camera,const Reticle* reticle,Input* input){
	float deltaTime = System::getInstance()->getDeltaTime();

	reticle3dPos_ = reticle->getWorldPos();

	isActive_ = input->isPressKey(DIK_SPACE) || leftEnergy_ > 0.0f;

	if(!isActive_){
		if(leftEnergy_ > maxEnergy_){
			return;
		}
		leftEnergy_ += healingEnergyPerSeconds_ * deltaTime;
		leftEnergy_  = (std::max)(leftEnergy_,maxEnergy_);
		return;
	}

	{ // Energy Update
		leftEnergy_ -= lostEnergyPerSeconds_ * deltaTime;
	}

	{
		const Matrix4x4& cameraMat = camera->getTransform().worldMat;
		// カメラ位置とオフセットを加算してオブジェクト位置を更新
		leftObject_->transform_.openData_.translate = cameraMat * leftOffset_;
		rightObject_->transform_.openData_.translate = cameraMat * rightOffset_;
	}
	// ビームをリティクルの位置に向ける
	// Left Object rotation to reticle
	{
		Vector3 leftToReticleDir = (reticle3dPos_ - leftObject_->transform_.openData_.translate).Normalize();
		leftObject_->transform_.openData_.rotate.y = std::atan2(leftToReticleDir.x,leftToReticleDir.z);  // Y軸回転
		Vector3 velocityZ = MakeMatrix::RotateY(-leftObject_->transform_.openData_.rotate.y) * leftToReticleDir;
		leftObject_->transform_.openData_.rotate.x = std::atan2(-velocityZ.y,velocityZ.z); // X軸回転
	}

	// Right Object rotation to reticle
	{
		Vector3 rightToReticleDir = (reticle3dPos_ - rightObject_->transform_.openData_.translate).Normalize();
		rightObject_->transform_.openData_.rotate.y = std::atan2(rightToReticleDir.x,rightToReticleDir.z);  // Y軸回転
		Vector3 velocityZ = MakeMatrix::RotateY(-rightObject_->transform_.openData_.rotate.y) * rightToReticleDir;
		rightObject_->transform_.openData_.rotate.x = std::atan2(-velocityZ.y,velocityZ.z); // X軸回転
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