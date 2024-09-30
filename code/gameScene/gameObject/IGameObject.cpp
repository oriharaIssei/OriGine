#include "IGameObject.h"

#include "imgui/imgui.h"

#include "System.h"

std::list<std::pair<std::string,std::string>> IGameObject::textureList_;

void IGameObject::Init([[maybe_unused]] const std::string& directryPath,const std::string& objectName){
	name_ = objectName;
	materialManager_ = System::getInstance()->getMaterialManager();

	checkedMaterial_.push_back(0);
}

void IGameObject::Update(){
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

	if(ImGui::Combo("Materials",&checkedMaterial_[0],materialNameVector_.data(),static_cast<int>(materialNameVector_.size()))){
		material_ = materialManager_->Create(materialNameVector_[checkedMaterial_[0]]);
	}
}