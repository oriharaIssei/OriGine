#pragma once

//Interface
#include "Module/IModule.h"

///stl
// memory
#include <array>
#include <memory>
// basic
#include <stdint.h>
#include <string>

///engine
//dxObject
#include "directX12/DxCommand.h"
#include "directX12/DxResource.h"
#include "directX12/Object3dMesh.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

//math
#include <Matrix4x4.h>
#include <Vector2.h>
#include <Vector4.h>

class Sprite;
class SpriteCommon
    : public IModule {
    friend class Sprite;

public:
    static SpriteCommon* getInstance();

    void Init();

    void PreDraw();

    void Finalize();

private:
    BlendMode currentBlend_ = BlendMode::None;

    void CreatePSO();
    Matrix4x4 viewPortMat_;
    std::unique_ptr<DxCommand> dxCommand_;
    std::array<PipelineStateObj*, kBlendNum> pso_;
};
