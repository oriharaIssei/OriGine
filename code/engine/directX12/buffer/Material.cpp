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
	DxFH::CreateBufferResource(System::getInstance()->getDxDevice(),constBuff_,sizeof(ConstBufferMaterial));

	constBuff_->Map(
		0,nullptr,reinterpret_cast<void **>(&mappingData_)
	);

	mappingData_->color = {1.0f,1.0f,1.0f,1.0f};
	mappingData_->enableLighting = 0;

	uvScale_ = {1.0f,1.0f,1.0f};
	uvRotate_ = {0.0f,0.0f,0.0f};
	uvTranslate_ = {0.0f,0.0f,0.0f};

	mappingData_->uvTransform = MakeMatrix::Affine(uvScale_,uvRotate_,uvTranslate_);
}

void Material::Finalize(){
	constBuff_.Reset();
}

void Material::SetForRootParameter(ID3D12GraphicsCommandList *cmdList,UINT rootParameterNum) const{
	cmdList->SetGraphicsRootConstantBufferView(rootParameterNum,constBuff_->GetGPUVirtualAddress());
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
		.color = data.color,
		.enableLighting = data.enableLighting,
		.uvTransform = data.uvTransform
	};

	return materialPallet_[materialName].get();
}

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

void MaterialManager::Edit(const std::string &materialName,const MaterialData &data){
	*materialPallet_[materialName]->mappingData_ = {
		.color = data.color,
		.enableLighting = data.enableLighting,
		.uvTransform = data.uvTransform
	};
}

void MaterialManager::EditColor(const std::string &materialName,const Vector4 &color){
	materialPallet_[materialName]->mappingData_->color = color;
}

void MaterialManager::EditUvTransform(const std::string &materialName,const Vector3 &scale,const Vector3& rotate,const Vector3& translate){
	materialPallet_[materialName]->mappingData_->uvTransform = MakeMatrix::Affine(scale,rotate,translate);
}

void MaterialManager::EditEnableLighting(const std::string &materialName,bool enableLighting){
	materialPallet_[materialName]->mappingData_->enableLighting = enableLighting;
}

void MaterialManager::DeleteMaterial(const std::string &materialName){
	materialPallet_[materialName].reset();
	materialPallet_[materialName] = nullptr;

	std::erase_if(materialPallet_,[](const auto &pair){
		return pair.second == nullptr;
	});
}