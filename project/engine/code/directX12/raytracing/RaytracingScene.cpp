#include "RaytracingScene.h"

using namespace OriGine;

void RaytracingScene::Initialize() {
    if (!blasMap_.empty()) {
        blasMap_.clear();
    }
    tlas_.Initialize();
}

void RaytracingScene::Finalize() {
    if (!blasMap_.empty()) {
        for (auto& [meshHandle, blas] : blasMap_) {
            blas.Finalize();
        }
    }
    tlas_.Finalize();
}

void RaytracingScene::UpdateBlases(
    ID3D12Device8* _device,
    ID3D12GraphicsCommandList6* _commandList,
    const std::vector<RaytracingMeshEntry>& _entries) {
    for (const auto& entry : _entries) {
        auto it = blasMap_.find(entry.meshHandle);
        if (it == blasMap_.end()) {
            // BLAS未作成なら作成
            BottomLevelAccelerationStructure blas;
            blas.CreateResource(
                _device,
                _commandList,
                entry.mesh->GetVertexBuffer().GetResource()->GetGPUVirtualAddress(),
                entry.mesh->GetVertexSize(),
                entry.mesh->GetIndexBuffer().GetResource()->GetGPUVirtualAddress(),
                entry.mesh->GetIndexSize(),
                entry.isDynamic);
            blasMap_.emplace(entry.meshHandle, std::move(blas));
        } else {
            // BLAS更新
            it->second.Update(_commandList);
        }
    }
}

void RaytracingScene::UpdateTlas(ID3D12Device8* _device, ID3D12GraphicsCommandList6* _commandList, const std::vector<RayTracingInstance>& _instances) {
    if (_instances.empty()) {
        return;
    }

    if (!tlasIsCreated_) {
        tlas_.CreateResource(_device, _commandList, _instances, true);
        tlasIsCreated_ = true;
    } else {
        tlas_.Update(_device, _commandList, _instances);
    }
}

bool OriGine::RaytracingScene::IsEmpty() const {
    return !tlasIsCreated_ || blasMap_.empty();
}
