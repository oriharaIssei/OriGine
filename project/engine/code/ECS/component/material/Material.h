#pragma once

/// engine
// ECS
// directX12 object
#include "directX12/IConstantBuffer.h"

/// math
#include "Matrix4x4.h"
#include "stdint.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

struct UVTransform {
    Vec2f scale_     = Vec2f(1.f, 1.f);
    float rotate_    = 0.f;
    Vec2f translate_ = Vec2f(0.f, 0.f);
};

struct Material {
    friend void to_json(nlohmann::json& j, const Material& m);
    friend void from_json(const nlohmann::json& j, Material& m);

public:
    Material() {}
    ~Material() {}

    void UpdateUvMatrix();

#ifdef _DEBUG
    void DebugGui([[maybe_unused]] const std::string& _parentLabel);
#endif // _DEBUG

public:
    UVTransform uvTransform_;
    Matrix4x4 uvMat_ = MakeMatrix::Identity();

    Vec4f color_ = {1.f, 1.f, 1.f, 1.f};

    bool enableLighting_          = false;
    float shininess_              = 0.f;
    float environmentCoefficient_ = 0.1f;
    Vec3f specularColor_          = {1.f, 1.f, 1.f};

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
