#pragma once

class Enemy;
class EnemyManager;
class Beam;
class CollisionManager{
public:
	CollisionManager() = default;

	void Update(EnemyManager* _enemyManager,Beam* _beam);
private:
	bool CheckCollison(Beam* _beam,const Enemy* _enemy);
};
