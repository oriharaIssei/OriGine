#pragma once

/// d3d12
#include <wrl.h>

#include <d3d12.h>

/// stl
#include <string>

#include <list>

namespace OriGine {
///=================================================
/// Pipeline State Object
///=================================================
struct PipelineStateObj {
    void Finalize();

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
};

}
