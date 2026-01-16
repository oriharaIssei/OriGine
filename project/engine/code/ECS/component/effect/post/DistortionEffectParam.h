#pragma once

#include "component/IComponent.h"

/// stl
#include <memory>
#include <string>
#include <vector>

/// engine
// directX12
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/buffer/SimpleConstantBuffer.h"

// component
#include "component/material/Material.h"

/// math
#include "math/Vector2.h"

namespace OriGine {
// 前方宣言
class PrimitiveMeshRendererBase;
enum class PrimitiveType : int32_t;

/// <summary>
/// Distortion Effectのパラメーター情報(GPUに送る)
/// </summary>
struct DistortionParamData {
    Vec2f distortionBias     = Vec2f(0.f, 0.f);
    Vec2f distortionStrength = Vec2f(1.f, 1.f);

    struct ConstantBuffer {
        Vec2f distortionBias;
        Vec2f distortionStrength;
        ConstantBuffer& operator=(const DistortionParamData& transform) {
            distortionBias     = transform.distortionBias;
            distortionStrength = transform.distortionStrength;
            return *this;
        }
    };
};

/// <summary>
/// DistortionEffect の パラメーター
/// </summary>
class DistortionEffectParam
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const DistortionEffectParam& _comp);
    friend void from_json(const nlohmann::json& _j, DistortionEffectParam& _comp);

public:
    DistortionEffectParam() {}
    ~DistortionEffectParam() override {}

    void Initialize(Scene* _scene, EntityHandle _hostEntity) override;
    void Edit(Scene* _scene, EntityHandle _handle, const std::string& _parentLabel);
    void Finalize();

    void LoadTexture(const std::string& _path);

private:
    bool isActive_        = true;
    bool use3dObjectList_ = false; // distortionObjects_ を使うかどうか (false の場合 textureを使用した2dのエフェクトとなる)

    int32_t materialIndex_ = -1;

    int32_t textureIndex_    = 0;
    std::string texturePath_ = "";

    IConstantBuffer<DistortionParamData> effectParamData_;
    SimpleConstantBuffer<ColorAndUvTransform> materialBuffer_;

    std::vector<std::pair<std::shared_ptr<PrimitiveMeshRendererBase>, PrimitiveType>> distortionObjects_;

public:
    bool GetIsActive() const { return isActive_; }
    void SetIsActive(bool _active) { isActive_ = _active; }

    bool GetUse3dObjectList() const { return use3dObjectList_; }
    void SetUse3dObjectList(bool _use) { use3dObjectList_ = _use; }

    const std::string& GetTexturePath() const { return texturePath_; }
    int32_t GetTextureIndex() const { return textureIndex_; }

    int32_t GetMaterialIndex() const { return materialIndex_; }
    void SetMaterialIndex(int32_t _index) { materialIndex_ = _index; }

    DistortionParamData& GetEffectParamData() {
        return effectParamData_.openData_;
    }
    IConstantBuffer<DistortionParamData>& GetEffectParamBuffer() {
        return effectParamData_;
    }
    SimpleConstantBuffer<ColorAndUvTransform>& GetMaterialBuffer() {
        return materialBuffer_;
    }

    std::vector<std::pair<std::shared_ptr<PrimitiveMeshRendererBase>, PrimitiveType>>& GetDistortionObjects() {
        return distortionObjects_;
    }
};

} // namespace OriGine
