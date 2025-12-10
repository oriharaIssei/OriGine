#pragma once

/// Microsoft
#include <wrl.h>

#include <d3d12.h>

/// stl
#include <list>
#include <string>

namespace OriGine {
///=================================================
/// Pipeline State Object
///=================================================
struct PipelineStateObj {
    void Finalize();

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
};

} // namespace OriGine
