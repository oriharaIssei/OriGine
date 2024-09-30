#include "DeltaTime.h"

DeltaTime* DeltaTime::getInstance(){
	static DeltaTime ins{};
	return &ins;
}

void DeltaTime::Init(){
	currentTime_ = std::chrono::high_resolution_clock::now();
}

void DeltaTime::Update(){
	preTime_ = currentTime_;
	currentTime_ = std::chrono::high_resolution_clock::now();
	deltaTime_ = static_cast<float>(std::chrono::duration<float>(currentTime_ - preTime_).count());
}
