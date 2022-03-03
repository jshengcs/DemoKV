#ifndef UTIL_H_
#define UTIL_H_

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <memory>

int readn(int fd, char* buf, int max_length);
int writen(int fd, char* buf, int length);
pid_t gettid_();

#endif