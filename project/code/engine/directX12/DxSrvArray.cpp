#include "Engine.h"
#include "directX12/DxCommand.h"
#include "material/texture/TextureManager.h"

#include <algorithm>
#include <assert.h>

void DxSrvArray::Finalize() {
    for (auto& resource : resources_) {
        if (resource != nullptr) {
            resource.Reset();
        }
    }
}

uint32_t DxSrvArray::CreateView(ID3D12Device* device, D3D12_SHADER_RESOURCE_VIEW_DESC& viewDesc, ID3D12Resource* resource) {

    uint32_t index = 0;

    for (; index < resources_.size(); index++) {
        if (resources_[index].Get() != nullptr) {
            continue;
        }
        break;
    }

    if (index >= resources_.size()) {
        assert(false);
        return -1;
    }

    { // Create View
        DxHeap* dxHeap = DxHeap::getInstance();
        device->CreateShaderResourceView(resource, &viewDesc, dxHeap->getSrvCpuHandle(arrayStartLocation_ + index));
        resources_[index] = resource;
    }

    std::cout << "Created SRV at index: " << index << " for resource: " << resource << std::endl; // デバッグ情報を追加

    return index;
}

uint32_t DxSrvArray::preCreateView() {
    uint32_t index = 0;
    for (; index < resources_.size(); index++) {
        // 空きではない もしくは 予約済みなら次へ
        if (resources_[index].Get() != nullptr
            || IsPreCreated(index)) {
            continue;
        }
        break;
    }
    if (index >= resources_.size()) {
        assert(false);
    }

    preCreateIndexs_.push_back(index);
    std::sort(preCreateIndexs_.begin(), preCreateIndexs_.end());
    return index;
}

void DxSrvArray::CreateView(ID3D12Device* device, D3D12_SHADER_RESOURCE_VIEW_DESC& viewDesc, ID3D12Resource* resource, uint32_t srvIndex) {
    if (resources_[srvIndex].Get() == nullptr) {
        // 予約済みなら予約を削除
        if (IsPreCreated(srvIndex)) {
            std::erase_if(preCreateIndexs_, [srvIndex](uint32_t x) {
                return x == srvIndex;
            });
            std::sort(preCreateIndexs_.begin(), preCreateIndexs_.end());
        }

        DxHeap* dxHeap = DxHeap::getInstance();
        device->CreateShaderResourceView(resource, &viewDesc, dxHeap->getSrvCpuHandle(arrayStartLocation_ + srvIndex));
        resources_[srvIndex] = resource;
    }
}

void DxSrvArray::DestroyView(uint32_t srvIndex) {
    resources_[srvIndex].Reset();
    resources_[srvIndex] = nullptr;
}

void DxSrvArray::Init(uint32_t size, uint32_t arrayLocation) {
    size_               = size;
    arrayStartLocation_ = arrayLocation;
    resources_.resize(size_);
    preCreateIndexs_.reserve(size_);
}

bool DxSrvArray::IsPreCreated(uint32_t index) const {
    if (std::binary_search(preCreateIndexs_.begin(), preCreateIndexs_.end(), index)) {
        return true;
    }

    return false;
}
