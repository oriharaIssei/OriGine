#pragma once

#include <chrono>

/// <summary>
/// DeltaTimeの計測をするクラス
/// </summary>
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
    void setDeltaTime(float dt) { deltaTime_ = dt; }
};
