#include <getopt.h>
#include <rpc/types.h>
#include <signal.h>
#include <strings.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>  //unix标准文件
#include <unistd.h>

#include <chrono>
#include <random>
#include <vector>

#include "iostream"
#include "socket.cc"
using namespace std;
int clients = 100;
const char* host = "127.0.0.1";
int port = 4444;
int mypipe[2];

void benchcore();
int putbench(char** cmds);
int total = 1000000 / clients;
int ran = 50000000;

int main() {
    pid_t pid;
    FILE* f;
    int ret = -1;

    // vector<string> cmds(ran);
    char** cmds = new char*[ran];
    for (int i = 0; i < ran; i++) {
        char* str = new char[125];
        memset(str, 0, 128);
        strcat(str,
               ("1\r\np:" + to_string(i) + ".mydatamydatavalue\r\n\0").c_str());
        cmds[i] = str;
        // cmds[i] = "1\r\np:" + to_string(i) + ".mydatamydatavalue\r\n\0";
    }
    // cout << cmds.size() << '\n';

    int status = 0;
    if (pipe(mypipe)) {
        perror("pipe failed.");
        return 3;
    }
    for (int i = 0; i < clients; i++) {
        pid = fork();
        if (pid <= (pid_t)0) {
            sleep(1);
            break;
        }
    }
    if (pid == -1)  //错误创建
    {
        perror("fork error");
        _exit(1);
    } else if (pid == 0) {
        // cout << "start bench" << '\n';
        auto start = std::chrono::high_resolution_clock::now();
        int suc = putbench(cmds);
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;
        // cout << "end bench" << '\n';
        // std::cout << "insert elapsed:" << elapsed.count() << std::endl;
        f = fdopen(mypipe[1], "w");
        if (f == NULL) {
            perror("open pipe for writing failed.");
            return 3;
        }
        /* fprintf(stderr,"Child - %d %d\n",speed,failed); */
        fprintf(f, "%lf %d\n", elapsed.count(), suc);
        fclose(f);
        // cout << "In child time-----: " << elapsed.count()
        //      << "   success:" << suc << "/total:" << total << '\n';

        return 0;
    } else {
        f = fdopen(mypipe[0], "r");
        if (f == NULL) {
            perror("open pipe for reading failed.");
            return 3;
        }

        setvbuf(f, NULL, _IONBF, 0);
        double gap = 0.0;
        int cnt = 0;
        srand(time(NULL));
        int xxx = clients;
        auto start = std::chrono::high_resolution_clock::now();
        while (1) {
            double i;
            int j;
            pid = fscanf(f, "%lf %d", &i, &j);
            if (pid < 2) {
                fprintf(stderr, "Some of our childrens died.\n");
                break;
            }

            gap += i;
            cnt += j;

            /* fprintf(stderr,"*Knock* %d %d read=%d\n",speed,failed,pid);
             */
            if (--xxx == 0) break;
        }
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;

        fclose(f);
        ret = wait(&status);
        if (-1 == ret) {
            perror("wait");
            return 1;
        }
        cout << "-------total time: " << elapsed.count() << "  success:" << cnt
             << "/total:" << clients * total << '\n';
        return 0;
    }
    return 0;
}

int putbench(char** cmds) {
    // char req[1024];
    char buf[1024];
    int s = Socket(host, port);
    int succcnt = 0;
    std::random_device rd;  // A function object for generating seeds
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, ran - total - 1);  //
    int start = dis(gen);
    // cout << start << '\n';
    for (int i = start; i < total + start; i++) {
        // cout << "write::" << i << "--" << req;
        //   usleep(300000);
        // cout << cmds[i] << '\n';
        int rlen = strlen(cmds[i]);
        if (rlen != write(s, cmds[i], rlen)) {
            // cout << "write failed" << req << "\n";
            close(s);
        }
        int len = read(s, buf, 1500);
        if (len < 0) {
            close(s);
        }
        // req[0] = 0;
        buf[len] = 0;
        if (buf[4] == 's') succcnt++;
        // cout << "read"
        //      << "----" << buf << "----\n";
    }
    // cout << "start: " << start << "/ end: " << total + start;
    //  cout << "success:" << succcnt << "/total:" << total << '\n';
    //  cout << "close s" << '\n';
    close(s);

    return succcnt;
}
