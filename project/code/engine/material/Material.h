#pragma once

///stl
#include <memory>
#include <string>
#include <unordered_map>
///microsoft
#include <d3d12.h>
#include <wrl.h>

///engine
#include "assets/IAsset.h"
#include "directX12/IConstantBuffer.h"
#include "globalVariables/SerializedField.h"
#include "module/IModule.h"
#include "module/editor/IEditor.h"
///math
#include "Matrix4x4.h"
#include "Vector4.h"
#include "stdint.h"

class MaterialManager;
struct Material
    : IAsset{
    friend class MaterialManager;

public:
    Material(){}
    Material(const std::string& _materialName)
        : uvScale_("Materials",_materialName,"uvScale"),
        uvRotate_("Materials",_materialName,"uvRotate"),
        uvTranslate_("Materials",_materialName,"uvTranslate"),
        color_("Materials",_materialName,"color"),
        enableLighting_("Materials",_materialName,"enableLighting"),
        shininess_("Materials",_materialName,"shininess"),
        specularColor_("Materials",_materialName,"specularColor"){}
    ~Material(){}

    void UpdateUvMatrix();

public:
    SerializedField<Vector3> uvScale_;
    SerializedField<Vector3> uvRotate_;
    SerializedField<Vector3> uvTranslate_;
    Matrix4x4 uvMat_ = MakeMatrix::Identity();

    SerializedField<Vector4> color_;

    SerializedField<int32_t> enableLighting_;
    SerializedField<float> shininess_;
    SerializedField<Vector3> specularColor_;

public:
    struct ConstantBuffer{
        Vector4 color;
        uint32_t enableLighting;
        float padding[3]; // 下記を参照
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
    : public IModule{
    friend class MaterialEditor;
public:
    IConstantBuffer<Material>* Create(const std::string& materialName);
    IConstantBuffer<Material>* Create(const std::string& materialName,const Material& data);

    void Finalize();

private:
    std::unordered_map<std::string,std::unique_ptr<IConstantBuffer<Material>>> materialPallet_;

public:
    IConstantBuffer<Material>* getMaterial(const std::string& materialName) const{
        auto it = materialPallet_.find(materialName);
        if(it != materialPallet_.end()){
            return it->second.get();
        } else{
            // キーが存在しない場合の処理
            return nullptr; // または適切なエラー処理を行う
        }
    }

    const std::unordered_map<std::string,std::unique_ptr<IConstantBuffer<Material>>>& getMaterialPallet() const{ return materialPallet_; }
    IConstantBuffer<Material>* getMaterial(const std::string& name);

    void DeleteMaterial(const std::string& materialName);
};

class MaterialEditor
    : public IEditor{
public:
    MaterialEditor(MaterialManager* materialManager)
        : IEditor(),materialManager_(materialManager){}
    ~MaterialEditor(){}

    void Update() override;
private:
    void MenuUpdate();
private:
    MaterialManager* materialManager_;

    bool openCreateNewPopup_ = false;
};
