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

class MaterialManager;
class Material{
	friend class MaterialManager;
public:
	Material() = default;
	~Material(){}

	void Init();
	void UpdateUvMatrix();
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
		ConstantBuffer& operator=(const Material& material){
			color          = material.color_;
			enableLighting = material.enableLighting_;
			uvTransform    = material.uvMat_;
			shininess      = material.shininess_;
			specularColor  = material.specularColor_;
			return *this;
		}
	};
};

class MaterialManager
{
public:
	IConstantBuffer<Material>* Create(const std::string& materialName);
	IConstantBuffer<Material>* Create(const std::string& materialName,const Material& data);

	void DebugUpdate();

	void Finalize();
private:
	std::unordered_map<std::string,std::unique_ptr<IConstantBuffer<Material>>> materialPallet_;
#ifdef _DEBUG
	char newMaterialName_[64];
#endif // _DEBUG
public:
	IConstantBuffer<Material>* getMaterial(const std::string& materialName) const
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

	const std::unordered_map<std::string,std::unique_ptr<IConstantBuffer<Material>>>& getMaterialPallet()const { return materialPallet_; }
	IConstantBuffer<Material>* getMaterial(const std::string& name);

	void DeleteMaterial(const std::string& materialName);
};