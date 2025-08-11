#pragma once

#include "component/IComponent.h"

/// stl
#include <string>

/// engine
// directX12 Object
#include "directX12/DxDescriptor.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
#include "directX12/Mesh.h"
#include "directX12/ShaderManager.h"
/// ECS
// component
#include "component/material/Material.h"

/// math
#include <cstdint>
#include <math/Matrix4x4.h>
#include <math/Vector3.h>
#include <math/Vector4.h>

struct GpuParticleData {
    Vec3f scale;
    Vec3f translate;
    float lifeTime;
    Vec3f velocity;
    float maxTime;
    Vec4f color;

    struct ConstantBuffer {
        Vec3f scale;
        Vec3f translate;
        float lifeTime;
        Vec3f velocity;
        float maxTime;
        Vec4f color;
        ConstantBuffer& operator=(const GpuParticleData& other) {
            scale       = other.scale;
            translate   = other.translate;
            lifeTime    = other.lifeTime;
            velocity    = other.velocity;
            maxTime = other.maxTime;
            color       = other.color;
            return *this;
        }
    };
};
struct GpuParticleEmitSphere {
    Vec3f center = Vec3f(0.f, 0.f, 0.f);
    Vec3f size   = Vec3f(0.f, 0.f, 0.f);

    Vec3f minVelocity = Vec3f(0.f, 0.f, 0.f);
    Vec3f maxVelocity = Vec3f(0.f, 0.f, 0.f);

    Vec3f minScale = Vec3f(1.f, 1.f, 1.f);
    Vec3f maxScale = Vec3f(1.f, 1.f, 1.f);

    Vec3f minColor = Vec3f(1.f, 1.f, 1.f);
    Vec3f maxColor = Vec3f(1.f, 1.f, 1.f);

    uint32_t minParticleCount = 1; // 最小パーティクル数
    uint32_t maxParticleCount = 1; // 最大パーティクル数

    float minLifeParticleTime = 0.f; // 最小残り時間
    float maxLifeParticleTime = 0.f; // 最大残り時間

    float frequency     = 0.f;
    float frequencyTime = 0.f;
    uint32_t isEmit     = 0; // 0:emitしない, 1:emitする

    uint32_t isBox      = 0; // 0:球形, 1:立方体
    uint32_t isEmitEdge = 0; // 0:範囲内全てからEmit, 1:エッジからemitする

    uint32_t particleSize = 1024; // パーティクルのサイズ

    struct ConstantBuffer {
        Vec3f minColor            = Vec3f(1.f, 1.f, 1.f);
        float minLifeParticleTime = 0.f; // 最小残り時間
        Vec3f maxColor            = Vec3f(1.f, 1.f, 1.f);
        float maxLifeParticleTime = 0.f; // 最大残り時間

        // 16バイト: center + minParticleCount
        Vec3f center              = Vec3f(0.f, 0.f, 0.f);
        uint32_t minParticleCount = 1;

        // 16バイト: size + maxParticleCount
        Vec3f size                = Vec3f(0.f, 0.f, 0.f);
        uint32_t maxParticleCount = 1;

        // 16バイト: minVelocity + isBox
        Vec3f minVelocity = Vec3f(0.f, 0.f, 0.f);
        uint32_t isBox    = 0;

        // 16バイト: maxVelocity + isEmit
        Vec3f maxVelocity = Vec3f(0.f, 0.f, 0.f);
        uint32_t isEmit   = 0;

        // 16バイト: minScale + isEmitEdge
        Vec3f minScale      = Vec3f(1.f, 1.f, 1.f);
        uint32_t isEmitEdge = 0;

        // 16バイト: maxScale + pad
        Vec3f maxScale        = Vec3f(1.f, 1.f, 1.f);
        uint32_t particleSize = 1024; // パーティクルのサイズ

        ConstantBuffer& operator=(const GpuParticleEmitSphere& other) {
            minColor            = other.minColor;
            minLifeParticleTime = other.minLifeParticleTime;
            maxColor            = other.maxColor;
            maxLifeParticleTime = other.maxLifeParticleTime;

            center           = other.center;
            minParticleCount = other.minParticleCount;

            size             = other.size;
            maxParticleCount = other.maxParticleCount;

            minVelocity = other.minVelocity;
            isBox       = other.isBox;

            maxVelocity = other.maxVelocity;
            isEmit      = other.isEmit;

            minScale   = other.minScale;
            isEmitEdge = other.isEmitEdge;

            maxScale     = other.maxScale;
            particleSize = other.particleSize;

            return *this;
        }
    };
};

struct PerView {
    Matrix4x4 viewProjectionMat;
    Matrix4x4 billboardMat;

    struct ConstantBuffer {
        Matrix4x4 viewProjectionMat;
        Matrix4x4 billboardMat;

        ConstantBuffer& operator=(const PerView& other) {
            viewProjectionMat = other.viewProjectionMat;
            billboardMat      = other.billboardMat;
            return *this;
        }
    };
};

class GpuParticleEmitter
    : public IComponent {
    friend void to_json(nlohmann::json& j, const GpuParticleEmitter& p);
    friend void from_json(const nlohmann::json& j, GpuParticleEmitter& p);

public:
    GpuParticleEmitter()  = default;
    ~GpuParticleEmitter() = default;

    void Initialize(GameEntity* _entity) override;

    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;

    void Finalize() override;

    /// <summary>
    /// Buffer を作成
    /// </summary>
    void CreateBuffer();

    void LoadTexture(const std::string& _path);

    void Play();
    void Stop();

private:
    bool isActive_       = false;
    BlendMode blendMode_ = BlendMode::Alpha;

    TextureMesh mesh_;

    DxResource particleResource_;
    std::shared_ptr<DxSrvDescriptor> particleSrvDescriptor_ = nullptr;
    std::shared_ptr<DxUavDescriptor> particleUavDescriptor_ = nullptr;

    DxResource freeListResource_;
    std::shared_ptr<DxUavDescriptor> freeListUavDescriptor_ = nullptr;
    DxResource freeIndexResource_;
    std::shared_ptr<DxUavDescriptor> freeIndexUavDescriptor_ = nullptr;

    IConstantBuffer<Material> materialBuffer_;
    IConstantBuffer<GpuParticleEmitSphere> shapeBuffer_;

    std::string texturePath_ = "";
    uint32_t textureIndex_   = 0;

public:
    bool isActive() const { return isActive_; }
    uint32_t getParticleSize() const { return shapeBuffer_->particleSize; }
    BlendMode getBlendMode() const { return blendMode_; }

    const TextureMesh& getMesh() const { return mesh_; }
    TextureMesh& getMeshRef() { return mesh_; }

    const DxResource& getParticleResource() const { return particleResource_; }
    std::shared_ptr<DxSrvDescriptor> getParticleSrvDescriptor() const {
        return particleSrvDescriptor_;
    }
    std::shared_ptr<DxUavDescriptor> getParticleUavDescriptor() const {
        return particleUavDescriptor_;
    }

    const DxResource& getFreeIndexResource() const { return freeIndexResource_; }
    std::shared_ptr<DxUavDescriptor> getFreeIndexUavDescriptor() const {
        return freeIndexUavDescriptor_;
    }

    const DxResource& getFreeListResource() const { return freeListResource_; }
    std::shared_ptr<DxUavDescriptor> getFreeListUavDescriptor() const {
        return freeListUavDescriptor_;
    }

    const IConstantBuffer<Material>& getMaterialBuffer() const { return materialBuffer_; }
    const IConstantBuffer<GpuParticleEmitSphere>& getShapeBuffer() const {
        return shapeBuffer_;
    }

    const GpuParticleEmitSphere& getShapeBufferData() const {
        return shapeBuffer_.openData_;
    }
    GpuParticleEmitSphere& getShapeBufferDataRef() {
        return shapeBuffer_.openData_;
    }

    const std::string& getTexturePath() const { return texturePath_; }
    uint32_t getTextureIndex() const { return textureIndex_; }
};
