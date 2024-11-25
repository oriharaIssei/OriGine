#include "DxRtvArrayManager.h"

#include "System.h"

#include "directX12/DxHeap.h"
#include "directX12/DxRtvArray.h"

DxRtvArrayManager *DxRtvArrayManager::getInstance(){
	static DxRtvArrayManager instance;
	return &instance;
}

void DxRtvArrayManager::Init(){
	heapCondition_.push_back({nullptr,DxHeap::dsvHeapSize,0});
}

void DxRtvArrayManager::Finalize(){
	for(size_t i = 0; i < heapCondition_.size(); i++){
		if(heapCondition_[i].dxRtvArray_ == nullptr){
			continue;
		}
		heapCondition_[i].dxRtvArray_.reset();
	}
}

std::shared_ptr<DxRtvArray> DxRtvArrayManager::Create(uint32_t size){
	std::shared_ptr<DxRtvArray> dxRtvArray = std::make_shared<DxRtvArray>();
	uint32_t locate = SearchEmptyLocation(size,dxRtvArray);
	dxRtvArray->Init(size,locate);
	heapCondition_.push_back({dxRtvArray,size,locate});
	return dxRtvArray;
}

uint32_t DxRtvArrayManager::SearchEmptyLocation(uint32_t size,std::shared_ptr<DxRtvArray> dxRtvArray){
	DxHeap *dxHeap = DxHeap::getInstance();
	std::vector<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE,uint32_t>> usedArrays_;
	uint32_t currentLocation = 0;

	for(uint32_t i = 0; i < heapCondition_.size(); i++){
		if(heapCondition_[i].dxRtvArray_ != nullptr){
			usedArrays_.push_back({dxHeap->getDsvCpuHandle(i),heapCondition_[i].arraySize});
			currentLocation += heapCondition_[i].arraySize;
			continue;
		}
		if(heapCondition_[i].arraySize < size){
			currentLocation += heapCondition_[i].arraySize;
			continue;
		}

		if(static_cast<int32_t>(heapCondition_[i].arraySize -= size) == 0){
			// sizeがぴったりなら そこを使う
			heapCondition_[i] = {dxRtvArray,size,i};
		} else{
			// size が違ったら 使う分だけ前詰めする
			std::vector<ArrayCondition>::iterator itr = heapCondition_.begin() + i;
			heapCondition_.insert(itr,{dxRtvArray,size,currentLocation});
			heapCondition_[i + 1].arrayLocation = currentLocation + size;
		}
		return heapCondition_[i].arrayLocation;
	}

	// 前詰め処理
	DxHeap::getInstance()->CompactDsvHeap(System::getInstance()->getDxDevice()->getDevice(),
										  usedArrays_);

	///使われていないものを一箇所にまとめる
	uint32_t endLocation = 0;
	uint32_t emptySize = 0;
	auto it = heapCondition_.begin();
	while(it != heapCondition_.end()){
		if(it->dxRtvArray_ != nullptr){
			// 使用されていない場合
			emptySize += it->arraySize;
			it = heapCondition_.erase(it);
		} else{
			it->dxRtvArray_->arrayStartLocation_ = endLocation;
			it->arrayLocation = endLocation;
			endLocation += it->arraySize;
			++it;
		}
	}

	if(endLocation + size >= DxHeap::srvHeapSize){
		assert(false);
		return 0;
	}

	heapCondition_.push_back({dxRtvArray,size,endLocation,});

	heapCondition_.push_back({nullptr,emptySize,endLocation + size});

	return heapCondition_[heapCondition_.size() - 2].arrayLocation;
}