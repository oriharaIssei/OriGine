#include "Spline.h"

Vector3 CatmullRomInterpolation(const Vector3& p0,const Vector3& p1,const Vector3& p2,const Vector3& p3,float t){
	constexpr float half = 0.5f;

	float t2 = t * t;
	float t3 = t2 * t;

	Vector3 e3 = -p0 + 3.0f * p1 - 3.0f * p2 + p3;
	Vector3 e2 = 2 * p0 - 5 * p1 + 4 * p2 - p3;
	Vector3 e1 = -p0 + p2;
	Vector3 e0 = 2 * p1;

	return half * (e3 * t3 + e2 * t2 + e1 * t + e0);
}

Vector3 CatmullRomInterpolation(const std::vector<Vector3>& points,float t){
	assert(points.size() >= 4 && "制御点は 4点以上必要です");
	size_t division = points.size() - 1;
	float areaWidth = 1.0f / division;

	float t_2 = std::fmod(t,areaWidth) * division;
	t_2 = std::clamp(t_2,0.0f,1.0f);

	size_t index = static_cast<size_t>(t / areaWidth);
	index = (std::min)(index,division - 1);

	size_t index0 = index - 1;
	size_t index1 = index;
	size_t index2 = index + 1;
	size_t index3 = index + 2;

	if(index == 0){
		index0 = index1;
	}
	if(index3 >= points.size()){
		index3 = index2;
	}

	Vector3 p0 = points[index0];
	Vector3 p1 = points[index1];
	Vector3 p2 = points[index2];
	Vector3 p3 = points[index3];

	return CatmullRomInterpolation(p0,p1,p2,p3,t_2);
}

Spline::Spline(const std::vector<Vector3>& points): controlPoints_(points){
	CalculateArcLength();
}

void Spline::CalculateArcLength(){
	arcLengthSegments_.clear();
	totalLength_ = 0.0f;
	constexpr int NUM_STEPS = 2048;
	Vector3 prevPoint = GetPosition(0.0f);
	arcLengthSegments_.emplace_back(ArcLengthSegment{0.0f,0.0f});

	for(int i = 1; i <= NUM_STEPS; ++i){
		float t = static_cast<float>(i) / NUM_STEPS;
		Vector3 currentPoint = GetPosition(t);
		float segmentLength = (currentPoint - prevPoint).length();
		totalLength_ += segmentLength;
		arcLengthSegments_.emplace_back(ArcLengthSegment{t,totalLength_});
		prevPoint = currentPoint;
	}
}

float Spline::GetTFromDistance(float distance) const{
	if(distance <= 0.0f) return 0.0f;
	if(distance >= totalLength_) return 1.0f;

	// 二分探索で対応するセグメントを探す
	size_t low = 0;
	size_t high = arcLengthSegments_.size() - 1;
	while(low <= high){
		size_t mid = low + (high - low) / 2;
		if(arcLengthSegments_[mid].length < distance){
			low = mid + 1;
		} else{
			high = mid - 1;
		}
	}

	// 線形補間でtを計算
	if(low == 0) return arcLengthSegments_[0].t;
	float prevLength = arcLengthSegments_[low - 1].length;
	float segmentLength = arcLengthSegments_[low].length - prevLength;
	float segmentT = arcLengthSegments_[low].t - arcLengthSegments_[low - 1].t;
	float ratio = (distance - prevLength) / segmentLength;
	return arcLengthSegments_[low - 1].t + ratio * segmentT;
}
