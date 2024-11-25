#pragma once

#include <memory>
#include <vector>

#include <stdint.h>

class DxSrvArray;
class DxSrvArrayManager{
public:
	static DxSrvArrayManager *getInstance();
public:
	void Init();
	void Finalize();
	/// @brief 生成
	/// @param size Srv 格納可能数
	/// @return 
	std::shared_ptr<DxSrvArray> Create(uint32_t size);
	/// @brief size分 空いているメモリを検索
	/// @param size 格納したい srv 数
	/// @param dxSrvArray 
	/// @return 
	uint32_t SearchEmptyLocation(uint32_t size,std::shared_ptr<DxSrvArray> dxSrvArray);
private:
	DxSrvArrayManager() = default;
	DxSrvArrayManager(const DxSrvArrayManager &) = delete;
	DxSrvArrayManager *operator=(const DxSrvArrayManager &) = delete;
private:
	struct ArrayCondition{
		std::shared_ptr<DxSrvArray> dxSrvArray_;
		// nullptr用
		uint32_t arraySize;
		uint32_t arrayLocation;
	};
	std::vector<ArrayCondition> heapCondition_;
};