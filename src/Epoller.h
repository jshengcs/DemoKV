#ifndef EPOLLER_H_
#define EPOLLER_H_

#include <assert.h>
#include <sys/epoll.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "Channel.h"
#include "util.h"

class Epoller {
public:
    Epoller();
    ~Epoller();
    void epollAdd(std::shared_ptr<Channel> channel);
    void epollDel(std::shared_ptr<Channel> channel);
    void epollMod(std::shared_ptr<Channel> channel);
    std::vector<std::shared_ptr<Channel>> epoll();

private:
    std::unordered_map<int, std::shared_ptr<Channel>> fd2chan_;
    int epoll_fd_;
    std::vector<epoll_event> events_;  // epoll_wait返回
};

#endif
