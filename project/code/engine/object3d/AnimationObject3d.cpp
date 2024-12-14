#include "AnimationObject3d.h"

#include "material/Material.h"

#include "material/texture/TextureManager.h"
#include "model/ModelManager.h"

std::unique_ptr<AnimationObject3d> AnimationObject3d::Create(const std::string& _directoryPath,
															 const std::string& _filename){
	std::unique_ptr<AnimationObject3d> newInstance = std::make_unique<AnimationObject3d>();
	newInstance->data_ = ModelManager::getInstance()->Create(_directoryPath,_filename);
	newInstance->animation_= std::make_unique<Animation>(ModelManager::LoadAnimation(_directoryPath,_filename));
	return newInstance;
}

std::unique_ptr<AnimationObject3d> AnimationObject3d::Create(const std::string& _modelDirectoryPath,
															 const std::string& _modelFilename,
															 const std::string& _animationDirectoryPath,
															 const std::string& _animationFilename){
	std::unique_ptr<AnimationObject3d> newInstance = std::make_unique<AnimationObject3d>();
	// model 
	newInstance->data_ = ModelManager::getInstance()->Create(_modelDirectoryPath,_modelFilename);
	// animation
	newInstance->animation_= std::make_unique<Animation>(ModelManager::LoadAnimation(_animationDirectoryPath,_animationFilename));
	return newInstance;
}

void AnimationObject3d::Update(float deltaTime){
	animation_->UpdateTime(deltaTime);

	// Animationより 先に Object 座標系の 行進
	transform_.UpdateMatrix();

	// ノードごとのトランスフォームを計算
	std::map<std::string,Matrix4x4> nodeTransforms;
	ApplyAnimationToNodes(data_->meshData_->rootNode,transform_.worldMat,*animation_,nodeTransforms);

	// 計算結果をメッシュに適用
	for(auto& mesh : data_->meshData_->mesh_){
		auto it = nodeTransforms.find(mesh.nodeName);
		if(it != nodeTransforms.end()){
			mesh.transform_.openData_.worldMat = it->second;  // ワールド行列
			mesh.transform_.ConvertToBuffer();
		}
	}
}

void AnimationObject3d::Draw(){
	drawFuncTable_[(int)data_->currentState_]();
}

void AnimationObject3d::DrawThis(){
	ModelManager* manager = ModelManager::getInstance();
	auto* commandList = manager->dxCommand_->getCommandList();

	uint32_t index = 0;

	for(auto& mesh : data_->meshData_->mesh_){
		auto& material = data_->materialData_[index];
		ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
		commandList->SetDescriptorHeaps(1,ppHeaps);
		commandList->SetGraphicsRootDescriptorTable(
			7,
			TextureManager::getDescriptorGpuHandle(material.textureNumber)
		);

		commandList->IASetVertexBuffers(0,1,&mesh.meshBuff->vbView);
		commandList->IASetIndexBuffer(&mesh.meshBuff->ibView);

		mesh.transform_.SetForRootParameter(commandList,0);

		material.material->SetForRootParameter(commandList,2);
		// 描画!!!
		commandList->DrawIndexedInstanced(UINT(mesh.indexSize),1,0,0,0);

		++index;
	}
}

void AnimationObject3d::setMaterial(IConstantBuffer<Material>* material,uint32_t index){
	data_->materialData_[index].material = material;
}