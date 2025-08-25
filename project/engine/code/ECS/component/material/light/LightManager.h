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
#include "directX12/DxDescriptor.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"

#include "globalVariables/SerializedField.h"

struct LightCounts {
    int32_t directionalLightNum = 0;
    int32_t pointLightNum       = 0;
    int32_t spotLightNum        = 0;

    struct ConstantBuffer {
        int32_t directionalLightNum;
        int32_t pointLightNum;
        int32_t spotLightNum;

        ConstantBuffer& operator=(const LightCounts& light) {
            this->directionalLightNum = light.directionalLightNum;
            this->pointLightNum       = light.pointLightNum;
            this->spotLightNum        = light.spotLightNum;
            return *this;
        }
    };
};

class LightManager {
public:
    static LightManager* getInstance() {
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

    void SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, int32_t _lightCountIndex, int32_t _directionalLightIndex, int32_t _pointLightIndex, int32_t _spotLightIndex);

private:
    SerializedField<int32_t> directionalLightSize_{"LightManager", "LightCounts", "directionalLightSize"};
    SerializedField<int32_t> pointLightSize_{"LightManager", "LightCounts", "pointLightSize"};
    SerializedField<int32_t> spotLightSize_{"LightManager", "LightCounts", "spotLightSize"};

    IConstantBuffer<LightCounts> lightCounts_;

    IStructuredBuffer<DirectionalLight> directionalLights_;
    IStructuredBuffer<PointLight> pointLights_;
    IStructuredBuffer<SpotLight> spotLights_;

public:
    size_t getDirectionalLightCount() const { return directionalLights_.openData_.size(); }
    size_t getPointLightCount() const { return pointLights_.openData_.size(); }
    size_t getSpotLightCount() const { return spotLights_.openData_.size(); }

    size_t getDirectionalLightCapacity() const { return directionalLights_.capacity(); }
    size_t getPointLightCapacity() const { return pointLights_.capacity(); }
    size_t getSpotLightCapacity() const { return spotLights_.capacity(); }

    void pushDirectionalLight(const DirectionalLight& light);
    void pushPointLight(const PointLight& light);
    void pushSpotLight(const SpotLight& light);

    void clearLights() {
        directionalLights_.openData_.clear();
        pointLights_.openData_.clear();
        spotLights_.openData_.clear();
    }
};
