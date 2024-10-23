#pragma once

#include "model/Model.h"

#include "Matrix4x4.h"
#include "Vector3.h"

class Input;
class TransformBuffer;
class CameraBuffer;
class RailCamera;

class Beam
{
public:
	void Initialize();
	void Update(const RailCamera* camera,Input* input);
	void Draw(const CameraBuffer& cameraBuff);
private:
	bool isActive_;
	float lostEnergyPerSeconds_;
	float healingEnergyPerSeconds_;
	float leftEnergy_;
	float maxEnergy_;

	// Vector3 origin_;
	Vector3 direction_;
	float length_ = 5.0f;

	Vector3 end_;
	TransformBuffer transform_;

	Matrix4x4 viewPortMat_;
public:
	void SetOrigin(const Vector3 &origin){transform_.translate = origin;}
};