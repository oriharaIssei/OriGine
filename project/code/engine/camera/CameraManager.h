#pragma once

///stl
#include <memory>

///engine
#include "directX12/IConstantBuffer.h"
#include "module/IModule.h"
#include "transform/CameraTransform.h"

class CameraManager
    : public IModule {
public:
    static CameraManager* getInstance();

    void Init();

    void Finalize();

private:
    CameraManager();
    ~CameraManager();
    CameraManager(const CameraManager&)            = delete;
    CameraManager* operator=(const CameraManager&) = delete;

private:
    IConstantBuffer<CameraTransform> cTransform_;

public:
    const CameraTransform& getTransform() const { cTransform_.openData_; }
    void setTransform(const CameraTransform& transform) { cTransform_.openData_ = transform; }

    void DataConvertToBuffer() {
        cTransform_.ConvertToBuffer();
    }
    void setBufferForRootParameter(ID3D12GraphicsCommandList* cmdList, uint32_t rootParameterNum) {
        cTransform_.SetForRootParameter(cmdList, rootParameterNum);
    }
};
