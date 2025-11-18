#pragma once

#include <chrono>
#include <limits>
#include <random>
#include <stdint.h>

namespace MyRandom{
	static std::mt19937_64 engine(std::chrono::system_clock::now().time_since_epoch().count());

    
    /// <summary>
    /// Int 型の乱数を生成するクラス
    /// </summary>
	class Int{
	private:
		std::uniform_int_distribution<int32_t> distribution;

	public:
		Int(int32_t min = (std::numeric_limits<int32_t>::min)(),int32_t max = (std::numeric_limits<int32_t>::max)())
			: distribution(min,max){}

		int32_t Get(){
			return distribution(engine);
		}

		void SetRange(int32_t min,int32_t max){
			distribution = std::uniform_int_distribution<int32_t>(min,max);
		}
	};

    /// <summary>
    /// Float 型の乱数を生成するクラス
    /// </summary>
	class Float{
	private:
		std::uniform_real_distribution<float> distribution;

	public:
		Float(float min = 0.0,float max = 1.0)
			: distribution(min,max){}

		float Get(){
			return distribution(engine);
		}

		void SetRange(float min,float max){
			distribution = std::uniform_real_distribution<float>(min,max);
		}
	};

}
