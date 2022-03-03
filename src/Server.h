#ifndef SERVER_H_
#define SERVER_H_

#include <memory>

#include "Channel.h"
#include "CoarseSkipList.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "OptimisticSkipList.h"
#include "SkipList.h"
#define OP 0

class Server {
public:
    Server(int thread_num, int port);
    ~Server();
    EventLoop* getLoop();
    void start();
    void newConnHandler();
    void thisConnHandler();
    EventLoop* getNextLoop();

private:
    EventLoop* base_loop_;
    int thread_num_;
    bool started_;
    std::vector<std::shared_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
    int next_ = 0;
    std::shared_ptr<Channel> accept_channel_;
    int port_;
    int accept_fd_;
    SkipList<string, string>* skip_list_;
    static const int MAXFDS = 1000000;
    int socketBind(int port);
    int setSocketNonBlocking(int fd);
};

#endif