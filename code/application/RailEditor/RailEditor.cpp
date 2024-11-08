#include "RailEditor.h"

#include <fstream>
#include <iostream>
#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include "primitiveDrawer/PrimitiveDrawer.h"
#include "System.h"

#include <algorithm>
#include <numbers>
#include "Spline.h"

void ControlPoint::Init(const Vector3 pos,float radius){
	transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	transform_.openData_.translate = pos;
	transform_.openData_.UpdateMatrix();
	transform_.ConvertToBuffer();
	radius_ = radius;
}

void ControlPoint::Update(){
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

	transform_.openData_.worldMat = MakeMatrix::Scale(transform_.openData_.scale) * billboardMat * MakeMatrix::Translate(transform_.openData_.translate);
	transform_.ConvertToBuffer();
}

#ifdef _DEBUG
void ControlPoint::Debug(int32_t num){
	std::string label = "Translate_" + std::to_string(num);
	ImGui::DragFloat3(label.c_str(),&transform_.openData_.translate.x,0.1f);
}

#endif // _DEBUG

void ControlPoint::Draw(const IConstantBuffer<CameraTransform>& cameraTrans,const IConstantBuffer<Material>* material){
	Vector3 p[3];

	p[0] = {0,radius_,0};
	p[1] = {radius_,-radius_,0};
	p[2] = {-radius_,-radius_,0};
	PrimitiveDrawer::Triangle(p[0],p[1],p[2],transform_,cameraTrans,material);
}

void RailEditor::Init(){
	origin_.Init();

	Load();
	for(auto& point : ctlPoints_){
		controlPointPositions_.push_back(point->getWorldPosition());
	}

	for(size_t i = 0; i < segmentCount_; ++i){
		float t = 1.0f / segmentCount_ * i;
		splineSegmentPoint_.push_back(CatmullRomInterpolation(controlPointPositions_,t));
		auto& rail = railObjects_.emplace_back(Object3d::Create("./resource","rail.obj"));
		rail->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
		rail->transform_.ConvertToBuffer();
	}
}

void RailEditor::Update(){
#ifdef _DEBUG
	ImGui::Begin("RailEditor");
	if(ImGui::Button("Add controlPoint")){

		ctlPoints_.emplace_back(new ControlPoint(pCameraBuffer_));
		ctlPoints_.back()->Init({0.0f,0.0f,0.0f},1.0f);
	}
	if(ImGui::Button("Save")){
		Save();
	}

	int32_t index = 0;
	for(auto& ctlPoint : ctlPoints_){
		ctlPoint->Debug(index);
		++index;
	}
	ImGui::End();
#endif // _DEBUG

	controlPointPositions_.clear();
	for(auto& ctlPoint : ctlPoints_){
		ctlPoint->Update();
		controlPointPositions_.push_back(ctlPoint->getWorldPosition());
	}

	splineSegmentPoint_.clear();
	for(uint32_t i = 0; i < segmentCount_; i++){
		float t = 1.0f / segmentCount_ * i;
		splineSegmentPoint_.push_back(CatmullRomInterpolation(controlPointPositions_,t));
	}
	for(uint32_t i = 0; i < segmentCount_ - 1; i++){
		railObjects_[i]->transform_.openData_.translate = splineSegmentPoint_[i];
		Vector3 diff = splineSegmentPoint_[i] - splineSegmentPoint_[i + 1];
		railObjects_[i]->transform_.openData_.rotate.y = atan2(diff.x,diff.z);
		railObjects_[i]->transform_.openData_.UpdateMatrix();
		railObjects_[i]->transform_.ConvertToBuffer();
	}
}

// 可変長のポイントから Catmull-Rom 補間を行う関数
void RailEditor::Draw(const IConstantBuffer<CameraTransform>& cameraTrans){
	for(auto& rail : railObjects_){
		rail->Draw(cameraTrans);
	}
	for(auto& point : ctlPoints_){
		point->Draw(cameraTrans,System::getInstance()->getMaterialManager()->getMaterial("white"));
	}
}

std::string filename = "resource/RailPoint.csv";
void RailEditor::Load(){
	std::ifstream file(filename);

	if(!file.is_open()){
		std::cerr << "Error opening file: " << filename << std::endl;
		return;
	}

	std::string line;
	bool firstLine = true;

	while(std::getline(file,line)){
		// 最初の行（ヘッダー行）をスキップ
		if(firstLine){
			firstLine = false;
			continue;
		}

		std::stringstream ss(line);
		std::string item;
		std::vector<float> values;

		// カンマ区切りでデータを読み込む
		while(std::getline(ss,item,',')){
			values.push_back(std::stof(item));  // 文字列をfloatに変換
		}

		// Vector3として追加
		if(values.size() == 3){
			ctlPoints_.push_back(std::make_unique<ControlPoint>(pCameraBuffer_));
			ctlPoints_.back()->Init({values[0],values[1],values[2]},0.1f);
		}
	}

	file.close();
}

void RailEditor::Save(){

	std::ofstream file(filename);

	if(!file.is_open()){
		std::cerr << "Error opening file: " << filename << std::endl;
		return;
	}

	// ヘッダー行（任意）
	file << "x,y,z" << std::endl;

	// データを書き込む
	Vector3 pos;
	for(const auto& point : ctlPoints_){
		pos = point->getTranslate();
		file << pos.x << "," << pos.y << "," << pos.z << std::endl;
	}

	file.close();
}