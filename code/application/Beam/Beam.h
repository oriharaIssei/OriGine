#pragma once

#include <memory>

#include "object3d/Object3d.h"

#include "Matrix4x4.h"
#include "Vector3.h"

class Input;
struct Transform;
class CameraTransform;
class RailCamera;

class Beam{
public:
	void Initialize();
	void Update(const RailCamera* camera,Input* input);
	void Draw(const IConstantBuffer<CameraTransform>& cameraBuff);
private:
	bool isActive_;

	std::unique_ptr<Object3d> leftObject_;
	std::unique_ptr<Object3d> rightObject_;

	std::unique_ptr<Object3d> reticleObject_;

	Vector3 leftOffset_;
	Vector3 rightOffset_;

	Vector3 reticle3dPos_;

	float lostEnergyPerSeconds_;
	float healingEnergyPerSeconds_;
	float leftEnergy_;
	float maxEnergy_;

	float kDistancePlayerTo3DReticle_;

	float radius_;

	Matrix4x4 viewPortMat_;
public:
	Vector3 getLeftOrigin()const{ leftObject_->transform_.openData_.worldMat[3]; }
	Vector3 getRightOrigin()const{ rightObject_->transform_.openData_.worldMat[3]; }
	const Vector3& getEndPos()const{ return reticle3dPos_; }

	float getRadius()const{ return radius_; }
};