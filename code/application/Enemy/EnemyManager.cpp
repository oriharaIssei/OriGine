#include "EnemyManager.h"

#include "Enemy.h"

#include <iostream>
#include <string>

const std::string directory = "resource/";

void EnemyManager::Init(){
	Load();
}

void EnemyManager::Update(){

}

void EnemyManager::Draw(IConstantBuffer<CameraTransform>& cameraTransform){}

void EnemyManager::Load(){

}

void EnemyManager::Save(){}

void EnemyManager::Debug(){}

void EnemySpawnEvent::Init(float t){
	eventTriggerT_;
}

std::list<std::unique_ptr<Enemy>>& EnemySpawnEvent::Spawn(){
	return enemies_;
}

void EnemySpawnEvent::SetEnemy(std::unique_ptr<Enemy> enemy){ enemies_.push_back(std::move(enemy)); }
