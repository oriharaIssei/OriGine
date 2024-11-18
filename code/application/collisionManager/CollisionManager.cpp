#include "CollisionManager.h"

#include <algorithm>

#include "../Beam/Beam.h"
#include "../enemy/EnemyManager.h"
#include "../railCamera/RailCamera.h"
#include "../Reticle/Reticle.h"

void CollisionManager::Update(EnemyManager* _enemyManager,const Beam* _beam,const Reticle* reticle,const RailCamera* railCamera){
	if(!_beam->getIsActive()){
		return;
	}
	for(auto& enemy : _enemyManager->getActiveEnemies()){
		if(CheckCollison(reticle,enemy.get(),railCamera)){
			enemy->OnCollision();
		}
	}
}

/// <summary>
/// Reticle の 2D座標と Enemy の 2D座標が接触しているか
/// </summary>
bool CollisionManager::CheckCollison(const Reticle* _reticle,const Enemy* _enemy,const RailCamera* _railCamera){
	if(!_enemy->getIsAlive()){
		return false; // 敵が死んでいる場合は無視
	}


	// 敵のワールド座標を計算
	Vector3 enemyLeftPos = _enemy->GetPos() - Vector3(_enemy->GetRadius(),0.0f,0.0f);
	Vector3 enemyRightPos = _enemy->GetPos() + Vector3(_enemy->GetRadius(),0.0f,0.0f);

	Vector3 enemyScreenPos = _reticle->getVpvMat() * _enemy->GetPos();
	Vector3 enemyScreenLeftPos = _reticle->getVpvMat() * enemyLeftPos;
	Vector3 enemyScreenRightPos = _reticle->getVpvMat() * enemyRightPos;

	if(enemyScreenPos.x > 1280.0f || enemyScreenPos.x < 0.0f
	   || enemyScreenPos.y > 720.0f || enemyScreenPos.y < 0.0f){
		return false;
	}

	// 敵のスクリーン空間における半径の2乗
	float enemyScreenRadiusSq = (enemyScreenRightPos.x - enemyScreenLeftPos.x) * (enemyScreenRightPos.x - enemyScreenLeftPos.x);

	// レティクルのスクリーン座標を取得
	const Vector3& reticleScreenPos = _reticle->getScreenPos();

	// スクリーン空間での衝突判定 (距離の2乗と半径の2乗を比較)
	if((enemyScreenPos - reticleScreenPos).lengthSq() <= enemyScreenRadiusSq){
		return true; // 衝突している
	}

	return false; // 衝突していない
}
