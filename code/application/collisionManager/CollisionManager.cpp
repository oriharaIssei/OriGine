#include "CollisionManager.h"

#include "../beam/Beam.h"
#include "../enemy/EnemyManager.h"

#pragma region "Math Functions"
Vector3 Projection(const Vector3& v1,const Vector3& v2){
	return Vector3(v2.Normalize() * (v1.dot(v2.Normalize())));
}
Vector3 ClosestPoint(const Vector3& point,const Vector3& segmentOrigin,const Vector3& segmentDiff){
	return segmentOrigin + Projection(point - segmentOrigin,segmentDiff);
}
#pragma endregion

void CollisionManager::Update(EnemyManager* _enemyManager,Beam* _beam){
	for(auto& enemy : _enemyManager->getActiveEnemies()){
		if(!CheckCollison(_beam,enemy.get())){
			enemy->OnCollision();
		}
	}
}

bool CollisionManager::CheckCollison(Beam* _beam,const Enemy* _enemy){
	const Vector3 enemyPos = _enemy->GetPos();
	const float  collisionRadius = _enemy->GetRadius() + _beam->getRadius();

	// 左右の ビーム それぞれ と 一番近い 点を求める (各ビーム の 線分上)
	Vector3 leftSeg_ClosestPoint  = ClosestPoint(enemyPos,_beam->getLeftOrigin(),_beam->getEndPos());
	Vector3 rightSeg_ClosestPoint = ClosestPoint(enemyPos,_beam->getRightOrigin(),_beam->getEndPos());

	/// 右 の ビームと 接触 していないか
	if((leftSeg_ClosestPoint - enemyPos).length() >= collisionRadius){
		return false;
	}
	/// 左 の ビームと 接触 していないか
	if((rightSeg_ClosestPoint - enemyPos).length() >= collisionRadius){
		return false;
	}

	return true;
}