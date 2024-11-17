#pragma once

#include <memory>

#include "object3d/Object3d.h"
#include "sprite/Sprite.h"

#include "Matrix4x4.h"
#include "Vector3.h"

class Input;
struct Transform;
class CameraTransform;
class Reticle;
class RailCamera;

class Beam{
public:
	void Initialize();
	void Update(const RailCamera* camera,const Reticle* reticle,Input* input);
	void Draw(const IConstantBuffer<CameraTransform>& cameraBuff);
	void DrawSprite();

	void ResetStatus(){
		leftEnergy_ = maxEnergy_;
	}
private:
	bool isActive_;

	std::unique_ptr<Object3d> leftObject_;
	std::unique_ptr<Object3d> rightObject_;

	Vector3 leftOffset_;
	Vector3 rightOffset_;

	Vector3 reticle3dPos_;

	float lostEnergyPerSeconds_;
	float healingEnergyPerSeconds_;
	float leftEnergy_;
	float maxEnergy_;

	Matrix4x4 viewPortMat_;

	std::unique_ptr<Sprite> currentEnergy_;
	std::unique_ptr<Sprite> energyBackground_;

	Vector2 spritePos_;
	Vector2 maxSpriteSize_;
	Vector2 currentSpriteSize_;
public:
	bool getIsActive()const{ return isActive_; }

	void setParent(Transform* parent){
		leftObject_->transform_.openData_.parent = parent;
		rightObject_->transform_.openData_.parent = parent;
	}
	Vector3 getLeftOrigin()   const{ return leftObject_->transform_.openData_.worldMat[3]; }
	Vector3 getRightOrigin()  const{ return rightObject_->transform_.openData_.worldMat[3]; }
	const Vector3& getEndPos()const{ return reticle3dPos_; }

};