#include "transform/Transform.h"

#include "directX12/DxFunctionHelper.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include <Engine.h>

void Transform::Init(){
	worldMat = MakeMatrix::Identity();
}

void Transform::UpdateMatrix(){
	worldMat = MakeMatrix::Affine(scale,rotate,translate);

	if(parent){
		worldMat = parent->worldMat * worldMat;
	}
}

void Transform::Debug(const std::string& transformName){
#ifdef _DEBUG
	std::string labelName = transformName + " scale";
	ImGui::DragFloat3(labelName.c_str(),&scale[X],0.01f);
	labelName = transformName + " rotate";
	ImGui::DragFloat4(labelName.c_str(),&rotate.x,0.01f);
	labelName = transformName + " translation";
	ImGui::DragFloat3(labelName.c_str(),&translate[X],0.1f);
#endif // _DEBUG
}
