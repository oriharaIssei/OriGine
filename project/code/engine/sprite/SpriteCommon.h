#pragma once

#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

#include <array>
#include <memory>

#include "directX12/DxCommand.h"
#include "directX12/DxResource.h"
#include "directX12/Object3dMesh.h"
#include "directX12/PipelineStateObj.h"
#include "Module/IModule.h"

#include <string>

#include <Matrix4x4.h>
#include <stdint.h>
#include <Vector2.h>
#include <Vector4.h>

class Sprite;
class SpriteCommon
	:public IModule{
	friend class Sprite;
public:
	static SpriteCommon* getInstance();

	void Init();

	void PreDraw();

	void Finalize();
	Sprite* Create(const std::string& textureFilePath);

private:
	BlendMode currentBlend_;

	void CreatePSO();
	Matrix4x4 viewPortMat_;
	std::unique_ptr<DxCommand> dxCommand_;
	std::array<PipelineStateObj*,kBlendNum> pso_;
};