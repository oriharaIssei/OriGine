#pragma once
#include "system/ISystem.h"

/// engine
// drecitX12
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

/// <summary>
/// Smoothing に 使用する BoxFilter のサイズ
/// </summary>
struct BoxFilterSize {
    float x = 0.0f;
    float y = 0.0f;

public:
    struct ConstantBuffer {
        Vec2f boxSize;
        ConstantBuffer& operator=(const BoxFilterSize& _size) {
            boxSize = Vec2f(_size.x, _size.y);
            return *this;
        }
    };
};

/// <summary>
/// SmoothingEffect をかけるシステム
/// </summary>
class SmoothingEffect
    : public ISystem {
public:
    SmoothingEffect() : ISystem(SystemCategory::PostRender) {}
    ~SmoothingEffect() override = default;

    void Initialize() override;
    void Update() override;
    void Finalize();

protected:
    void CreatePSO();

    void Render();

protected:
    PipelineStateObj* pso_                = nullptr;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;

    IConstantBuffer<BoxFilterSize> boxFilterSize_;

public:
    Vec2f getBoxFilterSize() const {
        return Vec2f(boxFilterSize_->x, boxFilterSize_->y);
    }
    void setBoxFilterSize(float x, float y) {
        boxFilterSize_->x = x;
        boxFilterSize_->y = y;
        boxFilterSize_.ConvertToBuffer();
    }
};
