#ifndef _LOGFILE
#define _LOGFILE
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <queue>
#include <string>
using namespace std;
typedef function<void()> LogFun;
void* LogSwapStart(void*);
void* LogWriteStart(void*);
class LogFile {
public:
    LogFile(string);
    queue<char*> WriteTask;
    queue<char*> Wait_WriteTask;
    void put_write_string(char*);
    void Start_Log();
    void write_file();
    void swap();

private:
    pthread_mutex_t put_Mutex;
    pthread_mutex_t get_Mutex;
    pthread_cond_t get_Cond;
    pthread_cond_t Swap_Cond;
    int fd;
};
#endif