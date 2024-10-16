#include "RailEditor.h"

#include "imgui/imgui.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "System.h"

#include <algorithm>
#include <numbers>

void ControlPoint::Init(const Vector3 pos,float radius){
	transform_.Init();
	transform_.translate = pos;
	radius_ = radius;
}

void ControlPoint::Update(int32_t num){
#ifdef _DEBUG
	std::string label = "Translate_" + std::to_string(num);
	ImGui::DragFloat3(label.c_str(),&transform_.translate.x,0.1f);
#endif // _DEBUG

	///===========================================================================
	///  Billboard化
	///===========================================================================
	Matrix4x4 cameraRotateMat = pCameraBuffer_.viewMat;
	// 平方移動 を無視
	for(size_t i = 0; i < 3; i++){
		cameraRotateMat[3][i] = 0.0f;
	}
	cameraRotateMat[3][3] = 1.0f;

	// Y 軸に pi/2 回転させる
	Matrix4x4 billboardMat = cameraRotateMat.Inverse();

	transform_.worldMat = MakeMatrix::Scale(transform_.scale) * billboardMat * MakeMatrix::Translate(transform_.translate);
	transform_.ConvertToBuffer();
}

void ControlPoint::Draw(const Material* material){
	Vector3 p[3];

	p[0] = {0,radius_,0};
	p[1] = {radius_,-radius_,0};
	p[2] = {-radius_,-radius_,0};
	PrimitiveDrawer::Triangle(p[0],p[1],p[2],transform_,pCameraBuffer_,material);
}

void RailEditor::Init(){
	origin_.Init();
	for(size_t i = 0; i < 4; i++){
		ctlPoints_.emplace_back(std::make_unique<ControlPoint>(pCameraBuffer_));
		ctlPoints_.back()->Init({0.0f,0.0f,0.0f},1.0f);
	}
}

void RailEditor::Update(){
	int32_t index = 0;
	ImGui::Begin("RailEditor");
	if(ImGui::Button("Add controlPoint")){
	{
		ctlPoints_.emplace_back(new ControlPoint(pCameraBuffer_));
		ctlPoints_.back()->Init({0.0f,0.0f,0.0f},1.0f);
	}
	}
	ImGui::End();
	for(auto& ctlPoint : ctlPoints_){
		ctlPoint->Update(index);
		++index;
	}
}

Vector3 CatmullRomInterpolation(const Vector3& p0,const Vector3& p1,const Vector3& p2,const Vector3& p3,float t){
	constexpr float half = 0.5f;

	float t2 = t * t;
	float t3 = t2 * t;

	Vector3 e3 = -p0 + 3.0f * p1 - 3.0f * p2 + p3;
	Vector3 e2 = 2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3;
	Vector3 e1 = -p0 + p2;
	Vector3 e0 = 2.0f * p1;

	return half * (e3 * t3 + e2 * t2 + e1 * t + e0);
}
Vector3 CatmullRomInterpolation(const std::vector<Vector3>& points,float t){
	assert(points.size() >= 4 && "制御点は 4点以上必要です");
	size_t division = points.size() - 1;
	float areaWidth = 1.0f / division;

	float t_2 = std::fmod(t,areaWidth) * division;
	t_2 = std::clamp(t_2,0.0f,1.0f);

	size_t index = static_cast<size_t>(t / areaWidth);
	index = (std::min)(index,division - 1);

	size_t index0 = index - 1;
	size_t index1 = index;
	size_t index2 = index + 1;
	size_t index3 = index + 2;

	if(index == 0){
		index0 = index1;
	}
	if(index3 >= points.size()){
		index3 = index2;
	}

	Vector3 p0 = points[index0];
	Vector3 p1 = points[index1];
	Vector3 p2 = points[index2];
	Vector3 p3 = points[index3];

	return CatmullRomInterpolation(p0,p1,p2,p3,t_2);
}

void RailEditor::Draw(){
	for(auto& point : ctlPoints_){
		point->Draw(System::getInstance()->getMaterialManager()->getMaterial("white"));
		controlPointPositions_.push_back(point->getWorldPosition());
	}

	splineSegmentPoint_.clear();
	for(size_t i = 0; i < segmentCount_; ++i){
		float t = 1.0f / segmentCount_ * i;
		splineSegmentPoint_.push_back(CatmullRomInterpolation(controlPointPositions_,t));
	}

	/*
	Vector3 segmentStart = splineSegmentPoint_[0];
	Vector3 segmentEnd = splineSegmentPoint_[1];
	for(size_t i = 1; i < splineSegmentPoint_.size() - 1; i++)
	{
		PrimitiveDrawer::Line(segmentStart,segmentEnd,origin_,pCameraBuffer_,System::getInstance()->getMaterialManager()->getMaterial("white"));
		segmentStart = segmentEnd;
		segmentEnd = splineSegmentPoint_[i+1];
	}
	*/
}