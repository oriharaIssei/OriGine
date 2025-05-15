#pragma once

/// stl
#include <memory>

/// engine
#include "component/transform/CameraTransform.h"
#include "directX12/IConstantBuffer.h"
#include "module/IModule.h"

class DebugCamera;

class CameraManager
    : public IModule {
public:
    static CameraManager* getInstance();

    void Initialize();
    void UpdateMatrix();
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

    Matrix4x4 viewPortMat_;

public:
    const Matrix4x4& getViewPortMat() const { return viewPortMat_; }
    Vec2f world2Screen(const Matrix4x4& _worldMat) const;

    const CameraTransform& getTransform() const { return cTransform_.openData_; }
    void setTransform(const CameraTransform& transform) { cTransform_.openData_ = transform; }

    void DataConvertToBuffer() {
        cTransform_.ConvertToBuffer();
    }
    void setBufferForRootParameter(ID3D12GraphicsCommandList* cmdList, uint32_t rootParameterNum) {
        cTransform_.SetForRootParameter(cmdList, rootParameterNum);
    }
};
