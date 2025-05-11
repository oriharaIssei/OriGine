#pragma once

/// engine
// ECS
#include "assets/IAsset.h"
// directX12 object
#include "directX12/IConstantBuffer.h"

/// math
#include "Matrix4x4.h"
#include "stdint.h"
#include "Vector4.h"

struct Material
    : IAsset {
    friend void to_json(nlohmann::json& j, const Material& m);
    friend void from_json(const nlohmann::json& j, Material& m);

public:
    Material() {}
    ~Material() {}

    void UpdateUvMatrix();

#ifdef _DEBUG
    void DebugGui();
#endif // _DEBUG

public:
    Vec3f uvScale_     = {1.f, 1.f, 1.f};
    Vec3f uvRotate_    = {0.f, 0.f, 0.f};
    Vec3f uvTranslate_ = {0.f, 0.f, 0.f};
    Matrix4x4 uvMat_   = MakeMatrix::Identity();

    Vec4f color_ = {1.f, 1.f, 1.f, 1.f};

    bool enableLighting_ = 0;
    float shininess_     = 0.f;
    Vec3f specularColor_ = {1.f, 1.f, 1.f};

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
            enableLighting = static_cast<uint32_t>(material.enableLighting_);
            uvTransform    = material.uvMat_;
            shininess      = material.shininess_;
            specularColor  = material.specularColor_;
            return *this;
        }
    };
};
