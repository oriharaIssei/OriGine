#pragma once

#include <d3d12.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <wrl.h>

#include "Matrix4x4.h"
#include "stdint.h"
#include "Vector4.h"

struct ConstBufferMaterial{
	Vector4 color;
	uint32_t enableLighting;
	float padding[3];// 下記を参照
	Matrix4x4 uvTransform;
	float shininess;
	Vector3 specularColor;
	/*
		< パディング >
	機会に都合のいいような
	c++とhlslのメモリ配置の違いによる誤差のようなもの。

	c++ :
	color			: [][][][]
	enableLighting	: []
	uvTransform		: [][][]float1
					  [][][]float2
					  [][][]float3
	しかし、hlslでは
	hlsl :
	color			: [][][][]
	enableLighting	: []<><><>
	uvTransform		: [][][]<>float1
					  [][][]<>float2
					  [][][]<>float3
	(<>は実際には使われないメモリ)
	となっているらしい。
	この誤差を埋めるためにc++側で隙間のメモリを上手く埋める。
	*/
};

struct MaterialData{
	Vector4 color;
	uint32_t enableLighting;
	Matrix4x4 uvTransform;
	float shininess = 10.0f;
	Vector3 specularColor = {1.0f,1.0f,1.0f};
};

class MaterialManager;
class Material{
	friend class MaterialManager;
public:
	void Finalize();

	void SetForRootParameter(ID3D12GraphicsCommandList *cmdList,UINT rootParameterNum)const;
public:
	Vector3 uvScale_;
	Vector3 uvRotate_;
	Vector3 uvTranslate_;
private:
	void Init();
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
	ConstBufferMaterial *mappingData_ = nullptr;
};

class MaterialManager{
public:
	Material *Create(const std::string &materialName);
	Material *Create(const std::string &materialName,const MaterialData &data);

	void DebugUpdate();

	void Finalize();
private:
	std::unordered_map<std::string,std::unique_ptr<Material>> materialPallet_;
	char newMaterialName_[64];
public:
	Material *getMaterial(const std::string &materialName) const{
		auto it = materialPallet_.find(materialName);
		if(it != materialPallet_.end()){
			return it->second.get();
		} else{
			// キーが存在しない場合の処理
			return nullptr; // または適切なエラー処理を行う
		}
	}

	const std::unordered_map<std::string,std::unique_ptr<Material>> &getMaterialPallet()const{ return materialPallet_; }

	void Edit(const std::string &materialName,const MaterialData &data);
	void EditColor(const std::string &materialName,const Vector4 &color);
	void EditUvTransform(const std::string& materialName,const Vector3& scale,const Vector3& rotate,const Vector3& translate);
	void EditEnableLighting(const std::string &materialName,bool enableLighting);

	void DeleteMaterial(const std::string &materialName);
};