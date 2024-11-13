#include "Enemy.h"
#include "EnemyManager.h"

#include <iostream>
#include <string>

#include "globalVariables/GlobalVariables.h"
#include "object3d/ModelManager.h"
#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

const std::string directory = "resource/";

#pragma region"EnemyManager"
void EnemyManager::Init(){
	enemyModel_ = ModelManager::getInstance()->Create("resource/Models","Enemy.obj");

#ifndef  _DUBUG
	int32_t eventSize_;
#endif // ! _DUBUG

	GlobalVariables* variables	= GlobalVariables::getInstance();
	variables->addValue("Game","EnemyManager","enemySpawnEventSize_",eventSize_);
	for(int32_t i = 0; i < eventSize_; ++i){
		auto& spawnEvent = spawnEvents_.emplace_back(new EnemySpawnEvent());
		spawnEvent->Init(i,enemyModel_);
	}
}

void EnemyManager::Update(float currentDistance){

	if(!spawnEvents_.empty()){
		auto& frontEvent = spawnEvents_.front();
		if(frontEvent->GetTriggerDistance() <= currentDistance){
			for(auto& spawnEnemy : frontEvent->GetEnemyList()){
				activeEnemies_.push_back(std::move(spawnEnemy));
			}
			spawnEvents_.pop_front();
		}
	}

	if(!activeEnemies_.empty()){
		for(auto& enemy : activeEnemies_){
			enemy->Update();
		}
		std::erase_if(activeEnemies_,[](std::unique_ptr<Enemy>& enemy){return !enemy->getIsAlive(); });
	}
}

void EnemyManager::Draw(IConstantBuffer<CameraTransform>& cameraTransform){
	for(auto& enemy : activeEnemies_){
		enemy->Draw(cameraTransform);
	}
}
#pragma endregion

#pragma region"SpawnEvent"
void EnemySpawnEvent::Init(int32_t eventNum,Model* model){
#ifndef _DEBUG
	std::string groupName_;
	int32_t hasEnemySize_;
#endif // !_DEBUG

	groupName_ = "EnemySpawnEvent_" + std::to_string(eventNum);
	GlobalVariables* variables	= GlobalVariables::getInstance();
	variables->addValue("Game",groupName_,"eventTriggerDistance",eventTriggerDistance_);
	variables->addValue("Game",groupName_,"hasEnemySize_",hasEnemySize_);

	for(int32_t i = 0; i < hasEnemySize_; i++){
		enemies_.emplace_back(new Enemy());
		enemies_.back()->Init(
			groupName_,
			i,
			model
		);
	}
}

#ifdef _DEBUG
void EnemySpawnEvent::Debug(int32_t num,Model* model){
	if(preHasEnemySize_ != hasEnemySize_){
		if(hasEnemySize_ > preHasEnemySize_){
			enemies_.emplace_back(new Enemy());
			enemies_.back()->Init(
				groupName_,
				static_cast<int32_t>(enemies_.size() - 1),
				model
			);
		} else{
			enemies_.pop_back();
		}
	}

	preHasEnemySize_ = hasEnemySize_;
	for(auto& enemy : enemies_){
		enemy->Update();
	}

}
#endif // _DEBUG

std::list<std::unique_ptr<Enemy>>& EnemySpawnEvent::Spawn(){
	return enemies_;
}

void EnemySpawnEvent::SetEnemy(std::unique_ptr<Enemy> enemy){ enemies_.push_back(std::move(enemy)); }
#pragma endregion