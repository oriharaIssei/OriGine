#include "model/Model.h"
#include "model/ModelManager.h"

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

#pragma region"Model"
Model* Model::Create(const std::string& directoryPath,const std::string& filename){
	return ModelManager::getInstance()->Create(directoryPath,filename);
}

void Model::DrawThis(const IConstantBuffer<CameraTransform>& view,BlendMode blend){
	ModelManager* manager = ModelManager::getInstance();
	auto* commandList = manager->dxCommand_->getCommandList();

	for(auto& model : data_){

		commandList->SetGraphicsRootSignature(manager->texturePso_[static_cast<uint32_t>(blend)]->rootSignature.Get());
		commandList->SetPipelineState(manager->texturePso_[static_cast<uint32_t>(blend)]->pipelineState.Get());

		ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
		commandList->SetDescriptorHeaps(1,ppHeaps);
		commandList->SetGraphicsRootDescriptorTable(
			6,
			TextureManager::getDescriptorGpuHandle(model->materialData.textureNumber)
		);

		commandList->IASetVertexBuffers(0,1,&model->meshData.meshBuff->vbView);
		commandList->IASetIndexBuffer(&model->meshData.meshBuff->ibView);

		transform_.SetForRootParameter(commandList,0);
		view.SetForRootParameter(commandList,1);

		model->materialData.material->SetForRootParameter(commandList,2);
		System::getInstance()->getDirectionalLight()->SetForRootParameter(commandList,3);
		System::getInstance()->getPointLight()->SetForRootParameter(commandList,4);
		System::getInstance()->getSpotLight()->SetForRootParameter(commandList,5);

		// 描画!!!
		commandList->DrawIndexedInstanced(UINT(model->meshData.indexSize),1,0,0,0);
	}
}

void Model::Draw(const IConstantBuffer<CameraTransform>& view,BlendMode blend){
	drawFuncTable_[(size_t)currentState_](view,blend);
}
#pragma endregion