#include "transform/Transform.h"

#include "directX12/DxFunctionHelper.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include <System.h>

void Transform::Init(){
	worldMat = MakeMatrix::Identity();
}

void Transform::UpdateMatrix(){
	worldMat = MakeMatrix::Affine(scale,rotate,translate);

	worldMat = CalculateWithParent(parent);
}

void Transform::Debug(const std::string& transformName){
	std::string labelName = transformName + " scale";
	ImGui::DragFloat3(labelName.c_str(),&scale.x,0.01f);
	labelName = transformName + " rotate";
	ImGui::DragFloat3(labelName.c_str(),&rotate.x,0.01f);
	labelName = transformName + " translation";
	ImGui::DragFloat3(labelName.c_str(),&translate.x,0.1f);
}

Matrix4x4 Transform::CalculateWithParent(const Transform* parent){
	if(parent == nullptr){
		return worldMat;
	}
	return worldMat * CalculateWithParent(parent->parent);
}