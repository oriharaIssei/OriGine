#include "Beam.h"

#include <algorithm>

#include "globalVariables/GlobalVariables.h"
#include "input/Input.h"
#include "math/Easing.h"
#include "object3d/ModelManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "SLerp.h"
#include "sprite/SpriteCommon.h"
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
	leftObject_->SetModel(ModelManager::getInstance()->Create("resource/Models","Beam.obj"));
	leftObject_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	variables->addValue("Game","Beam","leftOffset",leftOffset_);
	leftObject_->transform_.openData_.translate = leftOffset_;
	// right Beam
	rightObject_ = std::make_unique<Object3d>();
	rightObject_->SetModel(ModelManager::getInstance()->Create("resource/Models","Beam.obj"));
	rightObject_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	variables->addValue("Game","Beam","rightOffset",rightOffset_);
	rightObject_->transform_.openData_.translate = rightOffset_;

	radius_ = 2.0f;

	///===========================================================
	/// Sprites
	///===========================================================
	currentEnergy_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
	energyBackground_ = std::make_unique<Sprite>(SpriteCommon::getInstance());

	currentEnergy_->Init("resource/Texture/Energy.png");
	energyBackground_->Init("resource/Texture/EnergyBackground.png");
	currentEnergy_->setAnchorPoint({0.5f,1.0f});
	energyBackground_->setAnchorPoint({0.5f,1.0f});
	currentEnergy_->setTextureSize({36.0f,256.0f});
	energyBackground_->setTextureSize({36.0f,256.0f});

	variables->addValue("Game","Beam","energySpritePos",spritePos_);
	variables->addValue("Game","Beam","energySpriteSize",maxSpriteSize_);
	variables->addValue("Game","Beam","rightOffset",rightOffset_);

	currentEnergy_->setPosition(spritePos_);
	energyBackground_->setPosition(spritePos_);

	currentEnergy_->setSize(maxSpriteSize_);
	energyBackground_->setSize(maxSpriteSize_);

	currentEnergy_->Update();
	energyBackground_->Update();

	currentSpriteSize_ = maxSpriteSize_;
}

void Beam::Update(const RailCamera* camera,const Reticle* reticle,Input* input){
	float deltaTime = System::getInstance()->getDeltaTime();

	reticle3dPos_ = reticle->getWorldPos();

	{ // Sprite Update
		float energyPercent = leftEnergy_ / maxEnergy_;
		currentSpriteSize_.y = Lerp(energyPercent,0.0f,maxSpriteSize_.y);
		currentEnergy_->setSize(currentSpriteSize_);

		currentEnergy_->Update();
	}

	isActive_ = input->isPressKey(DIK_SPACE) && leftEnergy_ > 0.0f;

	if(!isActive_){
		if(leftEnergy_ > maxEnergy_){
			return;
		}
		leftEnergy_ += healingEnergyPerSeconds_ * deltaTime;
		leftEnergy_  = (std::min)(leftEnergy_,maxEnergy_);
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
	if(!isActive_){
		return;
	}

	leftObject_->Draw(cameraBuff);
	rightObject_->Draw(cameraBuff);

}

void Beam::DrawSprite(){
	currentEnergy_->Draw();
	energyBackground_->Draw();
}
