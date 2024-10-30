#pragma once

#include <concepts>
#include <functional>
#include <vector>

#include <mutex>
#include <queue>
#include <thread>

#include <stdint.h>

class IThread{
public:
	IThread() = default;
	virtual ~IThread(){}
	/// <summary>
	/// 初期化 
	/// </summary>
	/// <param name="threadNum">開くスレッド数</param>
	virtual void Init(int32_t threadNum);
	/// <summary>
	/// 終了処理(必須)
	/// </summary>
	virtual void Finalize();
protected:
	/// <summary>
	/// Thread 内で 実行される 処理
	/// </summary>
	virtual void Update() = 0;
protected:
	std::vector<std::thread> threads_;
	std::mutex  mutex_;
	std::condition_variable condition_;
	bool stopThread_;
public:
	void StopThread(){

		std::lock_guard<std::mutex> lock(mutex_);
		stopThread_ = true;
	}
	void RunThread(){
		std::lock_guard<std::mutex> lock(mutex_);
		stopThread_ = false;
	}
};

template<typename T>
concept HaveUpdate = requires(T t){
	{ t.Update() } -> std::same_as<void>; // Update()がvoid型であることも要求
};
template <HaveUpdate Task>
class TaskThread :
	public IThread{
public:
	TaskThread():IThread(){}
	~TaskThread()override{}

	void Init(int32_t threadNum)override;
	void Finalize()override;
private:
	void Update()override;

	std::queue<Task> taskQueue_;
public:
	void pushTask(Task task){
		std::lock_guard<std::mutex> lock(mutex_);
		taskQueue_.emplace(task);
		condition_.notify_one();
	}
};

template<HaveUpdate Task>
inline void TaskThread<Task>::Init(int32_t threadNum){
	IThread::Init(threadNum);
}

template<HaveUpdate Task>
inline void TaskThread<Task>::Finalize(){
	IThread::Finalize();
}

template<HaveUpdate Task>
inline void TaskThread<Task>::Update(){
	while(true){
		Task task;
		{
			std::unique_lock<std::mutex> lock(IThread::mutex_);
			IThread::condition_.wait(lock,[this]{ return !taskQueue_.empty() || IThread::stopThread_; });

			task = taskQueue_.front();
			taskQueue_.pop();
		}

		{
			std::lock_guard guard(IThread::mutex_);
			task.Update();
		}
	}
}
