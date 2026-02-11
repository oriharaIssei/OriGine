#pragma once
#include "component/IComponent.h"

/// engine
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/buffer/SimpleConstantBuffer.h"

/// component
#include "component/material/Material.h"

/// math
#include <math/Vector4.h>

namespace OriGine {

/// <summary>
/// Dissolveエフェクトのパラメーター情報(GPUに送るやつ)
/// </summary>
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

/// <summary>
/// DissolveEffectのパラメーター
/// </summary>
class DissolveEffectParam
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const DissolveEffectParam& _comp);
    friend void from_json(const nlohmann::json& _j, DissolveEffectParam& _comp);

public:
    DissolveEffectParam()           = default;
    ~DissolveEffectParam() override = default;

    void Initialize(Scene* _scene, EntityHandle _entity) override;
    void Finalize() override;

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    void LoadTexture(const std::string& _filePath);

    /// <summary>
    /// エフェクトを有効化
    /// </summary>
    void Play();
    /// <summary>
    /// Effectを無効化
    /// </summary>
    void Stop() {
        isActive_ = false;
    }

private:
    bool isActive_ = false; // 有効かどうか
    IConstantBuffer<DissolveParamData> paramBuffer_;

    std::string textureFilePath_ = ""; // テクスチャのファイルパス
    size_t textureIndex_       = 0;

    int32_t materialIndex_ = -1; // マテリアルコンポーネントのインデックス
    SimpleConstantBuffer<ColorAndUvTransform> materialBuffer_;

public:
    bool IsActive() const {
        return isActive_;
    }

    const IConstantBuffer<DissolveParamData>& GetDissolveBuffer() const {
        return paramBuffer_;
    }
    IConstantBuffer<DissolveParamData>& GetDissolveBufferRef() {
        return paramBuffer_;
    }

    SimpleConstantBuffer<ColorAndUvTransform>& GetMaterialBuffer() {
        return materialBuffer_;
    }

    int32_t GetMaterialIndex() const {
        return materialIndex_;
    }
    void SetMaterialIndex(int32_t _index) {
        materialIndex_ = _index;
    }

    float GetThreshold() const {
        return paramBuffer_->threshold;
    }
    void SetThreshold(float _threshold) {
        paramBuffer_->threshold = _threshold;
    }
    float GetEdgeWidth() const {
        return paramBuffer_->edgeWidth;
    }
    void SetEdgeWidth(float _edgeWidth) {
        paramBuffer_->edgeWidth = _edgeWidth;
    }
    const Vec4f& GetOutLineColor() const {
        return paramBuffer_->outLineColor;
    }
    void SetOutLineColor(const Vec4f& _color) {
        paramBuffer_->outLineColor = _color;
    }

    size_t GetTextureIndex() const {
        return textureIndex_;
    }
};

} // namespace OriGine
