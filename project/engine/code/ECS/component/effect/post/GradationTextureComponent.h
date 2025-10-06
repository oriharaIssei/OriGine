#pragma once

#include "component/IComponent.h"

/// stl
#include <string>

/// engine
// directX12
#include "directX12/IConstantBuffer.h"
#include "directX12/SimpleConstantBuffer.h"

// component
#include "component/material/Material.h"

/// math
#include "mathEnv.h"

struct GradationParam {
    ColorChannel inputChannel  = ColorChannel::R;
    ColorChannel outputChannel = ColorChannel::R;
    struct ConstantBuffer {
        int inputChannel  = 0;
        int outputChannel = 0;
        float padding[2];

        ConstantBuffer& operator=(const GradationParam& _param) {
            inputChannel  = static_cast<int>(_param.inputChannel);
            outputChannel = static_cast<int>(_param.outputChannel);
            return *this;
        }
    };
};

class GradationTextureComponent
    : public IComponent {
    friend void to_json(nlohmann::json& j, const GradationTextureComponent& _g);
    friend void from_json(const nlohmann::json& j, GradationTextureComponent& _g);

public:
    GradationTextureComponent()           = default;
    ~GradationTextureComponent() override = default;

    void Initialize(GameEntity* _entity) override;
    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;
    void Finalize() override;

    void LoadTexture(const std::string& _texturePath);

private:
    bool isActive_ = true;

    IConstantBuffer<GradationParam> paramBuff_;
    SimpleConstantBuffer<ColorAndUvTransform> materialBuff_;

    int32_t textureIndex_ = 0;
    std::string texturePath_;

    int32_t materialIndex_ = -1;

public:
    bool isActive() const { return isActive_; }
    void setActive(bool _active) { isActive_ = _active; }

    int32_t getMaterialIndex() const { return materialIndex_; }
    void setMaterialIndex(int32_t _index) { materialIndex_ = _index; }

    IConstantBuffer<GradationParam>& getParamBuff() { return paramBuff_; }
    SimpleConstantBuffer<ColorAndUvTransform>& getMaterialBuff() { return materialBuff_; }

    int32_t getTextureIndex() const { return textureIndex_; }
    const std::string& getTexturePath() const { return texturePath_; }
};
