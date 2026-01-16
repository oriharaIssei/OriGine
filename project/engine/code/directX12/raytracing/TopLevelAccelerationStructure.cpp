#include "TopLevelAccelerationStructure.h"

using namespace OriGine;

TopLevelAccelerationStructure::TopLevelAccelerationStructure() {}
TopLevelAccelerationStructure::~TopLevelAccelerationStructure() {}

void TopLevelAccelerationStructure::Initialize() {}

void TopLevelAccelerationStructure::Finalize() {
    instanceResource_.Finalize();
    scratchResource_.Finalize();
    resultResource_.Finalize();
}

void TopLevelAccelerationStructure::CreateResource(ID3D12Device8* _device, ID3D12GraphicsCommandList6* _commandList, const std::vector<RayTracingInstance>& _instance, bool _allowUpdate) {
    allowUpdate_ = _allowUpdate;

    // instanceDescバッファ生成
    const UINT64 descBytes = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * _instance.size();
    instanceResource_.CreateUAVBuffer(
        _device,
        descBytes,
        D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_HEAP_TYPE_UPLOAD);

    // マッピング処理
    D3D12_RAYTRACING_INSTANCE_DESC* mapped = nullptr;
    instanceResource_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mapped));

    for (size_t i = 0; i < _instance.size(); ++i) {

        const RayTracingInstance& instance = _instance[i];
        auto& data                         = mapped[i];

        CopyMatrix3x4(data.Transform, Matrix4x4::Transpose(instance.matrix));
        data.InstanceID                          = instance.instanceID;
        data.InstanceContributionToHitGroupIndex = instance.hitGroupIdx;
        data.InstanceMask                        = instance.mask;
        data.Flags                               = instance.flags;
        data.AccelerationStructure               = instance.blas->GetGPUVirtualAddress();
    }
    instanceResource_.GetResource()->Unmap(0, nullptr);

    // Inputs
    inputs_               = {};
    inputs_.Type          = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    inputs_.Flags         = allowUpdate_ ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    inputs_.NumDescs      = static_cast<UINT>(_instance.size());
    inputs_.DescsLayout   = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs_.InstanceDescs = instanceResource_.GetResource()->GetGPUVirtualAddress();

    // サイズ問い合わせ
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info{};
    _device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs_, &info);

    scratchResource_.CreateUAVBuffer(
        _device,
        info.ScratchDataSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_HEAP_TYPE_DEFAULT);
    resultResource_.CreateUAVBuffer(
        _device,
        info.ResultDataMaxSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        D3D12_HEAP_TYPE_DEFAULT);

    // buildDesc設定
    buildDesc_.DestAccelerationStructureData    = resultResource_.GetResource()->GetGPUVirtualAddress();
    buildDesc_.ScratchAccelerationStructureData = scratchResource_.GetResource()->GetGPUVirtualAddress();
    buildDesc_.SourceAccelerationStructureData  = 0;
    buildDesc_.Inputs                           = inputs_;

    // build実行
    _commandList->BuildRaytracingAccelerationStructure(&buildDesc_, 0, nullptr);

    // バリア遷移
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource = resultResource_.GetResource().Get();
    _commandList->ResourceBarrier(1, &barrier);
}

void TopLevelAccelerationStructure::Update(ID3D12Device8* _device, ID3D12GraphicsCommandList6* _commandList, const std::vector<RayTracingInstance>& _instances) {
    if (!allowUpdate_) {
        return;
    }

    // インスタンスが変わったら再ビルドし直す
    if (static_cast<UINT>(_instances.size()) != inputs_.NumDescs) {
        CreateResource(_device, _commandList, _instances, allowUpdate_);
        return;
    }

    D3D12_RAYTRACING_INSTANCE_DESC* mapped = nullptr;
    instanceResource_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mapped));

    for (size_t i = 0; i < _instances.size(); ++i) {

        const RayTracingInstance& instance = _instances[i];
        auto& data                         = mapped[i];

        CopyMatrix3x4(data.Transform, Matrix4x4::Transpose(instance.matrix));
        data.InstanceID                          = instance.instanceID;
        data.InstanceContributionToHitGroupIndex = instance.hitGroupIdx;
        data.InstanceMask                        = instance.mask;
        data.Flags                               = instance.flags;
        data.AccelerationStructure               = instance.blas->GetGPUVirtualAddress();
    }
    instanceResource_.GetResource()->Unmap(0, nullptr);

    // PERFORM_UPDATEをセット
    buildDesc_.SourceAccelerationStructureData = resultResource_.GetResource()->GetGPUVirtualAddress();
    buildDesc_.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
    _commandList->BuildRaytracingAccelerationStructure(&buildDesc_, 0, nullptr);

    // バリア遷移
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource = resultResource_.GetResource().Get();
    _commandList->ResourceBarrier(1, &barrier);
}

void TopLevelAccelerationStructure::CopyMatrix3x4(float (&_dst)[3][4], const Matrix4x4& _src) {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 4; ++c) {
            _dst[r][c] = _src.m[r][c];
        }
    }
}
