#pragma once

#include <chrono>
#include <limits>
#include <random>
#include <stdint.h>

namespace MyRandom{
	static std::mt19937_64 engine(std::chrono::system_clock::now().time_since_epoch().count());

	class Int{
	private:
		std::uniform_int_distribution<int32_t> distribution;

	public:
		Int(int32_t min = (std::numeric_limits<int32_t>::min)(),int32_t max = (std::numeric_limits<int32_t>::max)())
			: distribution(min,max){}

		int32_t get(){
			return distribution(engine);
		}

		void setRange(int32_t min,int32_t max){
			distribution = std::uniform_int_distribution<int32_t>(min,max);
		}
	};

	class Float{
	private:
		std::uniform_real_distribution<float> distribution;

	public:
		Float(float min = 0.0,float max = 1.0)
			: distribution(min,max){}

		float get(){
			return distribution(engine);
		}

		void setRange(float min,float max){
			distribution = std::uniform_real_distribution<float>(min,max);
		}
	};

}
