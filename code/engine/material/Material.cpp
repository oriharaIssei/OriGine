#include "material/Material.h"

#include "imgui/imgui.h"

#include "System.h"

const char* lightTypes[] = {
	"NONE",
	"HALF_LAMBERT",
	"LAMBERT",
	"PHONG_REFLECTION",
	"BLINN_PHONG_REFLECTION",
};

void Material::Init(){
	color_.w = 1.0f;
	uvScale_     = {1.0f,1.0f,1.0f};
	uvRotate_    = {0.0f,0.0f,0.0f};
	uvTranslate_ = {0.0f,0.0f,0.0f};
}

void Material::UpdateUvMatrix(){
	uvMat_ = MakeMatrix::Affine(uvScale_,uvRotate_,uvTranslate_);
}

IConstantBuffer<Material>* MaterialManager::Create(const std::string& materialName){
	if(materialPallet_.count(materialName) == 0){
		materialPallet_[materialName] = std::make_unique<IConstantBuffer<Material>>();
		materialPallet_[materialName]->openData_.Init();
		materialPallet_[materialName]->CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	}
	return materialPallet_[materialName].get();
}

IConstantBuffer<Material>* MaterialManager::Create(const std::string& materialName,const Material& data){
	materialPallet_[materialName] = std::make_unique<IConstantBuffer<Material>>();
	materialPallet_[materialName]->openData_ = data;
	materialPallet_[materialName]->CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	return materialPallet_[materialName].get();
}

// TODO 
// リファクタリング
void MaterialManager::DebugUpdate(){
#ifdef _DEBUG
	for(auto& material : materialPallet_){
		if(!ImGui::TreeNode(material.first.c_str())){
			continue;
		}
		ImGui::TreePop();
		ImGui::ColorEdit4(std::string(material.first + "Color").c_str(),&material.second->openData_.color_.x);

		ImGui::Combo((material.first + " Lighting Type").c_str(),
					 (int*)&material.second->openData_.enableLighting_,
					 lightTypes,
					 IM_ARRAYSIZE(lightTypes),
					 3);

		ImGui::DragFloat3((material.first + " uvScale").c_str(),&material.second->openData_.uvScale_.x,0.1f);
		ImGui::DragFloat3((material.first + " uvRotate").c_str(),&material.second->openData_.uvRotate_.x,0.1f);
		ImGui::DragFloat3((material.first + " uvTranslate").c_str(),&material.second->openData_.uvTranslate_.x,0.1f);

		ImGui::DragFloat((material.first + " Shininess").c_str(),&material.second->openData_.shininess_,0.01f,0.0f,FLT_MAX);
		ImGui::ColorEdit3((material.first + " SpecularColor").c_str(),&material.second->openData_.specularColor_.x);

		material.second->ConvertToBuffer();
	}

	ImGui::Dummy(ImVec2(0.0f,12.0f));

	ImGui::InputText("MaterialName",newMaterialName_,64);
	if(ImGui::Button("Create",{64,24})){
		Create(newMaterialName_);
	}

#endif // _DEBUG
}

void MaterialManager::Finalize(){
	materialPallet_.clear();
}

IConstantBuffer<Material>* MaterialManager::getMaterial(const std::string& name){
	auto itr = materialPallet_.find(name);
	if(itr == materialPallet_.end()){
		return nullptr;
	}
	return itr->second.get();
}

void MaterialManager::DeleteMaterial(const std::string& materialName){
	materialPallet_[materialName].reset();
	materialPallet_[materialName] = nullptr;

	std::erase_if(materialPallet_,[](const auto& pair){
		return pair.second == nullptr;
				  });
}