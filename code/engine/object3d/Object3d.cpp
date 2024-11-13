#include "object3d/Object3d.h"
#include "ModelManager.h"

#include <cassert>
#include <unordered_map>

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include "directX12/DxFunctionHelper.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "material/TextureManager.h"
#include "System.h"
#include "directX12/DxHeap.h"
#include "material/Material.h"

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <stdint.h>

#include <chrono>
#include "Object3d.h"

BlendMode Object3d::currentBlend_ = BlendMode::None;

#pragma region"Object3d"
Object3d* Object3d::Create(const std::string& directoryPath,const std::string& filename){
	Object3d* newInstance = new Object3d();
	newInstance->data_ = ModelManager::getInstance()->Create(directoryPath,filename);
	return newInstance;
}

void Object3d::PreDraw(){
	ModelManager* manager = ModelManager::getInstance();
	auto* commandList = manager->dxCommand_->getCommandList();

	commandList->SetGraphicsRootSignature(manager->texturePso_[static_cast<uint32_t>(currentBlend_)]->rootSignature.Get());
	commandList->SetPipelineState(manager->texturePso_[static_cast<uint32_t>(currentBlend_)]->pipelineState.Get());

	System::getInstance()->getLightManager()->SetForRootParameter(commandList);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3d::DrawThis(const IConstantBuffer<CameraTransform>& view){
	ModelManager* manager = ModelManager::getInstance();
	auto* commandList = manager->dxCommand_->getCommandList();

	for(auto& model : data_->data_){
		ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
		commandList->SetDescriptorHeaps(1,ppHeaps);
		commandList->SetGraphicsRootDescriptorTable(
			6,
			TextureManager::getDescriptorGpuHandle(model.materialData.textureNumber)
		);

		commandList->IASetVertexBuffers(0,1,&model.meshData.meshBuff->vbView);
		commandList->IASetIndexBuffer(&model.meshData.meshBuff->ibView);

		transform_.SetForRootParameter(commandList,0);
		view.SetForRootParameter(commandList,1);

		model.materialData.material->SetForRootParameter(commandList,2);
		// 描画!!!
		commandList->DrawIndexedInstanced(UINT(model.meshData.indexSize),1,0,0,0);
	}
}

const Model* Object3d::getData() const{ return data_; }

void Object3d::setMaterial(IConstantBuffer<Material>* material,uint32_t index){
	data_->data_[index].materialData.material = material;
}

void Object3d::Draw(const IConstantBuffer<CameraTransform>& view){
	drawFuncTable_[(size_t)data_->currentState_](view);
}
#pragma endregion