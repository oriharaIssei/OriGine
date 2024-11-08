#include "EnemyManager.h"
#include "Enemy.h"

#include <iostream>
#include <string>

#include "object3d/ModelManager.h"
#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

const std::string directory = "resource/";

#pragma region"EnemyManager"
void EnemyManager::Init(){
	enemyModel_ = ModelManager::getInstance()->Create("resource","teapot.obj");
	Load();
}

void EnemyManager::Update(float currentDistance){
#ifdef _DEBUG
	ImGui::Begin("EnemyManager");

	if(ImGui::Button("AddEvent")){
		spawnEvents_.emplace_back(new EnemySpawnEvent());
	}

	if(ImGui::TreeNode("SpawnEventList")){
		int32_t index = 0;
		std::string spawnEventButtonLabel = "";
		for(auto& spawnEvent : spawnEvents_){
			spawnEventButtonLabel = "SpawnEvet_" + std::to_string(index);
			if(ImGui::Button(spawnEventButtonLabel.c_str())){
				eventIndex_ = index;
				currentDebugEnemySpawnEvent_ = spawnEvent.get();
				break;
			}
			index++;
		}
		ImGui::TreePop();
	}

	if(currentDebugEnemySpawnEvent_){
		currentDebugEnemySpawnEvent_->Debug(eventIndex_,enemyModel_);
	}

	ImGui::End();
#endif // _DEBUG
}

void EnemyManager::Draw(IConstantBuffer<CameraTransform>& cameraTransform){
#ifdef _DEBUG
	if(currentDebugEnemySpawnEvent_){
		for(auto& debugEnemy : currentDebugEnemySpawnEvent_->GetEnemyList()){
			debugEnemy->Draw(cameraTransform);
		}
	}
#endif // _DEBUG

	for(auto& enemy : enemies_){
		enemy->Draw(cameraTransform);
	}
}
void EnemyManager::Load(){}
void EnemyManager::Save(){}
#pragma endregion

#pragma region"SpawnEvent"
void EnemySpawnEvent::Init(float t){
	eventTriggerDistance_ = t;
}

#ifdef _DEBUG
void EnemySpawnEvent::Debug(int32_t num,Model* model){
	std::string label = "EnemySpawnEvent::" + std::to_string(num);
	if(ImGui::TreeNode(label.c_str())){
		ImGui::DragFloat("eventTriggerDistance",&eventTriggerDistance_,0.1f);

		if(ImGui::Button("AddEnemy")){
			enemies_.emplace_back(new Enemy());
			enemies_.back()->Init(
				enemyInitializeVariables_[enemies_.back().get()].first,
				enemyInitializeVariables_[enemies_.back().get()].second,
				model
			);
		}

		if(ImGui::TreeNode("EnemyList")){
			int32_t enemyIndex = 0;
			std::string enemyButtonLabel = "";
			for(auto& enemy : enemies_){
				std::string enemyButtonLabel = "Enemy_" + std::to_string(enemyIndex);
				if(ImGui::Button(enemyButtonLabel.c_str())){
					currentDebugEnemy_ = enemy.get();
					currentEnemyVariables_ = &enemyInitializeVariables_[enemies_.back().get()];
					break;
				}
				enemyIndex++;
			}
			ImGui::TreePop();
		}

		if(currentDebugEnemy_){
			ImGui::DragFloat3("SpawnPos",&currentEnemyVariables_->first.x,0.1f);
			ImGui::DragFloat3("SpawnVelocity",&currentEnemyVariables_->second.x,0.1f);
			currentDebugEnemy_->SetPos(currentEnemyVariables_->first);
			currentDebugEnemy_->SetPos(currentEnemyVariables_->second);
		}
		ImGui::TreePop();
	}
}
#endif // _DEBUG

std::list<std::unique_ptr<Enemy>>& EnemySpawnEvent::Spawn(){
	return enemies_;
}

void EnemySpawnEvent::SetEnemy(std::unique_ptr<Enemy> enemy){ enemies_.push_back(std::move(enemy)); }
#pragma endregion