#include "EmitterShape.h"


#ifdef _DEBUG
#include "myRandom/MyRandom.h"
#include "imgui/imgui.h"
#endif // _DEBUG

#ifdef _DEBUG
void EmitterSphere::Debug(){
	ImGui::DragFloat("radius",&radius_,0.1f);
}

Vector3 EmitterSphere::getSpawnPos(){
	MyRandom::Float randFloat = MyRandom::Float(-radius_,radius_);
	float randDist = randFloat.get();
	randFloat.setRange(-1.0f,1.0f);

	Vector3 randDire = {randFloat.get(),randFloat.get(),randFloat.get()};
	randDire = randDire.Normalize();

	return randDire * randDist;
}

void EmitterAABB::Debug(){
	ImGui::DragFloat3("min",&min_.x,0.1f);
	ImGui::DragFloat3("max",&max_.x,0.1f);
	min_ = {(std::min)(min_.x,max_.x),
			(std::min)(min_.y,max_.y),
			(std::min)(min_.z,max_.z)};

	max_ = {(std::max)(min_.x,max_.x),
			(std::max)(min_.y,max_.y),
			(std::max)(min_.z,max_.z)};
}

Vector3 EmitterAABB::getSpawnPos(){
	float randX,randY,randZ;
	MyRandom::Float randFloat(min_.x,max_.x);
	randX = randFloat.get();

	randFloat.setRange(min_.y,max_.y);
	randY = randFloat.get();

	randFloat.setRange(min_.z,max_.z);
	randZ = randFloat.get();

	return Vector3(randX,randY,randZ);
}
#endif // _DEBUG