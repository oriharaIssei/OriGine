#include "IConstantBuffer.h"

void IConstantBuffer::SetForRootParameter(ID3D12GraphicsCommandList* cmdList,uint32_t rootParameterNum) const
{
	cmdList->SetGraphicsRootConstantBufferView(rootParameterNum,buff_.getResource()->GetGPUVirtualAddress());
}
