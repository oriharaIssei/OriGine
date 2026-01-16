#pragma once

/// math
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Vector4.h"

namespace OriGine {

/// <summary>
/// Particle Transform コンポーネント
/// </summary>
struct ParticleTransform {
    friend void to_json(nlohmann::json& _j, const ParticleTransform& _comp);
    friend void from_json(const nlohmann::json& _j, ParticleTransform& _comp);

    ParticleTransform() = default;

    /// <summary>
    /// コピーコンストラクタ
    /// </summary>
    /// <param name="_comp"></param>
    ParticleTransform(const ParticleTransform& _comp) {
        this->scale     = _comp.scale;
        this->rotate    = _comp.rotate;
        this->translate = _comp.translate;

        this->uvScale     = _comp.uvScale;
        this->uvRotate    = _comp.uvRotate;
        this->uvTranslate = _comp.uvTranslate;

        color = _comp.color;

        UpdateMatrix();
    }

    ParticleTransform& operator=(const ParticleTransform& _comp) {
        this->scale     = _comp.scale;
        this->rotate    = _comp.rotate;
        this->translate = _comp.translate;

        this->uvScale     = _comp.uvScale;
        this->uvRotate    = _comp.uvRotate;
        this->uvTranslate = _comp.uvTranslate;

        color = _comp.color;

        UpdateMatrix();
        return *this;
    }

    ~ParticleTransform() = default;

    Vec3f scale, rotate, translate;
    Matrix4x4 worldMat;
    Matrix4x4* parentWorldMat = nullptr;

    Vec3f uvScale, uvRotate, uvTranslate;
    Matrix4x4 uvMat;

    Vec4f color;

    void UpdateMatrix();

public:
    struct ConstantBuffer {
        Matrix4x4 worldMat;
        Matrix4x4 uvMat;
        Vec4f color;

        ConstantBuffer& operator=(const ParticleTransform& _comp) {
            worldMat = _comp.worldMat;
            uvMat    = _comp.uvMat;
            color    = _comp.color;
            return *this;
        }
    };
};

} // namespace OriGine
