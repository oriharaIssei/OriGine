#include "DxSrvArrayManager.h"

/// engine
#include "Engine.h"

// directX12 Object
#include "directX12/DxHeap.h"
#include "directX12/DxSrvArray.h"

/// lib
#include "logger/Logger.h"

DxSrvArrayManager* DxSrvArrayManager::getInstance() {
    static DxSrvArrayManager instance;
    return &instance;
}

void DxSrvArrayManager::Initialize() {
    Logger::Debug("Initialize DxSrvArrayManager \n Size :" + std::to_string(DxHeap::srvHeapSize));
    heapCondition_.push_back({nullptr, DxHeap::srvHeapSize, 0});
}

void DxSrvArrayManager::Finalize() {
    Logger::Debug("Finalize DxSrvArrayManager");

    for (size_t i = 0; i < heapCondition_.size(); i++) {
        if (heapCondition_[i].dxSrvArray_ == nullptr) {
            continue;
        }
        heapCondition_[i].dxSrvArray_.reset();
    }
}

std::shared_ptr<DxSrvArray> DxSrvArrayManager::Create(uint32_t size) {
    Logger::Debug("Create DxSrvArray \n Size   :" + std::to_string(size));

    std::shared_ptr<DxSrvArray> dxSrvArray = std::make_shared<DxSrvArray>();
    uint32_t locate                        = SearchEmptyLocation(size, dxSrvArray);
    dxSrvArray->Initialize(size, locate);

    Logger::Debug("Complete Create DxSrvArray \n Locate :" + std::to_string(locate));

    return dxSrvArray;
}

uint32_t DxSrvArrayManager::SearchEmptyLocation(uint32_t size, std::shared_ptr<DxSrvArray> dxSrvArray) {
    DxHeap* dxHeap = DxHeap::getInstance();
    std::vector<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, uint32_t>> usedArrays_;
    uint32_t currentLocation = 0;

    for (uint32_t i = 0; i < heapCondition_.size(); i++) {
        // 既に使用されているもの (Nullか refが1以下のもの)
        if (heapCondition_[i].dxSrvArray_ != nullptr && heapCondition_[i].dxSrvArray_.use_count() > 1) {
            usedArrays_.push_back({dxHeap->getSrvCpuHandle(i), heapCondition_[i].arraySize});
            currentLocation += heapCondition_[i].arraySize;
            continue;
        }

        // ref が 1 なら 初期化(このインスタンスが持っている 1だから 実質0)
        if (heapCondition_[i].dxSrvArray_.use_count() == 1) {
            heapCondition_[i].dxSrvArray_.reset();
            heapCondition_[i].dxSrvArray_ = nullptr;
        }

        // 空きが 必要なものより小さければ locationを更新して 次へ
        if (heapCondition_[i].arraySize < size) {
            currentLocation += heapCondition_[i].arraySize;
            continue;
        }

        // sizeがぴったりなら そこを使う
        if (heapCondition_[i].arraySize == size) {
            Logger::Debug("Find just Size Space \n Locate :" + std::to_string(currentLocation));

            heapCondition_[i].dxSrvArray_ = dxSrvArray;
        } else {
            // size が違ったら 使う分だけ前詰めする
            std::vector<ArrayCondition>::iterator itr = heapCondition_.begin() + i;
            heapCondition_.insert(itr, {dxSrvArray, size, currentLocation});
            // もともと大きかったものを 後ろに, サイズを使用した分 小さくする
            heapCondition_[i + 1].arrayLocation = currentLocation + size;
            heapCondition_[i + 1].arraySize -= size;
        }
        return heapCondition_[i].arrayLocation;
    }

    // 前詰め処理
    DxHeap::getInstance()->CompactSrvHeap(Engine::getInstance()->getDxDevice()->getDevice(),
        usedArrays_);

    /// 使われていないものを一箇所にまとめる
    uint32_t endLocation = 0;
    uint32_t emptySize   = 0;
    auto it              = heapCondition_.begin();
    while (it != heapCondition_.end()) {
        if (it->dxSrvArray_ != nullptr) {
            // 使用されていない場合
            emptySize += it->arraySize;
            it = heapCondition_.erase(it);
        } else {
            it->dxSrvArray_->arrayStartLocation_ = endLocation;
            it->arrayLocation                    = endLocation;
            endLocation += it->arraySize;
            ++it;
        }
    }

    if (endLocation + size >= DxHeap::srvHeapSize) {
        Logger::Error("SrvHeap Size Over \n Size :" + std::to_string(size) + "\n EndLocation :" + std::to_string(endLocation));
        assert(false);
        return 0;
    }

    heapCondition_.push_back({
        dxSrvArray,
        size,
        endLocation,
    });

    heapCondition_.push_back({nullptr, emptySize, endLocation + size});

    return heapCondition_[heapCondition_.size() - 2].arrayLocation;
}
