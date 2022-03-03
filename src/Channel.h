#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <sys/epoll.h>
#include <sys/socket.h>

#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "CoarseSkipList.h"
#include "OptimisticSkipList.h"
#include "SkipList.h"
#include "util.h"

class EventLoop;

//注意public继承
class Channel : public std::enable_shared_from_this<Channel> {
public:
    using CallBack = std::function<void()>;
    Channel(EventLoop* event_loop, int fd, SkipList<std::string, std::string>*);
    ~Channel();
    void handleEvents();
    void setEvents(uint32_t events);
    void setRevents(uint32_t revents);
    void setReadHandle(CallBack&& read_handle);
    void setWriteHandle(CallBack&& write_handle);
    int getFd();
    uint32_t& getEvents();
    void setFd(int fd);

private:
    EventLoop* loop_;
    int fd_;
    uint32_t revents_;
    uint32_t events_;
    char* read_buf_;
    char* write_buf_;
    int read_len_;
    int write_len_;
    int max_buf_len_;
    SkipList<std::string, std::string>* skip_list_;
    CallBack read_handle_;
    CallBack write_handle_;
    void defaultReadHandle();
    void defaultWriteHandle();
    void requestHandle();
};

#endif