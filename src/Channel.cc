#include "Channel.h"

#include "EventLoop.h"

Channel::Channel(EventLoop* event_loop, int fd,
                 SkipList<std::string, std::string>* skip_list)
    : fd_(fd),
      loop_(event_loop),
      skip_list_(skip_list),
      events_(0),
      read_len_(0),
      write_len_(0),
      read_handle_(std::bind(&Channel::defaultReadHandle, this)),
      write_handle_(std::bind(&Channel::defaultWriteHandle, this)) {
    max_buf_len_ = 1024;
    read_buf_ = new char[max_buf_len_];
    write_buf_ = new char[max_buf_len_];
}

Channel::~Channel() {
    std::cout << "----fd close:  " << fd_ << '\n';
    close(fd_);
    delete[] read_buf_;
    delete[] write_buf_;
}

void Channel::handleEvents() {
    // LOG << "++ in thread" << gettid_() << "handleEvents\n";
    if (revents_ & EPOLLIN) {
        // std::cout << " handle read events: " << fd_ << '\n';
        read_handle_();
    }
    if (revents_ & EPOLLOUT) {
        // std::cout << " handle write events: " << fd_ << '\n';
        write_handle_();
    }
}

int Channel::getFd() { return fd_; }

void Channel::setFd(int fd) { fd_ = fd; }

void Channel::setEvents(uint32_t events) { events_ = events; }
void Channel::setRevents(uint32_t revents) { revents_ = revents; }

void Channel::setReadHandle(CallBack&& read_handle) {
    read_handle_ = read_handle;
}

uint32_t& Channel::getEvents() { return events_; }

void Channel::setWriteHandle(CallBack&& write_handle) {
    write_handle_ = write_handle;
}

void Channel::defaultReadHandle() {
    // LOG << "++ in thread" << gettid_() << "defaultReadHandle\n";
    //  char buf[max_buf_len_ + 1];
    //   buf[max_buf_len_] = 0;
    int read_len_ = readn(fd_, read_buf_, max_buf_len_);
    read_buf_[read_len_] = 0;
    if (read_len_ > 0) {
        // std::cout << "  read from: " << fd_ << "  num: " << read_len_ << "  "
        //           << read_buf_ << "\n";
        //   defaultWriteHandle();
        requestHandle();
        defaultWriteHandle();
    } else if (read_len_ == 0) {
        std::shared_ptr<Channel> this_channel = shared_from_this();
        loop_->removeFromEpoller(this_channel);
    }
}

void Channel::defaultWriteHandle() {
    // LOG << "++ in thread" << gettid_() << "defaultWriteHandle\n";
    write_buf_[max_buf_len_ - 1] = 0;
    int write_len_ = writen(fd_, write_buf_, max_buf_len_);
    if (write_len_ > 0) {
        // std::cout << "write success: " << fd_ << "\n";
    }
    write_buf_[0] = '\0';
}

void Channel::requestHandle() {
    LOG << "++ in thread" << gettid_() << "requestHandle  " << read_buf_
        << "\n";
    int index = 0;
    int cnt = 0;
    while (read_buf_[index] != 0 && read_buf_[index] != '\r') {
        cnt = cnt * 10 + (read_buf_[index] - '0');
        index++;
    }
    index += 2;

    // std::cout << "count: " << cnt << '\n';

    char cmd;
    std::string key, value;
    int flag = 0;
    memset(write_buf_, 0, sizeof(write_buf_));
    while (read_buf_[index] != 0 && cnt) {
        if (read_buf_[index] == '\r') {
            flag = 0;
            index += 2;
            // std::cout << cmd << " " << key << " " << value << '\n';
            if (cmd == 'p') {
                if (skip_list_->add(key, value)) {
                    strcat(write_buf_, "put success: ");
                    // write_len_ += 15 + key.size();
                } else {
                    strcat(write_buf_, "put fail\r\n");
                    // std::cout << "put fail\n";
                }
                strcat(write_buf_, key.c_str());
                strcat(write_buf_, "\r\n");
            } else if (cmd == 'd') {
                if (skip_list_->remove(key)) {
                    strcat(write_buf_, "delete success: ");
                    // write_len_ += 18 + key.size();
                } else {
                    strcat(write_buf_, "delete fail\r\n");
                    // write_len_ += 13;
                }
                strcat(write_buf_, key.c_str());
                strcat(write_buf_, "\r\n");
            } else if (cmd == 's') {
                if (skip_list_->search(key)) {
                    strcat(write_buf_, "search success: ");
                    // write_len_ += 18 + key.size() + value.size();
                } else {
                    strcat(write_buf_, "search fail\r\n");
                    // write_len_ += 13;
                }
                strcat(write_buf_, key.c_str());
                strcat(write_buf_, "\r\n");
            }
            cnt--;
            key.resize(0);
            value.resize(0);
        } else if (read_buf_[index] == ':') {
            index++;
            flag = 1;
        } else if (read_buf_[index] == '.') {
            index++;
            flag = 2;
        } else {
            if (flag == 0)
                cmd = read_buf_[index];
            else if (flag == 1)
                key += read_buf_[index];
            else
                value += read_buf_[index];
            index++;
        }
    }
}
