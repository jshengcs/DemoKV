#include "Epoller.h"

const int MAXNUM = 4096;

Epoller::Epoller() : epoll_fd_(epoll_create(MAXNUM)), events_(MAXNUM) {
    // std::cout << "create epoll_fd_:-----" << epoll_fd_ << '\n';
    assert(epoll_fd_ > 0);
}

Epoller::~Epoller() {}

void Epoller::epollAdd(std::shared_ptr<Channel> channel) {
    if (fd2chan_.find(channel->getFd()) != fd2chan_.end()) {
        // std::cout << "epollAdd: channel exits!" << '\n';
        return;
    }

    // std::cout << "usecount: " << channel.use_count() << '\n';

    int fd = channel->getFd();
    epoll_event event;
    event.data.fd = fd;
    event.events = channel->getEvents();
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) == 0) {
        // std::cout << "epoll add success: " << fd << '\n';
        fd2chan_[fd] = channel;
    } else {
        perror("epollAdd error");
    }
}

void Epoller::epollDel(std::shared_ptr<Channel> channel) {
    if (fd2chan_.find(channel->getFd()) == fd2chan_.end()) {
        // std::cout << "epollDel: channel not exits!" << '\n';
        return;
    }

    int fd = channel->getFd();
    epoll_event event;
    event.data.fd = fd;
    event.events = channel->getEvents();
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) == 0) {
        fd2chan_.erase(fd);
        // std::cout << "epoll del success: " << fd << '\n';
    } else {
        perror("epollDel error");
    }
}

void Epoller::epollMod(std::shared_ptr<Channel> channel) {
    int fd = channel->getFd();
    epoll_event event;
    event.data.fd = fd;
    event.events = channel->getEvents();
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) < 0) {
        perror("epollMod error");
    }
}

std::vector<std::shared_ptr<Channel>> Epoller::epoll() {
    // LOG << "++ in thread" << gettid_() << " epoll\n";
    int event_count =
        epoll_wait(epoll_fd_, &*events_.begin(), events_.size(), -1);
    // std::cout << "end epoll: eventcount-" << event_count << '\n';
    if (event_count < 0) perror("epoll wait error!");
    std::vector<std::shared_ptr<Channel>> active_channels;
    for (int i = 0; i < event_count; i++) {
        int fd = events_[i].data.fd;
        // std::cout << "epoller:epoll event" << fd << '\n';
        std::shared_ptr<Channel> channel = fd2chan_[fd];
        // std::cout << "channel " << fd << ": " << channel.use_count() << '\n';
        if (channel) {
            channel->setRevents(events_[i].events);
            // channel->setEvents(0);
            active_channels.push_back(channel);
        }
    }

    return active_channels;
}