#include "RailEditor.h"

#include "primitiveDrawer/PrimitiveDrawer.h"

#include <numbers>

void ControlPoint::Init(const Vector3 pos,float radius){
	transform_.Init();
	transform_.translate = pos;
	radius_ = radius;
}

void ControlPoint::Update(){
	///===========================================================================
	///  Billboard化
	///===========================================================================
	Matrix4x4 cameraRotateMat = pViewProjection_.viewMat;
	// 平方移動 を無視
	for(size_t i = 0; i < 3; i++){
		cameraRotateMat[3][i] = 0.0f;
	}
	cameraRotateMat[3][3] = 1.0f;

	// Y 軸に pi/2 回転させる
	Matrix4x4 billboardMat = MakeMatrix::RotateY(std::numbers::pi_v<float>) * cameraRotateMat;

	transform_.worldMat = MakeMatrix::Scale(transform_.scale) * billboardMat * MakeMatrix::Translate(transform_.translate);
	transform_.ConvertToBuffer();
}

void ControlPoint::Draw(const Material* material){
	Vector3 p[3];

	p[0] = {0,radius_,0};
	p[1] = {radius_,-radius_,0};
	p[2] = {-radius_,-radius_,0};
	PrimitiveDrawer::Triangle(p[0],p[1],p[2],transform_,pViewProjection_,material);
}