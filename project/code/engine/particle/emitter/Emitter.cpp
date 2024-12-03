#include "../manager/ParticleManager.h"
#include "Emitter.h"

#include <array>

#include "globalVariables/GlobalVariables.h"
#include "material/texture/TextureManager.h"

#include "EmitterShape.h"
#include "model/Model.h"
#include "model/ModelManager.h"
#include "myRandom/MyRandom.h"
#include "particle/Particle.h"

#include <cmath>

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

Emitter::Emitter(){}

Emitter::~Emitter(){}

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
void Emitter::Debug(bool* isOpenedWindow){
	if(!isOpenedWindow){
		return;
	}
	ImGui::Begin(emitterName_.c_str(),isOpenedWindow);

	ImGui::DragInt("spawnParticleVal_",&spawnParticleVal_,1,0);
	ImGui::DragInt("particleMaxSize_",&particleMaxSize_,1,0);
	ImGui::DragFloat("particleLifeTime_",&particleLifeTime_,0.1f,0);

	if(ImGui::BeginCombo("EmitterShapeType",emitterShapeTypeWord_[shapeType_].c_str())){
		for(int32_t i = 0; i < shapeTypeCount; i++){
			bool isSelected = (shapeType_ == i); // 現在選択中かどうか

			if(ImGui::Selectable(emitterShapeTypeWord_[i].c_str(),isSelected)){
				switch(EmitterShapeType(i)){
					case EmitterShapeType::SPHERE:
						emitterSpawnShape_ = std::make_unique<EmitterSphere>();
						break;
					case EmitterShapeType::AABB:
						emitterSpawnShape_ = std::make_unique<EmitterAABB>();
						break;
					default:
						break;
				}
				shapeType_ = i;
			}

			// 現在選択中の項目をハイライトする
			if(isSelected){
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	if(ImGui::TreeNode("EmitterShape")){
		if(emitterSpawnShape_){
			emitterSpawnShape_->Debug();
		}
		ImGui::TreePop();
	}

	ImGui::End();
}
#endif // _DEBUG

void Emitter::Draw(const IConstantBuffer<CameraTransform>& camera){
	if(!particleModel_){
		return;
	}

	auto* commandList = ParticleManager::getInstance()->dxCommand_->getCommandList();

	uint32_t index = 0;
	for(auto& model : particleModel_->meshData_->mesh_){
		auto& material = particleModel_->materialData_[index];
		ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
		commandList->SetDescriptorHeaps(1,ppHeaps);
		commandList->SetGraphicsRootDescriptorTable(
			3,
			TextureManager::getDescriptorGpuHandle(material.textureNumber)
		);

		structuredTransform_.SetForRootParameter(commandList,0);
		camera.SetForRootParameter(commandList,1);

		material.material->SetForRootParameter(commandList,2);
		// 描画!!!
		commandList->DrawIndexedInstanced(UINT(model.indexSize),static_cast<UINT>(structuredTransform_.openData_.size()),0,0,0);

		++index;
	}
}

void Emitter::SpawnParticle(){
	int32_t canSpawnParticleValue_ = particleMaxSize_;
	for(auto& particle : particles_){
		if(particle->getIsAlive()){
			continue;
		}
		--canSpawnParticleValue_;
		// 指定された 形状内の ランダムな位置 を 指定
		particleInitialTransform_.translate = emitterSpawnShape_->getSpawnPos();

		particle->Spawn(particleInitialTransform_,particleLifeTime_);
	}
}