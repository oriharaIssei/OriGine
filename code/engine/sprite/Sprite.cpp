#include "sprite/Sprite.h"

#include "directX12/dxFunctionHelper/DxFunctionHelper.h"
#include "System.h"
#include "texture/TextureManager.h"
#include "logger/Logger.h"
#include <directX12/ShaderCompiler.h>

void SpriteMesh::Init()
{
	const int32_t vertexSize = 4;
	const int32_t indexSize = 6;
	UINT vertDataSize = sizeof(SpriteVertexData);

	vertBuff.CreateBufferResource(System::getInstance()->getDxDevice(),vertDataSize * vertexSize);
	vbView.BufferLocation = vertBuff.getResource()->GetGPUVirtualAddress();
	vbView.SizeInBytes = vertDataSize * vertexSize;
	vbView.StrideInBytes = vertDataSize;
	vertBuff.getResource()->Map(0,nullptr,reinterpret_cast<void**>(&vertexData));

	indexBuff.CreateBufferResource(System::getInstance()->getDxDevice(),sizeof(uint32_t) * indexSize);
	ibView.BufferLocation = indexBuff.getResource()->GetGPUVirtualAddress();
	ibView.SizeInBytes = sizeof(uint32_t) * indexSize;
	ibView.Format = DXGI_FORMAT_R32_UINT;
	indexBuff.getResource()->Map(0,nullptr,reinterpret_cast<void**>(&indexData));
}


void Sprite::Init(const std::string& filePath)
{
	this->textureIndex_ = TextureManager::LoadTexture(filePath);

	this->meshBuff_ = std::make_unique<SpriteMesh>();
	this->meshBuff_->Init();

	this->meshBuff_->vertexData[0] = {{0.0f,1.0f,0.0f,1.0f},{0.0f,1.0f}};
	this->meshBuff_->vertexData[1] = {{0.0f,0.0f,0.0f,1.0f},{0.0f,0.0f}};
	this->meshBuff_->vertexData[2] = {{1.0f,1.0f,0.0f,1.0f},{1.0f,1.0f}};
	this->meshBuff_->vertexData[3] = {{1.0f,0.0f,0.0f,1.0f},{1.0f,0.0f}};

	this->meshBuff_->indexData[0] = 0;
	this->meshBuff_->indexData[1] = 1;
	this->meshBuff_->indexData[2] = 2;
	this->meshBuff_->indexData[3] = 1;
	this->meshBuff_->indexData[4] = 3;
	this->meshBuff_->indexData[5] = 2;

	this->mappingConstBufferData_ = nullptr;
	this->constBuff_.CreateBufferResource(System::getInstance()->getDxDevice(),sizeof(SpritConstBuffer));

	this->constBuff_.getResource()->Map(
		0,nullptr,reinterpret_cast<void**>(&this->mappingConstBufferData_)
	);

	this->mappingConstBufferData_->color_ = {1.0f,1.0f,1.0f,1.0f};
}

void Sprite::Draw()
{
	auto commandList = spriteCommon_->dxCommand_->getCommandList();

	mappingConstBufferData_->mat_ = worldMat_ * spriteCommon_->viewPortMat_;
	mappingConstBufferData_->uvMat_ = MakeMatrix::Affine(uvScale_,uvRotate_,uvTranslate_);

	commandList->SetGraphicsRootConstantBufferView(
		0,constBuff_.getResource()->GetGPUVirtualAddress()
	);

	commandList->IASetVertexBuffers(0,1,&meshBuff_->vbView);
	commandList->IASetIndexBuffer(&meshBuff_->ibView);

	ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
	commandList->SetDescriptorHeaps(1,ppHeaps);
	commandList->SetGraphicsRootDescriptorTable(
		1,
		TextureManager::getDescriptorGpuHandle(textureIndex_)
	);
	commandList->SetGraphicsRootConstantBufferView(0,constBuff_.getResource()->GetGPUVirtualAddress());

	commandList->DrawIndexedInstanced(
		6,1,0,0,0
	);
}

void Sprite::UpdateMatrix()
{
}
