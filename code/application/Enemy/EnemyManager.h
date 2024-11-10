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

	void Init(int32_t eventNum,Model* model);

#ifdef _DEBUG
	void Debug(int32_t num,Model* model);
#endif // _DEBUG

	std::list<std::unique_ptr<Enemy>>& Spawn();
private:
	float eventTriggerDistance_;

#ifdef _DEBUG
	std::string groupName_;
	int32_t     preHasEnemySize_;
	int32_t     hasEnemySize_;
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
	Model* enemyModel_;

#ifdef _DEBUG
	int32_t eventSize_ = 0;
	int32_t preEventSize_ = 0;
#endif // _DEBUG

	std::deque<std::unique_ptr<EnemySpawnEvent>> spawnEvents_;
	std::list<std::unique_ptr<Enemy>> activeEnemies_;
public:
	std::list<std::unique_ptr<Enemy>>& getActiveEnemies(){ return activeEnemies_; }
};