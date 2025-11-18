#pragma once

/// stl
#include <memory>

/// engine
#include "component/transform/CameraTransform.h"
#include "directX12/buffer/IConstantBuffer.h"


class DebugCamera;

/// <summary>
/// カメラを管理するクラス. cTransform_を通してカメラの情報をGPUに送る.
/// </summary>
class CameraManager{
public:
    static CameraManager* GetInstance();

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
    const CameraTransform& GetTransform() const { return cTransform_.openData_; }
    void SetTransform(const CameraTransform& transform) { cTransform_.openData_ = transform; }

    void DataConvertToBuffer() {
        cTransform_.ConvertToBuffer();
    }
    void SetBufferForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) {
        cTransform_.SetForRootParameter(cmdList, rootParameterNum);
    }
};
