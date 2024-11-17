#include "CollisionManager.h"

#include <algorithm>

#include "../Beam/Beam.h"
#include "../enemy/EnemyManager.h"
#include "../Reticle/Reticle.h"

#pragma region "Math Functions"
Vector3 Projection(const Vector3& v1,const Vector3& v2){
	return Vector3(v2.Normalize() * (v1.dot(v2.Normalize())));
}

Vector3 ClosestPointOnSegment(const Vector3& point,const Vector3& segmentOrigin,const Vector3& segmentEnd){
	Vector3 segmentDiff = segmentEnd - segmentOrigin;
	float segmentLengthSquared = segmentDiff.lengthSq();

	// セグメントの長さが0に近い場合は、始点を返す
	if(segmentLengthSquared < std::numeric_limits<float>::epsilon()){
		return segmentOrigin;
	}

	// セグメント上の最も近い位置のスカラー値 t を計算し、[0, 1]でクランプ
	float t = (point - segmentOrigin).dot(segmentDiff) / segmentLengthSquared;
	t = (std::max)(0.0f,(std::min)(1.0f,t));

	// 最近傍点を計算
	return segmentOrigin + segmentDiff * t;
}
#pragma endregion

void CollisionManager::Update(EnemyManager* _enemyManager,Beam* _beam,Reticle* reticle){
	if(!_beam->getIsActive()){
		return;
	}
	for(auto& enemy : _enemyManager->getActiveEnemies()){
		if(CheckCollison(reticle,enemy.get())){
			enemy->OnCollision();
		}
	}
}

/// <summary>
/// Reticle の 2d座標 と Enemy の 2d座標 が 接触しているか
/// </summary>
bool CollisionManager::CheckCollison(Reticle* reticle,const Enemy* _enemy){
	if(!_enemy->getIsAlive()){
		return false;
	}

	Vector3 enemyLeftPos = _enemy->GetPos();
	enemyLeftPos.x -= _enemy->GetRadius();
	Vector3 enemyRightPos =  _enemy->GetPos();
	enemyRightPos.x += _enemy->GetRadius();

	Vector3 enemyScreenLeftPos = reticle->getVpvMat() * enemyLeftPos;
	Vector3 enemyScreenRightPos = reticle->getVpvMat() * enemyRightPos;
	Vector3 enemyScreenPos = reticle->getVpvMat() * _enemy->GetPos();

	float enemyScreenRadiusSq = (enemyScreenLeftPos.x - enemyScreenRightPos.x) * (enemyScreenLeftPos.x - enemyScreenRightPos.x);

	const Vector3& reticleScreenPos = reticle->getScreenPos();

	if((enemyScreenPos - reticleScreenPos).lengthSq() - enemyScreenRadiusSq <= 16.0f){
		return true;
	}

	return false;
}