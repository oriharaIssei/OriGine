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
    /// <summary>
    /// !!! 非推奨 !!!
    /// </summary>
    /// <param name="dt"></param>
    void setDeltaTime(float dt) { deltaTime_ = dt; }
};
