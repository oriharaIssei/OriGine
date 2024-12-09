#pragma once

#include "directX12/IConstantBuffer.h"
#include "transform/CameraTransform.h"

class Camera{
public:
	static Camera* getInstance();

	void Init();

	void Finalize();
private:
	Camera() = default;
	Camera(const Camera&) = delete;
	Camera* operator=(const Camera&) = delete;
private:
	IConstantBuffer<CameraTransform> cTransform_;
public:
	const CameraTransform& getTransform()const{ cTransform_.openData_; }
	void setTransform(const CameraTransform& transform){ cTransform_.openData_ = transform; }

	void DataConvertToBuffer(){
		cTransform_.ConvertToBuffer();
	}
	void setBufferForRootParameter(ID3D12GraphicsCommandList* cmdList,uint32_t rootParameterNum){
		cTransform_.SetForRootParameter(cmdList,rootParameterNum);
	}
};