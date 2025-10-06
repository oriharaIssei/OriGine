#pragma once
#include "component/IComponent.h"

/// engine
#include "directX12/IConstantBuffer.h"
#include "directX12/SimpleConstantBuffer.h"

/// component
#include "component/material/Material.h"

/// math
#include <math/Vector4.h>

struct DissolveParamData {
public:
    DissolveParamData()  = default;
    ~DissolveParamData() = default;

    float threshold    = 0.5f;
    float edgeWidth    = 0.1f;
    Vec4f outLineColor = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);

public:
    struct ConstantBuffer {
        ConstantBuffer() = default;

        float threshold;
        float edgeWidth;
        float pad[2];
        Vec4f outLineColor;

        ConstantBuffer& operator=(const DissolveParamData& data) {
            threshold    = data.threshold;
            edgeWidth    = data.edgeWidth;
            outLineColor = data.outLineColor;

            return *this;
        }
    };
};

class DissolveEffectParam
    : public IComponent {
    friend void to_json(nlohmann::json& j, const DissolveEffectParam& param);
    friend void from_json(const nlohmann::json& j, DissolveEffectParam& param);

public:
    DissolveEffectParam()           = default;
    ~DissolveEffectParam() override = default;

    void Initialize(GameEntity* _entity) override;
    void Finalize() override;

    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;

    void LoadTexture(const std::string& filePath);

    void Play();
    void Stop() {
        isActive_ = false;
    }

private:
    bool isActive_ = false; // 有効かどうか
    IConstantBuffer<DissolveParamData> paramBuffer_;

    std::string textureFilePath_ = ""; // テクスチャのファイルパス
    uint32_t textureIndex_       = 0;

    int32_t materialIndex_ = -1; // マテリアルコンポーネントのインデックス
    SimpleConstantBuffer<ColorAndUvTransform> materialBuffer_;

public:
    bool isActive() const {
        return isActive_;
    }

    const IConstantBuffer<DissolveParamData>& getDissolveBuffer() const {
        return paramBuffer_;
    }
    IConstantBuffer<DissolveParamData>& getDissolveBufferRef() {
        return paramBuffer_;
    }

    SimpleConstantBuffer<ColorAndUvTransform>& getMaterialBuffer() {
        return materialBuffer_;
    }

    int32_t getMaterialIndex() const {
        return materialIndex_;
    }
    void setMaterialIndex(int32_t index) {
        materialIndex_ = index;
    }

    float getThreshold() const {
        return paramBuffer_->threshold;
    }
    void setThreshold(float threshold) {
        paramBuffer_->threshold = threshold;
    }
    float getEdgeWidth() const {
        return paramBuffer_->edgeWidth;
    }
    void setEdgeWidth(float edgeWidth) {
        paramBuffer_->edgeWidth = edgeWidth;
    }
    const Vec4f& getOutLineColor() const {
        return paramBuffer_->outLineColor;
    }
    void setOutLineColor(const Vec4f& color) {
        paramBuffer_->outLineColor = color;
    }

    uint32_t getTextureIndex() const {
        return textureIndex_;
    }
};
