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
	enemyModel_ = ModelManager::getInstance()->Create("resource","teapot.obj");

	GlobalVariables* variables	= GlobalVariables::getInstance();
	variables->addValue("Game","EnemyManager","enemySpawnEventSize_",eventSize_);
	for(int32_t i = 0; i < eventSize_; ++i){
		auto& spawnEvent = spawnEvents_.emplace_back();
		spawnEvent->Init(i,enemyModel_);
	}
}

void EnemyManager::Update(float currentDistance){
#ifdef _DEBUG
	if(preEventSize_ != eventSize_){
		if(eventSize_ > preEventSize_){
			auto& spawnEvent = spawnEvents_.emplace_back(new EnemySpawnEvent());
			spawnEvent->Init(static_cast<int32_t>(spawnEvents_.size() - 1),enemyModel_);
		} else{
			if(!spawnEvents_.empty()){
				spawnEvents_.pop_back();
			}
		}
	}

	int32_t eventIndex_ = 0;
	for(auto& spawnEvent : spawnEvents_){
		spawnEvent->Debug(eventIndex_,enemyModel_);
		eventIndex_++;
	}

	preEventSize_ = eventSize_;
#endif // _DEBUG
}

void EnemyManager::Draw(IConstantBuffer<CameraTransform>& cameraTransform){
#ifdef _DEBUG
	for(auto& spawnEvent : spawnEvents_){
		for(auto& debugEnemy : spawnEvent->GetEnemyList()){
			debugEnemy->Draw(cameraTransform);
		}
	}
#endif // _DEBUG

#ifndef _DEBUG
	for(auto& enemy : activeEnemies_){
		enemy->Draw(cameraTransform);
	}
#endif // _DEBUG
}
#pragma endregion

#pragma region"SpawnEvent"
void EnemySpawnEvent::Init(int32_t eventNum,Model* model){
#ifndef _DEBUG
	std::string groupName_;
#endif // !_DEBUG

	groupName_ = "EnemySpawnEvent::" + std::to_string(eventNum);
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
}
#endif // _DEBUG

std::list<std::unique_ptr<Enemy>>& EnemySpawnEvent::Spawn(){
	return enemies_;
}

void EnemySpawnEvent::SetEnemy(std::unique_ptr<Enemy> enemy){ enemies_.push_back(std::move(enemy)); }
#pragma endregion