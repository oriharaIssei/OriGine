#include "Reticle.h"

#include <algorithm>

#include "../railCamera/RailCamera.h"
#include "globalVariables/GlobalVariables.h"
#include "input/Input.h"
#include "System.h"
#include "winApp/WinApp.h"

void Reticle::Init(){
	GlobalVariables* variables = GlobalVariables::getInstance();

	variables->addValue("Game","Reticle","speed",speed_);
	variables->addValue("Game","Reticle","minPos",minPos_);
	variables->addValue("Game","Reticle","maxPos",maxPos_);
	variables->addValue("Game","Reticle","distance3DReticle_",kDistance3DReticle_);

	///===========================================================
	/// viewPortMat
	///===========================================================
	WinApp* window = System::getInstance()->getWinApp();
	viewPortMat_ = MakeMatrix::ViewPort(0,0,(float)window->getWidth(),(float)window->getHeight(),0.0f,1.0f);

	///===========================================================
	/// Sprite
	///===========================================================
	sprite_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
	sprite_->Init("resource/Texture/Reticle.png");

#ifndef _DEBUG
	Vector2 size_;
#endif // _DEBUG
	variables->addValue("Game","Reticle","size",size_);
	sprite_->setPosition(Vector2((float)window->getWidth() * 0.5f,(float)window->getHeight() * 0.5f));
	sprite_->setTextureSize({36.0f,36.0f});
	sprite_->setSize(size_);
	sprite_->setAnchorPoint({0.5f,0.5f});
}

void Reticle::Update(const RailCamera* camera,Input* input){
	const CameraTransform& cameraT = camera->getCameraBuffer();
	{ // Reticle 3d Update
		Vector2 direction = {
			static_cast<float>(input->isPressKey(DIK_D) - input->isPressKey(DIK_A)),
			static_cast<float>(input->isPressKey(DIK_W) - input->isPressKey(DIK_S))
		};

		direction = Normalize(direction);

		reticleLocal3dPos_ += {direction* (speed_* System::getInstance()->getDeltaTime()),0.0f};
		reticleLocal3dPos_.z = kDistance3DReticle_;
		reticleLocal3dPos_.x = std::clamp(reticleLocal3dPos_.x,minPos_.x,maxPos_.x);
		reticleLocal3dPos_.y = std::clamp(reticleLocal3dPos_.y,minPos_.y,maxPos_.y);

		reticleWorld3dPos_ = cameraT.viewMat.Inverse() * reticleLocal3dPos_;
	}

	{ // 3d => screen 
		vpvMat_ = cameraT.viewMat * cameraT.projectionMat * viewPortMat_;

		reticleScreenPos_ = vpvMat_ * reticleWorld3dPos_;

		sprite_->setPosition({reticleScreenPos_.x,reticleScreenPos_.y});
	}
	sprite_->Update();
}

void Reticle::DrawSprite(){
	sprite_->Draw();
}