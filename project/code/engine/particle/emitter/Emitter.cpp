#include "../manager/ParticleManager.h"
#include "Emitter.h"

#include <array>

#include "Engine.h"
#include "globalVariables/GlobalVariables.h"
#include "material/texture/TextureManager.h"
#include "myFileSystem/MyFileSystem.h"

#include "EmitterShape.h"
#include "model/Model.h"
#include "model/ModelManager.h"
#include "myRandom/MyRandom.h"
#include "particle/Particle.h"

#include <cmath>

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

Emitter::Emitter(DxSrvArray* srvArray):srvArray_(srvArray){}

Emitter::~Emitter(){}

static std::list<std::pair<std::string,std::string>> objectFiles = MyFileSystem::SearchFile("resource/Models","obj",false);

void Emitter::Init(const std::string& emitterName){
	emitterName_ = emitterName;

	structuredTransform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice(),
									  srvArray_,
									  particleMaxSize_);
	particles_.reserve(particleMaxSize_);

	switch(EmitterShapeType(shapeType_)){
		case EmitterShapeType::SPHERE:
			emitterSpawnShape_ = std::make_unique<EmitterSphere>();
			break;
		case EmitterShapeType::AABB:
			emitterSpawnShape_ = std::make_unique<EmitterAABB>();
			break;
		default:
			break;
	}
}

void Emitter::Update(float deltaTime){
	{
		for(auto& particle : particles_){
			particle->Update(deltaTime);
		}
		// isAliveでないもの は 消す
		std::erase_if(particles_,[](std::unique_ptr<Particle>& particle){return !particle->getIsAlive(); });
	}

	std::erase_if(particles_,[](const std::unique_ptr<Particle>& particle){return !particle->getIsAlive(); });
	
	currentCoolTime_ -= deltaTime;
	if(currentCoolTime_ <= 0.0f){
		currentCoolTime_ = spawnCoolTime_;
		SpawnParticle();
	}

	structuredTransform_.openData_.clear();
	for(auto& particle : particles_){
		structuredTransform_.openData_.push_back(particle->getTransform());
	}
	structuredTransform_.ConvertToBuffer();
}

#ifdef _DEBUG
void Emitter::Debug(bool* isOpenedWindow){
	if(!isOpenedWindow){
		return;
	}
	ImGui::Begin(emitterName_.c_str(),isOpenedWindow);

	float deltaTime = Engine::getInstance()->getDeltaTime();
	ImGui::InputFloat("DeltaTime",&deltaTime,0.1f,1.0f,"%.3f",ImGuiInputTextFlags_ReadOnly);

	ImGui::Text("SpawnParticleVal");
	ImGui::DragInt("##spawnParticleVal",&spawnParticleVal_,1,0);
	ImGui::Text("ParticleMaxSize");
	if(ImGui::InputInt("##particleMaxSize",&particleMaxSize_,1,5,ImGuiInputTextFlags_EnterReturnsTrue)){
		structuredTransform_.Resize(Engine::getInstance()->getDxDevice()->getDevice(),particleMaxSize_);
	}

	ImGui::Text("SpawnCoolTime");
	ImGui::DragFloat("##SpawnCoolTime",&spawnCoolTime_,0.1f,0);
	ImGui::Text("ParticleLifeTime");
	ImGui::DragFloat("##ParticleLifeTime",&particleLifeTime_,0.1f,0);

	ImGui::Spacing();
	{
		ImGui::Text("Particle Color");
		ImGui::ColorEdit4("##Particle Color",&particleInitialTransform_.color.x);
		ImGui::Text("Particle Scale");
		ImGui::DragFloat3("##Particle Scale",&particleInitialTransform_.scale.x,0.1f);
		ImGui::Text("Particle Rotate");
		ImGui::DragFloat3("##Particle Rotate",&particleInitialTransform_.rotate.x,0.1f);
		ImGui::Text("Particle Translate");
		ImGui::DragFloat3("##Particle Translate",&particleInitialTransform_.translate.x,0.1f);
	}
	ImGui::Spacing();

	if(ImGui::Button("reload FileList")){
		objectFiles = MyFileSystem::SearchFile("resource/Models","obj",false);
	}

	if(ImGui::BeginCombo("ParticleModel",currentModelFileName_.c_str())){
		for(auto& fileName : objectFiles){
			bool isSelected = (fileName.second == currentModelFileName_); // 現在選択中かどうか
			if(ImGui::Selectable(fileName.second.c_str(),isSelected)){
				particleModel_ = ModelManager::getInstance()->Create(fileName.first,fileName.second);
				currentModelFileName_ = fileName.second;
			}
		}
		ImGui::EndCombo();
	}

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
	if(!particleModel_ ||
	   particleModel_->currentState_ != Model::LoadState::Loaded){
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

		commandList->IASetVertexBuffers(0,1,&model.meshBuff->vbView);
		commandList->IASetIndexBuffer(&model.meshBuff->ibView);

		structuredTransform_.SetForRootParameter(commandList,0);
		camera.SetForRootParameter(commandList,1);

		material.material->SetForRootParameter(commandList,2);
		// 描画!!!
		commandList->DrawIndexedInstanced(UINT(model.indexSize),static_cast<UINT>(structuredTransform_.openData_.size()),0,0,0);

		++index;
	}
}

void Emitter::SpawnParticle(){
	// スポーンして良い数 
	int32_t canSpawnParticleValue_ = (std::min)(spawnParticleVal_,static_cast<int32_t>(particleMaxSize_ - particles_.size()));

	for(int32_t i = 0; i < canSpawnParticleValue_; i++){
		//割りたてる Transform の 初期化
		particleInitialTransform_.translate = emitterSpawnShape_->getSpawnPos();
		structuredTransform_.openData_.push_back({});
		structuredTransform_.openData_.back() = particleInitialTransform_;

		// Particle 初期化
		std::unique_ptr<Particle>& spawnedParticle = particles_.emplace_back<std::unique_ptr<Particle>>(std::make_unique<Particle>());
		spawnedParticle->Init(particleInitialTransform_,
							  particleLifeTime_);
	}
}