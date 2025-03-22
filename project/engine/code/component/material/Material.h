#pragma once

/// stl
#include <memory>
#include <string>
#include <unordered_map>
/// microsoft
#include <d3d12.h>
#include <wrl.h>

/// engine
#include "assets/IAsset.h"
#include "component/IComponent.h"
#include "directX12/IConstantBuffer.h"
#include "globalVariables/SerializedField.h"
#include "module/editor/IEditor.h"
#include "module/IModule.h"

/// math
#include "Matrix4x4.h"
#include "stdint.h"
#include "Vector4.h"

class MaterialManager;
struct Material
    : IAsset {
    friend class MaterialManager;

public:
    Material()
        : uvScale_(SerializedField<Vec3f>::CreateNull()),
          uvRotate_(SerializedField<Vec3f>::CreateNull()),
          uvTranslate_(SerializedField<Vec3f>::CreateNull()),
          color_(SerializedField<Vec4f>::CreateNull()),
          enableLighting_(SerializedField<int32_t>::CreateNull()),
          shininess_(SerializedField<float>::CreateNull()),
          specularColor_(SerializedField<Vec3f>::CreateNull()) {}
    Material(const std::string& _materialName)
        : uvScale_("Materials", _materialName, "uvScale"),
          uvRotate_("Materials", _materialName, "uvRotate"),
          uvTranslate_("Materials", _materialName, "uvTranslate"),
          color_("Materials", _materialName, "color"),
          enableLighting_("Materials", _materialName, "enableLighting"),
          shininess_("Materials", _materialName, "shininess"),
          specularColor_("Materials", _materialName, "specularColor") {}
    ~Material() {}

    void UpdateUvMatrix();

public:
    // TODO : SerializedFieldを使用するべきか 要検討
    SerializedField<Vec3f> uvScale_;
    SerializedField<Vec3f> uvRotate_;
    SerializedField<Vec3f> uvTranslate_;
    Matrix4x4 uvMat_ = MakeMatrix::Identity();

    SerializedField<Vec4f> color_;

    SerializedField<int32_t> enableLighting_;
    SerializedField<float> shininess_;
    SerializedField<Vec3f> specularColor_;

public:
    struct ConstantBuffer {
        Vec4f color;
        uint32_t enableLighting;
        float padding[3]; // 下記を参照
        Matrix4x4 uvTransform;
        float shininess;
        Vec3f specularColor;
        ConstantBuffer& operator=(const Material& material) {
            color          = material.color_;
            enableLighting = material.enableLighting_;
            uvTransform    = material.uvMat_;
            shininess      = material.shininess_;
            specularColor  = material.specularColor_;
            return *this;
        }
    };
};

// template <>
// inline bool EditComponent<Material>(Material* _editComponent) {
//     bool isChange = false;
//     // --------------------------- uvScale --------------------------- //
//     isChange |= MyGui::Drag<3, float>("uvScale", _editComponent->uvScale_, 0.01f);
//     // --------------------------- uvRotate --------------------------- //
//     isChange |= MyGui::Drag<3, float>("uvRotate", _editComponent->uvRotate_, 0.01f);
//     // --------------------------- uvTranslate --------------------------- //
//     isChange |= MyGui::Drag<3, float>("uvTranslate", _editComponent->uvTranslate_, 0.01f);
//     // --------------------------- color --------------------------- //
//     isChange |= MyGui::Drag<4, float>("color", _editComponent->color_, 0.01f);
//     // --------------------------- enableLighting --------------------------- //
//     isChange |= MyGui::Drag<int32_t>("enableLighting", _editComponent->enableLighting_, 1);
//     // --------------------------- shininess --------------------------- //
//     isChange |= MyGui::Drag<float>("shininess", _editComponent->shininess_, 0.01f);
//     // --------------------------- specularColor --------------------------- //
//     isChange |= MyGui::Drag<3, float>("specularColor", _editComponent->specularColor_, 0.01f);
//     return isChange;
// }

class MaterialManager
    : public IModule {
    friend class MaterialEditor;

public:
    IConstantBuffer<Material>* Create(const std::string& materialName);
    IConstantBuffer<Material>* Create(const std::string& materialName, const Material& data);

    void Initialize() {}
    void Finalize();

private:
    std::unordered_map<std::string, std::unique_ptr<IConstantBuffer<Material>>> materialPallet_;

public:
    IConstantBuffer<Material>* getMaterial(const std::string& materialName) const {
        auto it = materialPallet_.find(materialName);
        if (it != materialPallet_.end()) {
            return it->second.get();
        } else {
            // キーが存在しない場合の処理
            return nullptr; // または適切なエラー処理を行う
        }
    }

    const std::unordered_map<std::string, std::unique_ptr<IConstantBuffer<Material>>>& getMaterialPallet() const { return materialPallet_; }
    IConstantBuffer<Material>* getMaterial(const std::string& name);

    void DeleteMaterial(const std::string& materialName);
};

class MaterialEditor
    : public IEditor {
public:
    MaterialEditor(MaterialManager* materialManager)
        : IEditor(), materialManager_(materialManager) {}
    ~MaterialEditor() {}

    void Initialize() override{}
    void Update() override;
    void Finalize()override{}
private:
    void MenuUpdate();

private:
    MaterialManager* materialManager_;

    bool openCreateNewPopup_ = false;
};
