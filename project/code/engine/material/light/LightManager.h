#pragma once

#include <memory>

#include "Assets/IAsset.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
#include "globalVariables/SerializedField.h"
#include "Module/IModule.h"

#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

#include "directX12/DxSrvArray.h"

struct LightCounts{
	SerializedField<int32_t> directionalLightNum{"LightManager","LightCounts","directionalLightNum"};
	SerializedField<int32_t> spotLightNum{"LightManager","LightCounts","spotLightNum"};
	SerializedField<int32_t> pointLightNum{"LightManager","LightCounts","pointLightNum"};

	struct ConstantBuffer{
		int32_t directionalLightNum;
		int32_t spotLightNum;
		int32_t pointLightNum;

		ConstantBuffer& operator=(const LightCounts& light){
			this->directionalLightNum = light.directionalLightNum;
			this->spotLightNum = light.spotLightNum;
			this->pointLightNum = light.pointLightNum;
			return *this;
		}
	};
};

class LightManager
	:IModule{
public:
	LightManager() = default;
	~LightManager() = default;

	void Init();
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