#include <getopt.h>

#include <iostream>

#include "EventLoop.h"
#include "Log.h"
#include "LogFile.h"
#include "Server.h"

int main(int argc, char *argv[]) {
    int thread_num = 4;
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

    LogFile log_file("/mnt/e/Work/DemoKV/ServerLog.txt");
    log_file.Start_Log();
    Log::log_file = &log_file;

    // EventLoop base_loop;
    Server kvServer(thread_num, port);
    kvServer.start();

    return 0;
}
// std::cout << "Hello, world!\n"; }
