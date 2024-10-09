#pragma once

#include <d3d12.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <wrl.h>

#include "IConstantBuffer.h"

#include "Matrix4x4.h"
#include "stdint.h"
#include "Vector4.h"

struct MaterialData
{
	Vector4 color;
	uint32_t enableLighting;
	Matrix4x4 uvTransform;
	float shininess = 10.0f;
	Vector3 specularColor = {1.0f,1.0f,1.0f};
};

class MaterialManager;
class Material
	: public IConstantBuffer{
	friend class MaterialManager;
public:
	Material() = default;
	~Material()override{}

	void Update()override;
	void Finalize()override;

	void ConvertToBuffer()override;
	void SetForRootParameter(ID3D12GraphicsCommandList* cmdList,UINT rootParameterNum)const;
public:
	Vector3 uvScale_;
	Vector3 uvRotate_;
	Vector3 uvTranslate_;
	Matrix4x4 uvMat_;

	Vector4 color_;

	uint32_t enableLighting_;
	float	 shininess_;
	Vector3  specularColor_;
public:
	struct ConstantBuffer{
		Vector4 color;
		uint32_t enableLighting;
		float padding[3];// 下記を参照
		Matrix4x4 uvTransform;
		float shininess;
		Vector3 specularColor;
	};
private:
	void Init()override;
private:
	Material::ConstantBuffer* mappingData_ = nullptr;
};

class MaterialManager
{
public:
	Material* Create(const std::string& materialName);
	Material* Create(const std::string& materialName,const MaterialData& data);

	void DebugUpdate();

	void Finalize();
private:
	std::unordered_map<std::string,std::unique_ptr<Material>> materialPallet_;
	char newMaterialName_[64];
public:
	Material* getMaterial(const std::string& materialName) const
	{
		auto it = materialPallet_.find(materialName);
		if(it != materialPallet_.end())
		{
			return it->second.get();
		} else
		{
	  // キーが存在しない場合の処理
			return nullptr; // または適切なエラー処理を行う
		}
	}

	const std::unordered_map<std::string,std::unique_ptr<Material>>& getMaterialPallet()const { return materialPallet_; }
	Material* getMaterial(const std::string& name);

	void DeleteMaterial(const std::string& materialName);
};