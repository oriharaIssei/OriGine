#include "DxRtvArrayManager.h"

/// engine
#include "Engine.h"

// directX12 object
#include "directX12/DxHeap.h"
#include "directX12/DxRtvArray.h"

/// lib
#include "logger/Logger.h"

DxRtvArrayManager* DxRtvArrayManager::getInstance() {
    static DxRtvArrayManager instance;
    return &instance;
}

void DxRtvArrayManager::Initialize() {
    LOG_DEBUG("Initialize DxSrvArrayManager \n Size :" + std::to_string(DxHeap::rtvHeapSize));
    heapCondition_.push_back({nullptr, DxHeap::rtvHeapSize, 0});
}

void DxRtvArrayManager::Finalize() {
    LOG_DEBUG("Finalize DxRtvArrayManager");

    for (size_t i = 0; i < heapCondition_.size(); i++) {
        if (heapCondition_[i].dxRtvArray_ == nullptr) {
            continue;
        }
        heapCondition_[i].dxRtvArray_.reset();
    }
}

std::shared_ptr<DxRtvArray> DxRtvArrayManager::Create(uint32_t size) {
    LOG_DEBUG("Create DxRtvArray \n Size   :" + std::to_string(size));

    std::shared_ptr<DxRtvArray> dxRtvArray = std::make_shared<DxRtvArray>();
    uint32_t locate                        = SearchEmptyLocation(size, dxRtvArray);
    dxRtvArray->Initialize(size, locate);

    LOG_DEBUG("Complete Create DxRtvArray \n Locate :" + std::to_string(locate));

    return dxRtvArray;
}

uint32_t DxRtvArrayManager::SearchEmptyLocation(uint32_t size, std::shared_ptr<DxRtvArray> dxRtvArray) {
    DxHeap* dxHeap = DxHeap::getInstance();
    std::vector<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, uint32_t>> usedArrays_;
    uint32_t currentLocation = 0;

    for (uint32_t i = 0; i < heapCondition_.size(); i++) {
        // 既に使用されているもの (Nullか refが1以下のもの)
        if (heapCondition_[i].dxRtvArray_ != nullptr) {
            usedArrays_.push_back({dxHeap->getDsvCpuHandle(i), heapCondition_[i].arraySize});
            currentLocation += heapCondition_[i].arraySize;
            continue;
        }

        // ref が 1 なら 初期化(このインスタンスが持っている 1だから 実質0)
        if (heapCondition_[i].dxRtvArray_.use_count() == 1) {
            heapCondition_[i].dxRtvArray_.reset();
            heapCondition_[i].dxRtvArray_ = nullptr;
        }
        // 空きが 必要なものより小さければ locationを更新して 次へ
        if (heapCondition_[i].arraySize < size) {
            currentLocation += heapCondition_[i].arraySize;
            continue;
        }

        // sizeがぴったりなら そこを使う
        if (heapCondition_[i].arraySize == size) {
            LOG_DEBUG("Find just Size Space \n Locate :" + std::to_string(currentLocation));

            heapCondition_[i].dxRtvArray_ = dxRtvArray;
        } else {
            // size が違ったら 使う分だけ前詰めする
            std::vector<ArrayCondition>::iterator itr = heapCondition_.begin() + i;
            heapCondition_.insert(itr, {dxRtvArray, size, currentLocation});

            heapCondition_[i + 1].arrayLocation = currentLocation + size;
            heapCondition_[i + 1].arraySize -= size;
        }
        return heapCondition_[i].arrayLocation;
    }

    // 前詰め処理
    DxHeap::getInstance()->CompactRtvHeap(Engine::getInstance()->getDxDevice()->getDevice(),
        usedArrays_);

    /// 使われていないものを一箇所にまとめる
    uint32_t endLocation = 0;
    uint32_t emptySize   = 0;
    auto it              = heapCondition_.begin();
    while (it != heapCondition_.end()) {
        if (it->dxRtvArray_ != nullptr) {
            // 使用されていない場合
            emptySize += it->arraySize;
            it = heapCondition_.erase(it);
        } else {
            it->dxRtvArray_->arrayStartLocation_ = endLocation;
            it->arrayLocation                    = endLocation;
            endLocation += it->arraySize;
            ++it;
        }
    }

    if (endLocation + size >= DxHeap::rtvHeapSize) {
        LOG_ERROR("RtvHeap Size Over \n Size :" + std::to_string(size) + "\n EndLocation :" + std::to_string(endLocation));
        assert(false);
        return 0;
    }

    heapCondition_.push_back({
        dxRtvArray,
        size,
        endLocation,
    });

    heapCondition_.push_back({nullptr, emptySize, endLocation + size});

    return heapCondition_[heapCondition_.size() - 2].arrayLocation;
}
