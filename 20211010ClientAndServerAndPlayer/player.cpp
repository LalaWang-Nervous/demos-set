#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
using namespace std;

long long show_time() {
    struct timeval time;
    gettimeofday(&time,NULL);
    long long format_time = (long long)time.tv_sec * 1000 + (long long)time.tv_usec / 1000;
    cout << "Time is:" << format_time << endl;
    return format_time;
}

int main(int argc, char* argv[]) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1) {
        cout << "create listen socket fail." << endl;
        return -1;
    }

    // 2.init server address
    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);  // TODO: 用在这里干什么，转换地址和端口？
    bindaddr.sin_port = htons(4000);

    if(bind(listenfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) == -1) {
        cout << "bind listen socket fail." << endl;
    }

    if(listen(listenfd, SOMAXCONN) == -1) {
        cout << "listen error." << endl;
        return -1;
    }

    while(true) {
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);

        int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
        if(clientfd != -1) {
            char recvbuf[128] = {0};
            
            while(true) {
                int ret = recv(clientfd, recvbuf, 25, 0); //  = recvfrom(fd, buf, len, flags, NULL, 0);
                // With a zero flags argument, recv() is generally equivalent to read()
                if(ret > 0) {
                    show_time();
                    cout << "player receive data from server, data:" << recvbuf << endl;
                    cout << endl;
                }
            }
            
        }
        close(clientfd);
    }

    close(listenfd);

    return 0;
}