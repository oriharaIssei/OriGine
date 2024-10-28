#include "sprite/Sprite.h"

#include "directX12/dxFunctionHelper/DxFunctionHelper.h"
#include "System.h"
#include "texture/TextureManager.h"
#include "logger/Logger.h"
#include <directX12/ShaderCompiler.h>

#include "imgui/imgui.h"

void SpriteMesh::Init(){
	const int32_t vertexSize = 4;
	const int32_t indexSize = 6;
	UINT vertDataSize = sizeof(SpriteVertexData);

	vertBuff.CreateBufferResource(System::getInstance()->getDxDevice()->getDevice(),vertDataSize * vertexSize);
	vbView.BufferLocation = vertBuff.getResource()->GetGPUVirtualAddress();
	vbView.SizeInBytes = vertDataSize * vertexSize;
	vbView.StrideInBytes = vertDataSize;
	vertBuff.getResource()->Map(0,nullptr,reinterpret_cast<void**>(&vertexData));

	indexBuff.CreateBufferResource(System::getInstance()->getDxDevice()->getDevice(),sizeof(uint32_t) * indexSize);
	ibView.BufferLocation = indexBuff.getResource()->GetGPUVirtualAddress();
	ibView.SizeInBytes = sizeof(uint32_t) * indexSize;
	ibView.Format = DXGI_FORMAT_R32_UINT;
	indexBuff.getResource()->Map(0,nullptr,reinterpret_cast<void**>(&indexData));
}


void Sprite::Init(const std::string& filePath){
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
	this->constBuff_.CreateBufferResource(System::getInstance()->getDxDevice()->getDevice(),sizeof(SpritConstBuffer));

	this->constBuff_.getResource()->Map(
		0,nullptr,reinterpret_cast<void**>(&this->mappingConstBufferData_)
	);

	const DirectX::TexMetadata& texData = TextureManager::getTexMetadata(textureIndex_);
	textureSize_ = {static_cast<float>(texData.width),static_cast<float>(texData.height)};
	size_ = textureSize_;
}

void Sprite::Draw(){
	Update();
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

void Sprite::Debug(const std::string& name){
	ImGui::ColorEdit4("Color",&color_.x);

	ImGui::DragFloat2("UVScale",&uvScale_.x,0.1f);
	ImGui::DragFloat2("UVRotate",&uvRotate_.x,0.1f);
	ImGui::DragFloat2("UVTranslate",&uvTranslate_.x,0.1f);

	ImGui::DragFloat2("Position",&pos_.x,0.1f);
	ImGui::DragFloat2("Size",&size_.x,0.1f);

	ImGui::DragFloat2("textureLeftTopPos",&textureLeftTop_.x,0.1f);
	ImGui::DragFloat2("textureSize",&textureSize_.x,0.1f);
}

void Sprite::Update(){
	worldMat_ = MakeMatrix::Affine({size_,1.0f},{0.0f,0.0f,rotate_},{pos_,0.0f});
	uvMat_ = MakeMatrix::Affine(uvScale_,uvRotate_,uvTranslate_);

	float left = -anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = -anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;

	if(isFlipX_){
		left = -left;
		right =  -right;
	}
	if(isFlipY_){
		top = -top;
		bottom = -bottom;
	}

	meshBuff_->vertexData[0].pos = {left,bottom,0.0f,1.0f};
	meshBuff_->vertexData[1].pos = {left,top,0.0f,1.0f};
	meshBuff_->vertexData[2].pos = {right,bottom,0.0f,1.0f};
	meshBuff_->vertexData[3].pos = {right,top,0.0f,1.0f};

	const DirectX::TexMetadata& texData = TextureManager::getTexMetadata(textureIndex_);
	float texLeft = textureLeftTop_.x / static_cast<float>(texData.width);
	float texRight = (textureLeftTop_.x + textureSize_.x) / static_cast<float>(texData.width);
	float texTop = textureLeftTop_.y / static_cast<float>(texData.height);
	float texBottom = (textureLeftTop_.y + textureSize_.y) / static_cast<float>(texData.height);

	meshBuff_->vertexData[0].texcoord = {texLeft,texBottom};
	meshBuff_->vertexData[1].texcoord = {texLeft,texTop};
	meshBuff_->vertexData[2].texcoord = {texRight,texBottom};
	meshBuff_->vertexData[3].texcoord = {texRight,texTop};
	ConvertMappingData();
}

void Sprite::ConvertMappingData(){
	mappingConstBufferData_->mat_ = worldMat_;
	mappingConstBufferData_->uvMat_ = uvMat_;
	mappingConstBufferData_->color_  = color_;
}
