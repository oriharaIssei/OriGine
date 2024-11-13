#include "CollisionManager.h"

#include <algorithm>

#include "../beam/Beam.h"
#include "../enemy/EnemyManager.h"

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

void CollisionManager::Update(EnemyManager* _enemyManager,Beam* _beam){
	if(!_beam->getIsActive()){
		return;
	}
	for(auto& enemy : _enemyManager->getActiveEnemies()){
		if(!CheckCollison(_beam,enemy.get())){
			enemy->OnCollision();
		}
	}
}

bool CollisionManager::CheckCollison(Beam* _beam,const Enemy* _enemy){
	if(!_enemy->getIsAlive()){
		return false;
	}

	const Vector3 enemyPos = _enemy->GetPos();
	const float collisionRadius = _enemy->GetRadius() + _beam->getRadius();

	// 左右のビームそれぞれについて、最も近い点を求める
	Vector3 leftSeg_ClosestPoint = ClosestPointOnSegment(enemyPos,_beam->getLeftOrigin(),_beam->getEndPos());
	Vector3 rightSeg_ClosestPoint = ClosestPointOnSegment(enemyPos,_beam->getRightOrigin(),_beam->getEndPos());

	// 左右のビームと接触しているか確認
	if((leftSeg_ClosestPoint - enemyPos).length() <= collisionRadius){
		return true;
	}
	if((rightSeg_ClosestPoint - enemyPos).length() <= collisionRadius){
		return true;
	}

	return false;
}