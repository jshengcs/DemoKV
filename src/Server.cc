#include "Server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <functional>

#include "Log.h"

Server::Server(int thread_num, int port)
    : base_loop_(new EventLoop()),
      thread_num_(thread_num),
      port_(port),
      started_(false),
      accept_fd_(socketBind(port_)) {
#if OP
    skip_list_ = new OptimisticSkipList<std::string, std::string>(20);
#else
    skip_list_ = new CoarseSkipList<std::string, std::string>(20);
#endif
    accept_channel_ =
        std::make_shared<Channel>(base_loop_, accept_fd_, skip_list_);
    accept_channel_->setFd(accept_fd_);  // attention
    if (setSocketNonBlocking(accept_fd_) < 0) {
        perror("set socket non block failed");
        abort();
    }
}

Server::~Server() {}

void Server::start() {
    LOG << "server start\n";
    //从reactor start
    for (int i = 0; i < thread_num_; i++) {
        std::cout << "创建线程:   " << i << '\n';
        std::shared_ptr<EventLoopThread> th(new EventLoopThread());
        threads_.push_back(th);
        loops_.push_back(th->startLoop());
    }
    accept_channel_->setEvents(EPOLLIN | EPOLLET);
    accept_channel_->setReadHandle(std::bind(&Server::newConnHandler, this));
    std::cout << "accept channel fd: " << accept_channel_->getFd() << '\n';
    // std::cout << "usecount: " << accept_channel_.use_count() << '\n';
    base_loop_->addToEpoller(accept_channel_);
    started_ = true;

    //主reactor start
    base_loop_->loop();
}

void Server::newConnHandler() {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    while (true) {
        int new_conn_fd = accept(accept_fd_, (struct sockaddr*)&client_addr,
                                 &client_addr_len);
        if (new_conn_fd < 0) break;
        std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr)
                  << ":" << ntohs(client_addr.sin_port)
                  << "   fd:  " << new_conn_fd << '\n';
        if (new_conn_fd >= MAXFDS) {
            close(new_conn_fd);
            continue;
        }
        setSocketNonBlocking(new_conn_fd);
        EventLoop* next_loop = getNextLoop();
        // setSocketNodelay(new_conn_fd);
        std::shared_ptr<Channel> newConn =
            std::make_shared<Channel>(next_loop, new_conn_fd, skip_list_);
        newConn->setEvents(EPOLLIN | EPOLLET);
        next_loop->addToEpoller(newConn);
    }
}

EventLoop* Server::getNextLoop() {
    assert(started_);
    // 0加入主线程
    EventLoop* loop = base_loop_;
    if (!loops_.empty()) {
        loop = loops_[next_];
        next_ = (next_ + 1) % thread_num_;
    }
    return loop;
}

/*
void setSocketNodelay(int fd) {
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
}
*/

int Server::socketBind(int port) {
    if (port < 0 || port > 65535) return -1;

    //创建socket
    int accept_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (accept_fd == -1) return -1;

    // 设置服务器IP和端口
    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);
    if (bind(accept_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) ==
        -1) {
        close(accept_fd);
        return -1;
    }
    std::cout << "bind ok"
              << "\n";

    // 开始监听
    if (listen(accept_fd, 2048) == -1) {
        close(accept_fd);
        return -1;
    }
    std::cout << "listen ok"
              << "\n";
    // 无效描述符
    if (accept_fd == -1) {
        close(accept_fd);
        return -1;
    }
    std::cout << "acceptfd ok"
              << "\n";
    return accept_fd;
}

int Server::setSocketNonBlocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1) return -1;

    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) == -1) return -1;
    return 0;
}