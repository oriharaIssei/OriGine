#include "ModelObject.h"

#include "imgui/imgui.h"

#include "texture/TextureManager.h"

#include "System.h"

void ModelObject::Init(const std::string& directoryPath,const std::string& objectName){
	IGameObject::Init(directoryPath,objectName);

	model_ = Model::Create(directoryPath,objectName + ".obj");

	transform_.Init();
}

void ModelObject::Update(){
#ifdef _DEBUG
	ImGui::DragFloat3("Scale",&transform_.scale.x,0.1f);
	ImGui::DragFloat3("Rotate",&transform_.rotate.x,0.1f);
	ImGui::DragFloat3("Translate",&transform_.translate.x,0.1f);
	transform_.UpdateMatrix();

	if(!materialNameVector_.empty()){
		materialNameVector_.clear();
	}
	for(auto& material : materialManager_->getMaterialPallet()){
		materialNameVector_.push_back(material.first.c_str());
	}

	if(checkedMaterial_.size() < model_->getData().size()){
		checkedMaterial_.resize(model_->getData().size());
	}

	int index = 0;
	for(auto& data : model_->getData()){
		// ImGui::InputTextを使用して文字列の入力を受け取る
		if(ImGui::Combo(std::string("# " + std::to_string(index) + "Material").c_str(),&checkedMaterial_[index],materialNameVector_.data(),static_cast<int>(materialNameVector_.size()))){
			data->materialData.material = materialManager_->getMaterial(materialNameVector_[checkedMaterial_[index]]);
		}
		index++;
	}
#endif // _DEBUG
}

void ModelObject::Draw(const CameraBuffer& viewProj){
	model_->Draw(transform_,viewProj);
}