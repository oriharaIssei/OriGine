#include "EmitterShape.h"

#include "myRandom/MyRandom.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#ifdef _DEBUG
void EmitterSphere::Debug(){
	ImGui::DragFloat("radius",radius_,0.1f);
}
#endif // _DEBUG

Vector3 EmitterSphere::getSpawnPos(){
	MyRandom::Float randFloat = MyRandom::Float(-radius_,radius_);
	float randDist = randFloat.get();
	randFloat.setRange(-1.0f,1.0f);

	Vector3 randDire = {randFloat.get(),randFloat.get(),randFloat.get()};
	randDire = randDire.normalize();

	return randDire * randDist;
}

#ifdef _DEBUG
void EmitterAABB::Debug(){
	ImGui::DragFloat3("min",reinterpret_cast<float*>(min_.operator Vector3 * ()),0.1f);
	ImGui::DragFloat3("max",reinterpret_cast<float*>(max_.operator Vector3 * ()),0.1f);
	min_.setValue({(std::min)(min_->x,max_->x),
				  (std::min)(min_->y,max_->y),
				  (std::min)(min_->z,max_->z)});

	max_.setValue({(std::max)(min_->x,max_->x),
				  (std::max)(min_->y,max_->y),
				  (std::max)(min_->z,max_->z)});
}
#endif // _DEBUG

Vector3 EmitterAABB::getSpawnPos(){
	float randX,randY,randZ;
	MyRandom::Float randFloat(min_->x,max_->x);
	randX = randFloat.get();

	randFloat.setRange(min_->y,max_->y);
	randY = randFloat.get();

	randFloat.setRange(min_->z,max_->z);
	randZ = randFloat.get();

	return Vector3(randX,randY,randZ);
}