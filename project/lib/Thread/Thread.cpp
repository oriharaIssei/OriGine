#include "Thread.h"

void IThread::Initialize(int32_t threadNum) {
    stopThread_ = false;

    threads_.resize(threadNum);
    for (auto& thread : threads_) {
        thread = std::thread(&IThread::Update, this);
    }
}

void IThread::Finalize() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stopThread_ = true;
    }

    condition_.notify_all();
    for (std::thread& thread : threads_) {
        if (thread.joinable()) {
            thread.join(); // スレッドの終了を待機
        }
    }
    threads_.clear();
}
