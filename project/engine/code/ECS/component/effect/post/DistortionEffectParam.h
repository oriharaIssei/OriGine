#pragma once

#include "component/IComponent.h"

/// stl
#include <memory>
#include <string>
#include <vector>

/// engine
// directX12
#include "directX12/IConstantBuffer.h"
#include "directX12/SimpleConstantBuffer.h"
class PrimitiveMeshRendererBase;
enum class PrimitiveType : int32_t;

// component
#include "component/material/Material.h"

struct DistortionParamData {
    float distortionBias     = 0.f;
    float distortionStrength = 0.f;

    struct ConstantBuffer {
        float distortionBias;
        float distortionStrength;
        ConstantBuffer& operator=(const DistortionParamData& transform) {
            distortionBias     = transform.distortionBias;
            distortionStrength = transform.distortionStrength;
            return *this;
        }
    };
};

class DistortionEffectParam
    : public IComponent {
    friend void to_json(nlohmann::json& j, const DistortionEffectParam& param);
    friend void from_json(const nlohmann::json& j, DistortionEffectParam& param);

public:
    DistortionEffectParam() {}
    ~DistortionEffectParam() override {}

    void Initialize(GameEntity* _hostEntity) override;
    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel);
    void Finalize();

    void LoadTexture(const std::string& _path);

private:
    bool isActive_        = true;
    bool use3dObjectList_ = false; // distortionObjects_ を使うかどうか (false の場合 textureを使用した2dのエフェクトとなる)

    int32_t materialIndex_ = -1;

    int32_t textureIndex_    = 0; // -1 の場合はテクスチャを使用しない
    std::string texturePath_ = "";

    IConstantBuffer<DistortionParamData> effectParamData_;
    SimpleConstantBuffer<UVTransform> uvTransformBuffer_;

    std::vector<std::pair<std::shared_ptr<PrimitiveMeshRendererBase>, PrimitiveType>> distortionObjects_;

public:
    bool getIsActive() const { return isActive_; }
    void setIsActive(bool active) { isActive_ = active; }

    bool getUse3dObjectList() const { return use3dObjectList_; }
    void setUse3dObjectList(bool use) { use3dObjectList_ = use; }

    const std::string& getTexturePath() const { return texturePath_; }
    int32_t getTextureIndex() const { return textureIndex_; }

    int32_t getMaterialIndex() const { return materialIndex_; }
    void setMaterialIndex(int32_t index) { materialIndex_ = index; }

    DistortionParamData& getEffectParamData() {
        return effectParamData_.openData_;
    }
    IConstantBuffer<DistortionParamData>& getEffectParamBuffer() {
        return effectParamData_;
    }
    SimpleConstantBuffer<UVTransform>& getUVTransformBuffer() {
        return uvTransformBuffer_;
    }

    std::vector<std::pair<std::shared_ptr<PrimitiveMeshRendererBase>, PrimitiveType>>& getDistortionObjects() {
        return distortionObjects_;
    }
};
