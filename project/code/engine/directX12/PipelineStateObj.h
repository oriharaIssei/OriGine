#pragma once

#include <string>

#include <wrl.h>

#include <list>

#include <d3d12.h>

///=================================================
/// Pipeline State Object
///=================================================
struct PipelineStateObj {
	void Finalize();

	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr <ID3D12PipelineState> pipelineState = nullptr;
};