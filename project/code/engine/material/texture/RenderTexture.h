#pragma once

#include <stdint.h>
#include <string>

#include "directX12/DxCommand.h"
#include "directX12/DxCommand.h"
#include "directX12/DxResource.h"
#include "directX12/DxRtvArray.h"
#include "directX12/DxSrvArray.h"

#include "Vector2.h"
#include "Vector4.h"

#include "directX12/PipelineStateObj.h"

class RenderTexture{
public:
	RenderTexture(DxCommand* dxCom,DxRtvArray* rtvArray,DxSrvArray* srvArray):dxCommand_(dxCom),rtvArray_(rtvArray),srvArray_(srvArray){}
	~RenderTexture() = default;

	static void Awake();
	void Init(const Vector2& textureSize,DXGI_FORMAT format,const Vector4& _clearColor);

	void PreDraw();
	void PostDraw();

	void DrawTexture();
private:
	static PipelineStateObj* pso_;
	DxResource resource_;

	DxCommand* dxCommand_;

	uint32_t rtvIndex_;
	uint32_t srvIndex_;

	DxRtvArray* rtvArray_;
	DxSrvArray* srvArray_;

	Vector2 textureSize_;
	Vector4 clearColor_;
};