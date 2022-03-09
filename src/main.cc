#include <getopt.h>
#include <gperftools/profiler.h>
#include <signal.h>

#include <iostream>

#include "EventLoop.h"
#include "Log.h"
#include "LogFile.h"
#include "Server.h"

void setGperfStatus(int signum) {
    static bool is_open = false;
    if (signum != SIGUSR1) {
        return;
    }
    if (!is_open) {  // start
        is_open = true;
        ProfilerStart("test.prof");
        std::cout << "ProfilerStart success" << std::endl;
    } else {  // stop
        is_open = false;
        ProfilerStop();
        std::cout << "ProfilrerStop success" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    int thread_num = 0;
    int port = 4444;

    int opt;
    const char *str = "t:p:";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
            case 't': {
                thread_num = atoi(optarg);
                break;
            }
            case 'p': {
                port = atoi(optarg);
                break;
            }
            default:
                break;
        }
    }
    signal(SIGUSR1, setGperfStatus);
    // ProfilerStart("test.prof");

    LogFile log_file("/mnt/e/Work/DemoKV/ServerLog.txt");
    log_file.Start_Log();
    Log::log_file = &log_file;

    // EventLoop base_loop;
    Server kvServer(thread_num, port);
    kvServer.start();

    // ProfilerStop();

    return 0;
}
// std::cout << "Hello, world!\n"; }

/*
1+4
 36.5707
  28.1677

1
59.1296
58.3384
*/