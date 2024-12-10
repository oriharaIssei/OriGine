#include "Model.h"

#include <cassert>

Matrix4x4 Animation::CalculateCurrentLocal(){
	Matrix4x4 local = MakeMatrix::Identity();
	for(auto& [animationName,nodeAnimation] : nodeAnimations){
		Vector3 scale = CalculateValue(nodeAnimation.scale,currentAnimationTime);
		Quaternion rotate = CalculateValue(nodeAnimation.rotate,currentAnimationTime);
		Vector3 translate = CalculateValue(nodeAnimation.translate,currentAnimationTime);
		local = MakeMatrix::Affine(scale,rotate,translate);
	}
	return local;
}

Vector3 Animation::CalculateValue(const std::vector<KeyframeVector3>& keyframes,float time){
	///===========================================
	/// 例外処理
	///===========================================
	{
		assert(!keyframes.empty());
		if(keyframes.size() == 1 || time <= keyframes[0].time){
			return keyframes[0].value;
		}
	}
	for(size_t index = 0; index < keyframes.size() - 1; ++index){
		size_t nextIndex = index + 1;

		// index と nextIndex の 2つを 取得して 現時刻が 範囲内か
		if(keyframes[index].time <= time &&
		   time <= keyframes[nextIndex].time){
			// 範囲内 で 保管
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Lerp(keyframes[index].value,keyframes[nextIndex].value,t);
		}
	}

	// 登録されている時間より 後ろ
	// 最後の 値を返す
	return (*keyframes.rbegin()).value;
}

Quaternion Animation::CalculateValue(const std::vector<KeyframeQuaternion>& keyframes,float time){
	///===========================================
	/// 例外処理
	///===========================================
	{
		assert(!keyframes.empty());
		if(keyframes.size() == 1 || time <= keyframes[0].time){
			return keyframes[0].value;
		}
	}
	for(size_t index = 0; index < keyframes.size() - 1; ++index){
		size_t nextIndex = index + 1;

		// index と nextIndex の 2つを 取得して 現時刻が 範囲内か
		if(keyframes[index].time <= time &&
		   time <= keyframes[nextIndex].time){
			// 範囲内 で 保管
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Lerp(keyframes[index].value,keyframes[nextIndex].value,t);
		}
	}

	// 登録されている時間より 後ろ
	// 最後の 値を返す
	return (*keyframes.rbegin()).value;
}
