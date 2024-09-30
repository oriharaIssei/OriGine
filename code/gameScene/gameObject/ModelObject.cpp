#include "ModelObject.h"

#include "imgui/imgui.h"

#include "texture/TextureManager.h"

#include "System.h"

void ModelObject::Init(const std::string& directryPath,const std::string& objectName){
	IGameObject::Init(directryPath,objectName);

	model_ = Model::Create(directryPath,objectName + ".obj");

	transform_.Init();
}

void ModelObject::Update(){
#ifdef _DEBUG
	ImGui::DragFloat3("Scale",&transform_.scale.x,0.1f);
	ImGui::DragFloat3("Rotate",&transform_.rotate.x,0.1f);
	ImGui::DragFloat3("Translate",&transform_.translate.x,0.1f);
	transform_.Update();

	if(!materialNameVector_.empty()){
		materialNameVector_.clear();
	}
	for(auto& material : materialManager_->getMaterialPallete()){
		materialNameVector_.push_back(material.first.c_str());
	}

	if(checkedMaterial_.size() < model_->getData().size()){
		checkedMaterial_.resize(model_->getData().size());
	}

	int index = 0;
	for(auto& data : model_->getData()){
		// ImGui::InputTextを使用して文字列の入力を受け取る
		if(ImGui::Combo(std::string("# " + std::to_string(index) + "Material").c_str(),&checkedMaterial_[index],materialNameVector_.data(),static_cast<int>(materialNameVector_.size()))){
			data->material_ = materialManager_->getMaterial(materialNameVector_[checkedMaterial_[index]]);
		}
		index++;
	}
#endif // _DEBUG
}

void ModelObject::Draw(const ViewProjection& viewProj){
	model_->Draw(transform_,viewProj);
}