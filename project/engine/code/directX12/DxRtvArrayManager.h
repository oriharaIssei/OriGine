#pragma once

#include <memory>
#include <vector>

#include <stdint.h>

class DxRtvArray;
class DxRtvArrayManager{
public:
	static DxRtvArrayManager *getInstance();
public:
	void Initialize();
	void Finalize();
	/// @brief 生成
	/// @param size Srv 格納可能数
	/// @return 
	std::shared_ptr<DxRtvArray> Create(uint32_t size);
	/// @brief size分 空いているメモリを検索
	/// @param size 格納したい view 数
	/// @param dxSrvArray 
	/// @return 
	uint32_t SearchEmptyLocation(uint32_t size,std::shared_ptr<DxRtvArray>dxRtvArray);
private:
	DxRtvArrayManager() = default;
	DxRtvArrayManager(const DxRtvArrayManager &) = delete;
	DxRtvArrayManager *operator=(const DxRtvArrayManager &) = delete;
private:
	struct ArrayCondition{
		std::shared_ptr<DxRtvArray> dxRtvArray_;
		// nullptr用
		uint32_t arraySize;
		uint32_t arrayLocation;
	};
	std::vector<ArrayCondition> heapCondition_;
};