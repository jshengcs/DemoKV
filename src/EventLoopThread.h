#ifndef EVENT_LOOP_THREAD_
#define EVENT_LOOP_THREAD_

#include <condition_variable>
#include <mutex>
#include <thread>

#include "EventLoop.h"

class EventLoopThread {
private:
    void threadFunc();
    EventLoop* loop_;
    bool exiting_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;

public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();
};

#endif