#pragma once

/// stl
#include <memory>

/// engine
#include "component/transform/CameraTransform.h"
#include "directX12/IConstantBuffer.h"


class DebugCamera;

class CameraManager{
public:
    static CameraManager* getInstance();

    void Initialize();
    void DebugUpdate();
    void Finalize();

private:
    CameraManager();
    ~CameraManager();
    CameraManager(const CameraManager&)            = delete;
    CameraManager* operator=(const CameraManager&) = delete;

private:
    std::unique_ptr<DebugCamera> debugCamera_;
    IConstantBuffer<CameraTransform> cTransform_;

public:
    const CameraTransform& getTransform() const { return cTransform_.openData_; }
    void setTransform(const CameraTransform& transform) { cTransform_.openData_ = transform; }

    void DataConvertToBuffer() {
        cTransform_.ConvertToBuffer();
    }
    void setBufferForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) {
        cTransform_.SetForRootParameter(cmdList, rootParameterNum);
    }
};
