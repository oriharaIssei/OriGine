#include "directX12/buffer/Material.h"

#include "directX12/dxFunctionHelper/DxFunctionHelper.h"
#include <System.h>

#include "imgui/imgui.h"

const char *lightTypes[] = {
	"NONE",
	"HALF_LAMBERT",
	"LAMBERT",
	"PHONG_REFLECTION",
	"BLINN_PHONG_REFLECTION",
};

void Material::Init(){
	buff_.CreateBufferResource(System::getInstance()->getDxDevice(),sizeof(Material::ConstantBuffer));

	buff_.getResource()->Map(
		0,nullptr,reinterpret_cast<void **>(&mappingData_)
	);

	mappingData_->color = {1.0f,1.0f,1.0f,1.0f};
	mappingData_->enableLighting = 0;

	uvScale_     = {1.0f,1.0f,1.0f};
	uvRotate_    = {0.0f,0.0f,0.0f};
	uvTranslate_ = {0.0f,0.0f,0.0f};

	mappingData_->uvTransform = MakeMatrix::Affine(uvScale_,uvRotate_,uvTranslate_);
}

void Material::UpdateUvMatrix()
{
	uvMat_ = MakeMatrix::Affine(uvScale_,uvRotate_,uvTranslate_);
}

void Material::Finalize(){
	buff_.Finalize();
}

void Material::ConvertToBuffer()
{
	mappingData_->uvTransform = uvMat_;
	mappingData_->color = color_;
	mappingData_->enableLighting = enableLighting_;
	mappingData_->specularColor = specularColor_;
	mappingData_->shininess = shininess_;

}

Material *MaterialManager::Create(const std::string &materialName){
	if(materialPallet_.count(materialName) == 0){
		materialPallet_[materialName] = std::make_unique<Material>();
		materialPallet_[materialName]->Init();
	}
	return materialPallet_[materialName].get();
}

Material *MaterialManager::Create(const std::string &materialName,const MaterialData &data){
	materialPallet_[materialName] = std::make_unique<Material>();
	materialPallet_[materialName]->Init();
	*materialPallet_[materialName]->mappingData_ = {
		.color          = data.color,
		.enableLighting = data.enableLighting,
		.uvTransform    = data.uvTransform
	};

	return materialPallet_[materialName].get();
}

// TODO 
// リファクタリング
void MaterialManager::DebugUpdate(){
#ifdef _DEBUG
	for(auto &material : materialPallet_){
		if(!ImGui::TreeNode(material.first.c_str())){
			continue;
		}
		ImGui::TreePop();
		ImGui::ColorEdit4(std::string(material.first + "Color").c_str(),&material.second->mappingData_->color.x);

		ImGui::Combo((material.first + " Lighting Type").c_str(),
					 (int *)&material.second->mappingData_->enableLighting,
					 lightTypes,
					 IM_ARRAYSIZE(lightTypes),
					 3);

		ImGui::DragFloat3((material.first + " uvScale").c_str(),&material.second->uvScale_.x,0.1f);
		ImGui::DragFloat3((material.first + " uvRotate").c_str(),&material.second->uvRotate_.x,0.1f);
		ImGui::DragFloat3((material.first + " uvTranslate").c_str(),&material.second->uvTranslate_.x,0.1f);

		ImGui::DragFloat((material.first + " Shininess").c_str(),&material.second->mappingData_->shininess,0.01f,0.0f,FLT_MAX);
		ImGui::ColorEdit3((material.first + " SpecularColor").c_str(),&material.second->mappingData_->specularColor.x);

		material.second->mappingData_->uvTransform = MakeMatrix::Affine(material.second->uvScale_,material.second->uvRotate_,material.second->uvTranslate_);
	}

	ImGui::Dummy(ImVec2(0.0f,12.0f));

	ImGui::InputText("MaterialName",newMaterialName_,64);
	if(ImGui::Button("Create",{64,24})){
		Create(newMaterialName_);
	}

#endif // _DEBUG
}

void MaterialManager::Finalize(){
	for(auto &material : materialPallet_){
		material.second->Finalize();
	}
}

Material* MaterialManager::getMaterial(const std::string& name)
{
	auto itr = materialPallet_.find(name);
	if(itr == materialPallet_.end())
	{
		return nullptr;
	}
	return itr->second.get();
}

void MaterialManager::DeleteMaterial(const std::string &materialName){
	materialPallet_[materialName].reset();
	materialPallet_[materialName] = nullptr;

	std::erase_if(materialPallet_,[](const auto &pair){
		return pair.second == nullptr;
	});
}