#include "DeltaTime.h"

void DeltaTime::Init(){
	currentTime_ = std::chrono::high_resolution_clock::now();
	preTime_ = currentTime_;
}

void DeltaTime::Update(){
	preTime_ = currentTime_;
	currentTime_ = std::chrono::high_resolution_clock::now();
	deltaTime_ = static_cast<float>(std::chrono::duration<float>(currentTime_ - preTime_).count());
}
