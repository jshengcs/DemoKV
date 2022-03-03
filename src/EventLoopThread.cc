#include "EventLoopThread.h"

EventLoopThread::EventLoopThread()
    : loop_(nullptr), exiting_(false), thread_(), mutex_(), cond_() {}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    thread_ = std::thread(&EventLoopThread::threadFunc, this);
    {
        std::unique_lock<std::mutex> lock(mutex_);
        //等待thread运行
        while (loop_ == nullptr) cond_.wait(lock);
    }
    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop_->loop();
    loop_ = nullptr;
}