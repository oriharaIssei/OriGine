#pragma once

/// parent
#include "MeshRenderer.h"

/// math
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include <DirectXMath.h>

struct SkyboxVertex {
    Vec4f position;
};
struct SkyboxMaterial {
    friend void to_json(nlohmann::json& j, const SkyboxMaterial& c);
    friend void from_json(const nlohmann::json& j, SkyboxMaterial& c);
    Vec4f color;

    struct ConstantBuffer {
        Vec4f color;
        ConstantBuffer& operator=(const SkyboxMaterial& _material) {
            this->color = _material.color;
            return *this;
        }
    };
};

class SkyboxRenderer
    : public MeshRenderer<Mesh<SkyboxVertex>, SkyboxVertex> {
    friend void to_json(nlohmann::json& j, const SkyboxRenderer& c);
    friend void from_json(const nlohmann::json& j, SkyboxRenderer& c);

public:
    SkyboxRenderer() : MeshRenderer() {}
    ~SkyboxRenderer() {}

    void Initialize(GameEntity* _hostEntity) override;
    bool Edit() override;

private:
    std::string filePath_;
    IConstantBuffer<Transform> transformBuff_;
    IConstantBuffer<SkyboxMaterial> materialBuff_;

    int32_t textureIndex_ = 0;

public:

    void setTextureIndex(int32_t _textureIndex) {
        textureIndex_ = _textureIndex;
    }
    int32_t getTextureIndex() const {
        return textureIndex_;
    }
    void setFilePath(const std::string& _filePath) {
        filePath_ = _filePath;
    }
    const std::string& getFilePath() const {
        return filePath_;
    }

    IConstantBuffer<SkyboxMaterial>& getMaterialBuff() {
        return materialBuff_;
    }
    IConstantBuffer<Transform>& getTransformBuff() {
        return transformBuff_;
    }
};

void to_json(nlohmann::json& j, const SkyboxMaterial& c);
void from_json(const nlohmann::json& j, SkyboxMaterial& c);
