#include "Beam.h"

#include <algorithm>

#include "globalVariables/GlobalVariables.h"
#include "input/Input.h"
#include "object3d/ModelManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "SLerp.h"
#include "System.h"
#include "transform/CameraTransform.h"

#include "../railCamera/RailCamera.h"

void Beam::Initialize(){
	///===========================================================
	/// GlobalVariables
	///===========================================================
	GlobalVariables* variables = GlobalVariables::getInstance();

	///===========================================================
	/// Particle
	///===========================================================
	variables->addValue("Game","Beam","kDistancePlayerTo3DReticle_",kDistancePlayerTo3DReticle_);

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
	// right Beam
	rightObject_ = std::make_unique<Object3d>();
	rightObject_->SetModel(ModelManager::getInstance()->Create("resource","Beam.obj"));
	rightObject_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	variables->addValue("Game","Beam","rightOffset",rightOffset_);

	// reticle 
	reticleObject_ = std::make_unique<Object3d>();
	reticleObject_->SetModel(ModelManager::getInstance()->Create("resource","teapot.obj"));
	reticleObject_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	///===========================================================
	/// viewPortMat
	///===========================================================
	WinApp* window = System::getInstance()->getWinApp();
	viewPortMat_ = MakeMatrix::ViewPort(0,0,(float)window->getWidth(),(float)window->getHeight(),0.0f,1.0f);
}

void Beam::Update(const RailCamera* camera,Input* input){
	isActive_ = input->isPressKey(DIK_SPACE) || leftEnergy_ > 0.0f;

	{ // Reticle
		const CameraTransform& cameraT = camera->getCameraBuffer();
		Matrix4x4 inverseVpv = cameraT.viewMat * cameraT.projectionMat * viewPortMat_;
		inverseVpv = inverseVpv.Inverse();

		const Vector2& mousePos = input->getCurrentMousePos();
		Vector3 mouseOnNearClip = inverseVpv * Vector3(mousePos,0.0f);
		Vector3 mouseOnFarClip = inverseVpv * Vector3(mousePos,1.0f);

		Vector3 dir = (mouseOnFarClip - mouseOnNearClip).Normalize();
		reticle3dPos_ = mouseOnNearClip + (dir * kDistancePlayerTo3DReticle_);

		leftObject_->transform_.openData_.translate = mouseOnNearClip + leftOffset_;
		rightObject_->transform_.openData_.translate = mouseOnNearClip + rightOffset_;
	}

	{ // Rotate Update
		// Left Object rotation to reticle
		Vector3 leftToReticleDir = (reticle3dPos_ - leftObject_->transform_.openData_.translate).Normalize();
		leftObject_->transform_.openData_.rotate.y = std::atan2(leftToReticleDir.x,leftToReticleDir.z);  // Y軸回転
		leftObject_->transform_.openData_.rotate.x = std::atan2(-leftToReticleDir.y,leftToReticleDir.z); // X軸回転

		// Right Object rotation to reticle
		Vector3 rightToReticleDir = (reticle3dPos_ - rightObject_->transform_.openData_.translate).Normalize();
		rightObject_->transform_.openData_.rotate.y = std::atan2(rightToReticleDir.x,rightToReticleDir.z);  // Y軸回転
		rightObject_->transform_.openData_.rotate.x = std::atan2(-rightToReticleDir.y,rightToReticleDir.z); // X軸回転
	}

	{ // Objects Update
		leftObject_->transform_.openData_.UpdateMatrix();
		leftObject_->transform_.ConvertToBuffer();

		rightObject_->transform_.openData_.UpdateMatrix();
		rightObject_->transform_.ConvertToBuffer();

		reticleObject_->transform_.openData_.translate = reticle3dPos_;
		reticleObject_->transform_.openData_.UpdateMatrix();
		reticleObject_->transform_.ConvertToBuffer();

	}
}

void Beam::Draw(const IConstantBuffer<CameraTransform>& cameraBuff){
	leftObject_->Draw(cameraBuff);
	rightObject_->Draw(cameraBuff);

	reticleObject_->Draw(cameraBuff);
	//PrimitiveDrawer::Line({0.0f,0.0f,0.0f},end_,transform_,cameraBuff,System::getInstance()->getMaterialManager()->getMaterial("white"));
}
