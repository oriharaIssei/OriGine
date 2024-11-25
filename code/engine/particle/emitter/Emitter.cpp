#include "../manager/ParticleManager.h"
#include "Emitter.h"

#include "material/TextureManager.h"
#include "model/Model.h"
#include "model/ModelManager.h"
#include "particle/Particle.h"

#include "globalVariables/GlobalVariables.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG


void Emitter::Init(const std::string& emitterName){
	emitterName_ = emitterName;

}

void Emitter::Update(float deltaTime){
	for(auto& particle : particles_){
		particle->Update(deltaTime);
	}
	std::erase_if(particles_,[](const std::unique_ptr<Particle>& particle){return !particle->getIsAlive(); });
	currentCoolTime_ -= deltaTime;
	if(currentCoolTime_ <= 0.0f){
		currentCoolTime_ = particleLifeTime_;
		SpawnParticle();
	}
}

#ifdef _DEBUG
void Emitter::Debug(){
	if(!ImGui::Begin(emitterName_.c_str())){
		return;
	}

	ImGui::DragInt("spawnParticleVal_",&spawnParticleVal_,1,0);
	ImGui::DragInt("particleMaxSize_",&particleMaxSize_,1,0);
	ImGui::DragFloat("particleLifeTime_",&particleLifeTime_,0.1f,0);

	ImGui::End();
}
#endif // _DEBUG

void Emitter::Draw(const IConstantBuffer<CameraTransform>& camera){
	auto* commandList = ParticleManager::getInstance()->dxCommand_->getCommandList();

	for(auto& model : particleModel_->data_){
		ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
		commandList->SetDescriptorHeaps(1,ppHeaps);
		commandList->SetGraphicsRootDescriptorTable(
			3,
			TextureManager::getDescriptorGpuHandle(model.materialData.textureNumber)
		);

		structuredTransform_.SetForRootParameter(commandList,0);
		camera.SetForRootParameter(commandList,1);

		model.materialData.material->SetForRootParameter(commandList,2);
		// 描画!!!
		commandList->DrawIndexedInstanced(UINT(model.meshData.indexSize),static_cast<UINT>(structuredTransform_.openData_.size()),0,0,0);
	}
}

void Emitter::SpawnParticle(){
	int32_t canSpawnParticleValue_ = particleMaxSize_;
	for(auto& particle : particles_){
		if(particle->getIsAlive()){
			continue;
		}
		--canSpawnParticleValue_;
		particle->Spawn(particleInitialTransform_,particleLifeTime_);
	}
}