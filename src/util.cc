#include "util.h"

int readn(int fd, char* buf, int max_length) {
    int read_sum = 0;
    while (max_length > 0) {
        int nread = read(fd, buf, max_length);
        if (nread < 0) {
            //读完
            if (errno == EAGAIN) {
                return read_sum;
            }
            //慢系统调用阻塞
            else if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        } else if (nread > 0) {
            max_length -= nread;
            read_sum += nread;
            buf += nread;
        } else {
            return 0;
        }
    }

    return read_sum;
}

int writen(int fd, char* buf, int length) {
    // std::cout << buf << '\n';
    int write_sum = 0;
    while (length > 0) {
        int nwrite = write(fd, buf, length);
        if (nwrite < 0) {
            if (errno == EAGAIN) {
                return write_sum;
            } else if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }
        write_sum += nwrite;
        length -= nwrite;
        buf += nwrite;
    }
    return write_sum;
}

pid_t gettid_() { return static_cast<pid_t>(::syscall(SYS_gettid)); }