#pragma once

#include <chrono>

class DeltaTime{
public:
	void Initialize();
	void Update();
private:
	float deltaTime_;

	std::chrono::high_resolution_clock::time_point currentTime_;
	std::chrono::high_resolution_clock::time_point preTime_;
public:
	float getDeltaTime()const{ return deltaTime_; }
};