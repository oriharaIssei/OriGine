#pragma once

/// interface
#include "editor/IEditor.h"

/// stl
#include <memory>

/// engine
// lights
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
// directX12Object
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/buffer/IStructuredBuffer.h"

/// util
#include "globalVariables/SerializedField.h"

namespace OriGine {

/// <summary>
/// ライトの数を管理する構造体
/// </summary>
struct LightCounts {
    int32_t directionalLightNum = 0;
    int32_t pointLightNum       = 0;
    int32_t spotLightNum        = 0;

    struct ConstantBuffer {
        int32_t directionalLightNum;
        int32_t pointLightNum;
        int32_t spotLightNum;

        ConstantBuffer& operator=(const LightCounts& _light) {
            this->directionalLightNum = _light.directionalLightNum;
            this->pointLightNum       = _light.pointLightNum;
            this->spotLightNum        = _light.spotLightNum;
            return *this;
        }
    };
};

/// <summary>
/// ライトを管理するクラス
/// </summary>
class LightManager {
public:
    static LightManager* GetInstance() {
        static LightManager instance;
        return &instance;
    }
    LightManager();
    ~LightManager();
    LightManager(const LightManager&)            = delete;
    LightManager& operator=(const LightManager&) = delete;

    void Initialize();
    void Update();
    void Finalize();

    void SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList, int32_t _lightCountIndex, int32_t _directionalLightIndex, int32_t _pointLightIndex, int32_t _spotLightIndex);

private:
    SerializedField<int32_t> directionalLightSize_{"LightManager", "LightCounts", "directionalLightSize"};
    SerializedField<int32_t> pointLightSize_{"LightManager", "LightCounts", "pointLightSize"};
    SerializedField<int32_t> spotLightSize_{"LightManager", "LightCounts", "spotLightSize"};

    IConstantBuffer<LightCounts> lightCounts_;

    IStructuredBuffer<DirectionalLight> directionalLights_;
    IStructuredBuffer<PointLight> pointLights_;
    IStructuredBuffer<SpotLight> spotLights_;

public:
    size_t GetDirectionalLightCount() const { return directionalLights_.openData_.size(); }
    size_t GetPointLightCount() const { return pointLights_.openData_.size(); }
    size_t GetSpotLightCount() const { return spotLights_.openData_.size(); }

    size_t GetDirectionalLightCapacity() const { return directionalLights_.Capacity(); }
    size_t GetPointLightCapacity() const { return pointLights_.Capacity(); }
    size_t GetSpotLightCapacity() const { return spotLights_.Capacity(); }

    /// <summary>
    /// DirectionalLightを追加する
    /// </summary>
    /// <param name="_light"></param>
    void PushDirectionalLight(const DirectionalLight& _light);
    /// <summary>
    /// PointLightを追加する
    /// </summary>
    void PushPointLight(const PointLight& _light);
    /// <summary>
    /// SpotLightを追加する
    /// </summary>
    /// <param name="_light"></param>
    void PushSpotLight(const SpotLight& _light);

    /// <summary>
    /// Lightをclearする
    /// </summary>
    void ClearLights() {
        directionalLights_.openData_.clear();
        pointLights_.openData_.clear();
        spotLights_.openData_.clear();
    }
};

} // namespace OriGine
