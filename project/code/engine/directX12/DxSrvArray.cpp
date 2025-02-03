#include "Engine.h"
#include "directX12/DxCommand.h"
#include "material/texture/TextureManager.h"

#include <assert.h>

void DxSrvArray::Finalize() {
    for (auto& resource : resources_) {
        if (resource != nullptr) {
            resource.Reset();
        }
    }
}

uint32_t DxSrvArray::CreateView(ID3D12Device* device, D3D12_SHADER_RESOURCE_VIEW_DESC& viewDesc, ID3D12Resource* resource) {
    DxHeap* dxHeap = DxHeap::getInstance();

    uint32_t index = 0;
    for (; index < resources_.size(); index++) {
        if (resources_[index].Get() != nullptr) {
            continue;
        }
        break;
    }

    if (index >= resources_.size()) {
        return index + 1;
    }

    device->CreateShaderResourceView(resource, &viewDesc, dxHeap->getSrvCpuHandle(arrayStartLocation_ + index));
    resources_[index] = resource;

    std::cout << "Created SRV at index: " << index << " for resource: " << resource << std::endl; // デバッグ情報を追加

    return index;
}

void DxSrvArray::DestroyView(uint32_t srvIndex) {
    resources_[srvIndex].Reset();
    resources_[srvIndex] = nullptr;
}

void DxSrvArray::Init(uint32_t size, uint32_t arrayLocation) {
    size_               = size;
    arrayStartLocation_ = arrayLocation;
    resources_.resize(size_);
}
