#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string.h>
using namespace std;

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 3000
#define SEND_DATA "Hello, chenfei!"

long long show_time() {
    struct timeval time;
    gettimeofday(&time,NULL);
    long long format_time = (long long)time.tv_sec * 1000 + (long long)time.tv_usec / 1000;
    cout << "Time is:" << format_time << endl;
    return format_time;
}

string lltoString(long long timestamp) {
    string result;
    ostringstream ss;
    ss<<timestamp;
    istringstream is(ss.str());
    is>>result;
    return result;
}

int main(int argc, char* argv[]) {
    // 1.create client socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd == -1) {
        cout << "create client socket fail." << endl;
        return -1;
    }

    // 2.connect server
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serveraddr.sin_port = htons(SERVER_PORT);
    /*
      SYNOPSIS: int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

      The connect() system call connects 
      the socket referred to by the file descriptor sockfd 
      to 
      the address specified by addr.  
      The addrlen argument specifies the size of addr. The format of the address in addr is determined by 
      the address space of the socket sockfd;
      connect()将由scokfd指定的套接字和由addr指定的地址去连接起来
      If the connection or binding succeeds, zero is returned.  
      On error, -1 is returned, and errno is set appropriately.
     */
    if(connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        cout << "connect socket fail." << endl;
        return -1;
    }

    // 3.send server data
    while(true) {
        cout << "begin to send message:" << endl;
        for(int i=0;i<5;i++) {
            long long time = show_time();
            string sendbuf = "THIS IS MESSAGE FOR TEST.";
            int ret = send(clientfd, sendbuf.c_str(), strlen(sendbuf.c_str()), 0);
            if(ret != strlen(sendbuf.c_str())) {
                cout << "send data fail." << endl;
                return -1;
            } else {
                cout << "send data successfully, data:" << sendbuf << endl;
            }
            cout << endl;
        }
        
        // usleep(5000000);

        sleep(5);
    }
    
    // 5.close socket
    close(clientfd);

    return 0;
}