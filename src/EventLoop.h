#ifndef EVENT_LOOP_H_
#define EVENT_LOOP_H_

#include <sys/syscall.h>
#include <unistd.h>

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "Channel.h"
#include "Epoller.h"
#include "util.h"

class EventLoop {
private:
    using func = std::function<void()>;
    bool looping_;
    std::shared_ptr<Epoller> epoller_;
    bool quit_;
    bool event_handling_;
    std::mutex mutex_;
    const pid_t thread_id_;

public:
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    bool isInLoopThread();
    bool assertInLoopThread();
    // void runInLoop(func& cb);
    //  void shutdown(std::shared_ptr<Channel> channel);
    void removeFromEpoller(std::shared_ptr<Channel> channel);
    void updateEpoller(std::shared_ptr<Channel> channel);
    void addToEpoller(std::shared_ptr<Channel> channel);
};

#endif