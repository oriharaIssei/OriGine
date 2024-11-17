#pragma once

class Enemy;
class EnemyManager;
class Reticle;
class Beam;
class CollisionManager{
public:
	CollisionManager() = default;

	void Update(EnemyManager* _enemyManager,Beam* _beam,Reticle* _reticle);
private:
	bool CheckCollison(Reticle* _reticle,const Enemy* _enemy);
};
