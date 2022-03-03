#include "EventLoop.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>

__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop()
    : looping_(false),
      epoller_(new Epoller()),
      quit_(false),
      event_handling_(false),
      thread_id_(gettid_()) {
    // one loop per thread
    if (!t_loopInThisThread) {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop() { t_loopInThisThread = 0; }

bool EventLoop::isInLoopThread() { return thread_id_ == gettid_(); }

// void EventLoop::shutDownWR(int fd) { shutdown(fd, SHUT_WR); }

void EventLoop::removeFromEpoller(std::shared_ptr<Channel> channel) {
    epoller_->epollDel(channel);
}

void EventLoop::updateEpoller(std::shared_ptr<Channel> channel) {
    epoller_->epollMod(channel);
}
void EventLoop::addToEpoller(std::shared_ptr<Channel> channel) {
    epoller_->epollAdd(channel);
}

void EventLoop::loop() {
    //避免重复loop
    assert(!looping_);
    //避免其他线程调用
    assert(isInLoopThread());
    LOG << "++ in thread" << gettid_() << " eventloop loop\n";
    // looping_ = true;
    quit_ = false;
    std::vector<std::shared_ptr<Channel>> active_channels;
    while (!quit_) {
        active_channels = epoller_->epoll();
        for (auto&& channel : active_channels) {
            channel->handleEvents();
        }
        active_channels.clear();
    }
    // looping_ = false;
}

// void EventLoop::runInLoop(func& cb) {
//     if (isInLoopThread()) {  // 线程内调用
//         cb();
//     }
// }

void EventLoop::quit() {
    quit_ = true;
    // quit时可能阻塞在poll(), 需要wakeup
    //同理，在本IO线程中调用quit，表面未阻塞在poll(), 不需要调用wakeup
    // if (!isInLoopThread()) {
    //    wakeup();
    //}
}
