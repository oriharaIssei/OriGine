#pragma once

/// interface
#include "module/editor/IEditor.h"
#include "module/IModule.h"

/// stl
#include <memory>

/// engine
// lights
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
// directX12Object
#include "directX12/DxSrvArray.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
// lib
#include "globalVariables/SerializedField.h"

struct LightCounts {
    SerializedField<int32_t> directionalLightNum{"LightManager", "LightCounts", "directionalLightNum"};
    SerializedField<int32_t> pointLightNum{"LightManager", "LightCounts", "pointLightNum"};
    SerializedField<int32_t> spotLightNum{"LightManager", "LightCounts", "spotLightNum"};

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

class LightEditor;

class LightManager
    : public IModule {
    friend class LightEditor;

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

    void SetForRootParameter(ID3D12GraphicsCommandList* cmdList);

private:
    std::shared_ptr<DxSrvArray> srvArray_;

    IConstantBuffer<LightCounts> lightCounts_;

    IStructuredBuffer<DirectionalLight> directionalLights_;
    IStructuredBuffer<PointLight> pointLights_;
    IStructuredBuffer<SpotLight> spotLights_;
};

class LightEditor
    : public IEditor {
public:
    LightEditor();
    ~LightEditor();

    void Initialize();
    void Update() override;
    void Finalize()override{}
private:
    LightManager* lightManager_;
};
