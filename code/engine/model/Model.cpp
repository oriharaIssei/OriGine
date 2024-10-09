#include "model/Model.h"
#include "model/ModelManager.h"

#include <cassert>
#include <unordered_map>

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include "directX12/dxFunctionHelper/DxFunctionHelper.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "texture/TextureManager.h"
#include "System.h"
#include "directX12/dxHeap/DxHeap.h"

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <stdint.h>

#include <chrono>

std::unique_ptr<Matrix4x4> Model::fovMa_ = nullptr;
std::unique_ptr<ModelManager> Model::manager_ = nullptr;

std::array<PipelineStateObj *,kBlendNum> Model::primitivePso_;
std::array<PipelineStateObj *,kBlendNum> Model::texturePso_;
std::unique_ptr<DxCommand> Model::dxCommand_;

struct VertexKey{
	Vector4 position;
	Vector3 normal;
	Vector2 texCoord;

	bool operator==(const VertexKey &other) const{
		return position == other.position &&
			normal == other.normal &&
			texCoord == other.texCoord;
	}
};

namespace std{
	template<>
	struct hash<VertexKey>{
		size_t operator()(const VertexKey &key) const{
			return hash<float>()(key.position.x) ^ hash<float>()(key.position.y) ^ hash<float>()(key.position.z) ^
				hash<float>()(key.normal.x) ^ hash<float>()(key.normal.y) ^ hash<float>()(key.normal.z) ^
				hash<float>()(key.texCoord.x) ^ hash<float>()(key.texCoord.y);
		}
	};
}

#pragma region"Model"
Model* Model::Create(const std::string &directoryPath,const std::string &filename){
	return manager_->Create(directoryPath,filename);
}

void Model::Init(){
	manager_ = std::make_unique<ModelManager>();
	manager_->Init();

	fovMa_ = std::make_unique<Matrix4x4>();
	Matrix4x4 *maPtr = new Matrix4x4();
	*maPtr = MakeMatrix::PerspectiveFov(
		0.45f,
		static_cast<float>(System::getInstance()->getWinApp()->getWidth())
		/
		static_cast<float>(System::getInstance()->getWinApp()->getHeight()),
		0.1f,
		100.0f);
	fovMa_.reset(
		maPtr
	);

	dxCommand_ = std::make_unique<DxCommand>();
	dxCommand_->Init(System::getInstance()->getDxDevice()->getDevice(),"main","main");

	size_t index = 0;
	for(auto &primShaderKey : PrimitiveDrawer::getTrianglePsoKeys()){
		primitivePso_[index] = ShaderManager::getInstance()->getPipelineStateObj(primShaderKey);
		index++;
	}

	index = 0;
	for(auto &texShaderKey : System::getInstance()->getTexturePsoKeys()){
		texturePso_[index] = ShaderManager::getInstance()->getPipelineStateObj(texShaderKey);
		index++;
	}
}

void Model::Finalize(){
	manager_->Finalize();
	dxCommand_->Finalize();
}

void Model::DrawThis(const TransformBuffer &world,const ViewProjection &view,BlendMode blend){
	auto *commandList = dxCommand_->getCommandList();

	for(auto &model : data_){

		commandList->SetGraphicsRootSignature(texturePso_[static_cast<uint32_t>(blend)]->rootSignature.Get());
		commandList->SetPipelineState(texturePso_[static_cast<uint32_t>(blend)]->pipelineState.Get());
		
		ID3D12DescriptorHeap *ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
		commandList->SetDescriptorHeaps(1,ppHeaps);
		commandList->SetGraphicsRootDescriptorTable(
			6,
			TextureManager::getDescriptorGpuHandle(model->materialData.textureNumber)
		);

		commandList->IASetVertexBuffers(0,1,&model->meshData.meshBuff->vbView);
		commandList->IASetIndexBuffer(&model->meshData.meshBuff->ibView);

		world.SetForRootParameter(commandList,0);
		view.SetForRootParameter(commandList,1);

		model->materialData.material->SetForRootParameter(commandList,2);
		System::getInstance()->getDirectionalLight()->SetForRootParameter(commandList,3);
		System::getInstance()->getPointLight()->SetForRootParameter(commandList,4);
		System::getInstance()->getSpotLight()->SetForRootParameter(commandList,5);

		// 描画!!!
		commandList->DrawIndexedInstanced(UINT(model->meshData.indexSize),1,0,0,0);
	}
}

void Model::Draw(const TransformBuffer &world,const ViewProjection &view,BlendMode blend){
	drawFuncTable_[(size_t)currentState_](world,view,blend);
}
#pragma endregion