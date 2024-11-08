#pragma once

#include <deque>
#include <list>
#include <memory>
#include <stdint.h>
#include <unordered_map>

#include "directX12/IConstantBuffer.h"
#include "Enemy.h"
#include "transform/CameraTransform.h"
#include "Vector3.h"
#ifdef _DEBUG
struct EnemyInitializeVariables;
#endif // _DEBUG

struct Model;

class EnemySpawnEvent{
public:
	EnemySpawnEvent() = default;
	~EnemySpawnEvent(){}

	void Init(float t);

#ifdef _DEBUG
	void Debug(int32_t num,Model* model);
#endif // _DEBUG

	std::list<std::unique_ptr<Enemy>>& Spawn();
private:
	float eventTriggerDistance_;

#ifdef _DEBUG
	Enemy* currentDebugEnemy_ = nullptr;
	std::pair<Vector3,Vector3>* currentEnemyVariables_ = nullptr;
	std::unordered_map < Enemy*,std::pair<Vector3,Vector3>> enemyInitializeVariables_;
#endif // _DEBUG

	std::list<std::unique_ptr<Enemy>> enemies_;
public:
	void SetEnemy(std::unique_ptr<Enemy> enemy);
	std::list<std::unique_ptr<Enemy>>& GetEnemyList(){ return enemies_; }
	float GetTriggerDistance()const{ return eventTriggerDistance_; }
};

class EnemyManager{
public:
	void Init();
	void Update(float currentDistance);
	void Draw(IConstantBuffer<CameraTransform>& cameraTransform);
private:
	void Load();
	void Save();
private:
	Model* enemyModel_;
	std::deque<std::unique_ptr<EnemySpawnEvent>> spawnEvents_;
#ifdef _DEBUG
	int32_t eventIndex_ = 0;
	EnemySpawnEvent* currentDebugEnemySpawnEvent_;
#endif // _DEBUG

	std::list<std::unique_ptr<Enemy>> enemies_;
};