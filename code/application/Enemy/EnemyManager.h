#pragma once

#include <list>
#include <memory>

#include "directX12/IConstantBuffer.h"
#include "transform/CameraTransform.h"
class Enemy;

class EnemySpawnEvent{
public:
	EnemySpawnEvent() = default;
	~EnemySpawnEvent(){}

	void Init(float t);
	std::list<std::unique_ptr<Enemy>>& Spawn();
private:
	float eventTriggerT_;

	std::list<std::unique_ptr<Enemy>> enemies_;
public:
	void SetEnemy(std::unique_ptr<Enemy> enemy);
	float GetTriggerT()const{ return eventTriggerT_; }
};

class EnemyManager{
public:
	void Init();
	void Update();
	void Draw(IConstantBuffer<CameraTransform>& cameraTransform);
private:
	void Load();
	void Save();

	void Debug();
private:
	std::list<std::unique_ptr<Enemy>> enemies_;
};