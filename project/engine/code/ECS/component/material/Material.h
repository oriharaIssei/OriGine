#pragma once

/// stl
#include <optional>

/// engine
// directX12 object
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/DxDescriptor.h"

/// ecs
#include "component/IComponent.h"

/// math
#include "Matrix4x4.h"
#include "stdint.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

/// <summary>
/// UVTransform
/// </summary>
struct UVTransform {
    Vec2f scale_     = Vec2f(1.f, 1.f);
    float rotate_    = 0.f;
    Vec2f translate_ = Vec2f(0.f, 0.f);

    struct ConstantBuffer {
        Matrix4x4 uvTransform;

        ConstantBuffer& operator=(const UVTransform& _transform);
    };
};

/// <summary>
/// ColorとUVTransformの組み合わせ
/// </summary>
struct ColorAndUvTransform {
    ColorAndUvTransform() = default;
    ColorAndUvTransform(const Vec4f& _color, const UVTransform& _uvTransform) : color_(_color), uvTransform_(_uvTransform) {}
    Vec4f color_ = {1.f, 1.f, 1.f, 1.f};
    UVTransform uvTransform_;

    struct ConstantBuffer {
        Vec4f color;
        Matrix4x4 uvTransform;
        ConstantBuffer& operator=(const ColorAndUvTransform& _data);
    };
};

/// <summary>
/// Materialコンポーネント
/// </summary>
struct Material
    : public IComponent {
    friend void to_json(nlohmann::json& j, const Material& m);
    friend void from_json(const nlohmann::json& j, Material& m);

public:
    Material() {}
    ~Material() override {}

    /// <summary>
    /// UVTransformを計算してuvMat_に格納する
    /// </summary>
    void UpdateUvMatrix();

    void Initialize([[maybe_unused]] Entity* _entity) override;
    void Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, const std::string& _parentLabel) override;
    void Finalize() override;

    /// <summary>
    /// CustomTextureを指定したファイルから作成する
    /// </summary>
    /// <param name="_directory"></param>
    /// <param name="_filename"></param>
    void CreateCustomTextureFromTextureFile(const std::string& _directory, const std::string& _filename);
    /// <summary>
    /// CustomTextureを指定したテクスチャから作成する
    /// </summary>
    /// <param name="textureIndex">TextureManagerが持っているテクスチャのインデックス</param>
    void CreateCustomTextureFromTextureFile(int32_t textureIndex);

public:
    /// <summary>
    /// CustomTextureを表すデータ. textureと大差ないが、Material固有のものとして扱うために分けている
    /// </summary>
    struct CustomTextureData {
        CustomTextureData() = default;

        DxSrvDescriptor srv_{};
        DxResource resource_{};
    };

public:
    UVTransform uvTransform_;
    Matrix4x4 uvMat_ = MakeMatrix::Identity();

    Vec4f color_ = {1.f, 1.f, 1.f, 1.f};

    bool enableLighting_          = false;
    float shininess_              = 0.f;
    float environmentCoefficient_ = 0.1f;
    Vec3f specularColor_          = {1.f, 1.f, 1.f};

private:
    std::optional<CustomTextureData> customTexture_;

public:
    bool hasCustomTexture() const { return customTexture_.has_value(); }
    const std::optional<CustomTextureData>& getCustomTexture() const { return customTexture_; }
    void setCustomTexture(DxSrvDescriptor _srv, const DxResource& _resource) {
        if (!customTexture_) {
            customTexture_.emplace(CustomTextureData());
        }
        customTexture_->srv_      = _srv;
        customTexture_->resource_ = _resource;
    }
    void resetCustomTexture() { customTexture_.reset(); }

public:
    struct ConstantBuffer {
        Vec4f color;
        uint32_t enableLighting;
        float padding[3];
        Matrix4x4 uvTransform;
        float shininess;
        Vec3f specularColor;
        float environmentCoefficient;
        ConstantBuffer& operator=(const Material& material) {
            color                  = material.color_;
            enableLighting         = static_cast<uint32_t>(material.enableLighting_);
            uvTransform            = material.uvMat_;
            shininess              = material.shininess_;
            specularColor          = material.specularColor_;
            environmentCoefficient = material.environmentCoefficient_;
            return *this;
        }
    };
};
