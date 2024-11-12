#pragma once

#include <memory>

#include "DirectionalLight.h"
#include "directX12/IConstantBuffer.h"
#include "PointLight.h"
#include "SpotLight.h"

#include "directX12/DxSrvArray.h"

struct LightCounts{
	int32_t directionalLightNum;
	int32_t spotLightNum;
	int32_t pointLightNum;

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

class LightManager{
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