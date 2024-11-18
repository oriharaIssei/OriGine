#pragma once

class Enemy;
class EnemyManager;
class Reticle;
class Beam;
class RailCamera;
class CollisionManager{
public:
	CollisionManager() = default;

	void Update(EnemyManager* _enemyManager,const Beam* _beam,const Reticle* _reticle,const RailCamera* railCamera);
private:
	bool CheckCollison(const Reticle* _reticle,const Enemy* _enemy,const RailCamera* _railCamera);
};
