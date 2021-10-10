#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
using namespace std;

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 4000

long long show_time() {
    struct timeval time;
    gettimeofday(&time,NULL);
    long long format_time = (long long)time.tv_sec * 1000 + (long long)time.tv_usec / 1000;
    cout << "Time is:" << format_time << endl;
    return format_time;
}

int main(int argc, char* argv[]) {
    // 1.create socket
    /* SYNOPSIS:
       int socket(int domain, int type, int protocol);
       socket()  creates an endpoint for communication and 
       returns a file descriptor that refers to that endpoint.  The file descriptor returned by
       a successful call will be the lowest-numbered file descriptor not currently open for the process.

       @domain:
        The domain argument specifies a communication domain; 
        this selects the protocol family which will be used for communication.
        包括 AF_INET(IPv4),AF_INET6(IPv6)等等，代表底层协议选择，制定一个family
       @type
        The socket has the indicated type, which specifies the communication semantics.
        表示通信的类型：
        SOCK_STREAM: Provides  sequenced,  reliable,  two-way,  connection-based byte streams.  
        An out-of-band data transmission mechanism may be supported. 表示双向的可靠、基于连接的的字节流
        还有各种SOCK_XXX等等, 在所有的这些类型中，只有基于connection的，之后才会有accept操作
       @protocol
        The protocol specifies a particular protocol to be used with the socket.  
        Normally only a single protocol exists to support a particular socket type within a given protocol family, 
        in which case protocol can be specified as 0. 一般就写0.

     */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1) {
        cout << "create listen socket fail." << endl;
        return -1;
    }

    // 2.init server address
    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    /*
       htonl, htons, ntohl, ntohs - convert values between host and network byte order
       网络序和主机序之间的转换
       The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.
       The htons() function converts the unsigned short integer hostshort from host byte order to network byte order.
       The ntohl() function converts the unsigned integer netlong from network byte order to host byte order.
       The ntohs() function converts the unsigned short integer netshort from network byte order to host byte order.
       注意观察这四个函数名的结构:
       h - host，主机序
       to - 转换
       n - net，网络序
       l - unsigned integer
       s - short integer
       并且转换是线程安全
     */
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);  // TODO: 用在这里干什么，转换地址和端口？
    bindaddr.sin_port = htons(3000);
    /*
       bind - bind a name to a socket
       SYNOPSIS:int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

       When  a  socket is created with socket(2), it exists in a name space (address family) 
       but has no address assigned to it.  bind() assigns the address specified by addr to 
       the socket referred to by the file descriptor sockfd.  
       addrlen specifies the size, in  bytes,  of  the  address structure pointed to by addr.  
       Traditionally, this operation is called “assigning a name to a socket”.
       在用socket()函数后，生成了socketfd这个文件描述符来指向新创建的socket，但是此时还没有地址和它关联
       bind()函数就是将socketfd和地址关联起来，因此也称为为socket命名？不同的地址家族bind的细节还有所不同

       @listenfd: 绑定地址的目标文件描述符，由socket()创建
       @bindaddr: 不同address family这个结构体的具体内容不一样The only purpose of this structure is 
       to cast the structure pointer passed in addr in order to avoid compiler warnings. 

       It is normally necessary to assign a local  address  using  bind()  before  a
       SOCK_STREAM socket may receive connections. 一般来讲是绑定一个本地地址
     */
    if(bind(listenfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) == -1) {
        cout << "bind listen socket fail." << endl;
    }

    // 3.start listen
    /*
       SYNOPSIS: int listen(int sockfd, int backlog);
       listen() marks the socket referred to by sockfd as a passive socket, that is, 
       as a socket that will be used to accept incoming connection requests using accept(2).
       listen()函数将传入的那个socketfd所指向的socket标记为一个被动socket，表示随时会用于accept到来的连接请求
       
       @sockfd
       The sockfd argument is a file descriptor that refers to a socket of type SOCK_STREAM or SOCK_SEQPACKET.
       @socklog
       The backlog argument defines the maximum length to which the queue of pending connections for sockfd 
       may grow.  If a connection request arrives when the queue is full, the client may receive an error
       with an indication of ECONNREFUSED or, if the underlying protocol supports retransmission, 
       the request may be ignored so that a later reattempt at connection succeeds.
       backlog设置为这个socketfd所管理的等待连接处理的请求队列的最大长度

       注意：
       To accept connections, the following steps are performed:
           1.  A socket is created with socket(2).
           2.  The socket is bound to a local address using bind(2), 
               so that other sockets may be connect(2)ed to it.
               就进行socket连接而言，连接两头的主体都是socket对象
           3.  A willingness to accept incoming connections and 
               a queue limit for incoming connections are specified with listen().
               listen()表示传入的那个socket对象，从这条语句之后做好了去accept即将到来请求的准备
               同时规定了对于这种潜在连接请求的候选队列的长度限制
           4.  Connections are accepted with accept(2).
               真正执行和到来请求进行连接操作的是accept()
        backlog specifies the queue length for *completely* established
        sockets waiting to be accepted, instead of the number of *incomplete* connection requests.  
        The maximum length of the queue for *incomplete* sockets can be set using /proc/sys/net/ipv4/tcp_max_syn_backlog.  
        When syncookies are enabled there is no logical maximum length and this setting is ignored.
        注意这里对于来请求连接的其他socket还有一个complete established和incopmlete上的区分
        TODO: 或许这里和TCP三次握手有关，complete和incomplete指的是握手状态？
        If the backlog argument is greater than the value in /proc/sys/net/core/somaxconn, 
        somaxconn是一个内核关于TCP的参数，该参数定义了系统中每一个端口最大的监听队列的长度
        then it is silently truncated to that value. Since Linux5.4, the default in this file is 4096;


     */
    if(listen(listenfd, SOMAXCONN) == -1) {
        cout << "listen error." << endl;
        return -1;
    }

    // 连接播放器
    // 1.create client socket
    int self_clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(self_clientfd == -1) {
        cout << "server create client socket fail." << endl;
        return -1;
    }

    // 2.connect server
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serveraddr.sin_port = htons(SERVER_PORT);
    if(connect(self_clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        cout << "serer connect player socket fail." << endl;
        return -1;
    }

    // 等待client发数据过来
    while(true) {
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);
        // 4.accept client
        /*
          SYNOPSIS:int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
          The accept() system call is used with connection-based socket types (SOCK_STREAM, SOCK_SEQPACKET).
          传入的listenfd必须是基于连接的套接字  
          It extracts the first connection request on the queue of pending connections for the listening socket, 
          sockfd, creates a new connected socket, and returns a new file descriptor referring to that socket.
          创建一个新的套接字去处理监听端口上待处理连接队列的第一个
          The newly created socket is not in the listening state. 
          新创建的这个套接字并非出于监听态，一个套接字只有listen()之后才处于监听态 
          The original socket sockfd is unaffected by this call.

          @listenfd: 这就是和监听本地端口、处于监听状态的套接字对应的文件描述符
          @addr: The argument addr is a pointer to a sockaddr structure.  
          This structure is filled in with the address of the peer socket, 
          as  known  to  the communications layer.  The exact format of the address returned addr 
          is determined by the socket's address family.  
          When addr is NULL, nothing is filled in; 
          in this case, addrlen is not used, and should also be NULL.
          这个结构体传入的时候可以是空的，函数返回时，这个结构体会被填充具体的值
          @addrlen: The addrlen argument is a value-result argument: 
          the caller must initialize it to contain the size (in bytes) of the structure pointed to by addr; 
          on return it will contain the actual size of the peer address.
          addrlen返回时也会变成真实的peer address的size
          
          The  returned  address is truncated if the buffer provided is too small; 
          in this case, addrlen will return a value greater than was supplied to the call.
          On success, these system calls return a file descriptor for the accepted socket (a nonnegative integer). 
          注意，返回的这个socketfd，实际上就是对连接另一头的socket对象的描述了

          If no pending connections are present on the queue, and the socket is not marked as nonblocking, 
          accept() blocks the caller until a connection is present.  
          默认情况下，accept是阻塞的，如果没有连接处理，当前进程就block了，直到有连接来
          If the socket is marked nonblocking and no pending connections are present on the queue, 
          accept() fails with the error EAGAIN or EWOULDBLOCK.
          如果监听端口绑定的socket对象被标记为非阻塞的，那么没有可处理连接时，accept直接返回错误
          
          In order to be notified of incoming connections on a socket, you can use select(2), poll(2), or epoll(7).
          可以配合使用select/poll/epoll来处理socket事件  
          A readable event will  be  delivered  when a new connection is attempted 
          and you may then call accept() to get a socket for that connection.  
          Alternatively, you can set the socket to deliver SIGIO when activity occurs on a socket;

          There  may  not  always be a connection waiting 
          after a SIGIO is delivered or select(2), poll(2), or epoll(7) return a readability event 
          because the connection might have been removed by an asynchronous network error or another thread 
          before accept() is called.  
          If this happens, then the call will block waiting for the next connection to arrive.
          但即便是使用一些事件驱动的思想去处理socket事件，但是依旧当此类信号到达时，一定会有一个带处理的连接
          因此此类处理逻辑都是：1. 收到一些信息，表征有新连接来 2. 调用accept()去处理
          但是在1、2之间，可能连接就断了，或者各种故障，那么accept()还是会block住，直达连接请求到来
         */

        int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
        if(clientfd != -1) {
            char recvbuf[128] = {0};
            // 5.start to receive data from client.
            /*
              return the length of the message on successful completion.  
              If a message is too long to fit in the supplied  buffer,  excess bytes may be discarded 
              depending on the type of socket the message is received from.
              如果受到的数据体积超过缓存体积，超出的部分的丢弃策略受对方套接字类型影响。 TODO:具体啥影响？
              
              If no messages are available at the socket, the receive calls wait for a message to arrive, 
              unless the socket is nonblocking (see fcntl(2)),
              当socket是阻塞类型时，recv会一直阻塞直到有数据来，
              若是非阻塞情况，则直接返回-1
              in which case the value -1 is returned and the external variable errno is set to EAGAIN or EWOULDBLOCK.  
              The receive calls  normally  return any data available, up to the requested amount, 
              rather than waiting for receipt of the full amount requested.
              An application can use select(2), poll(2), or epoll(7) to determine when more data arrives on a socket.

             */
            
            while(true) {
                int ret = recv(clientfd, recvbuf, 25, 0); //  = recvfrom(fd, buf, len, flags, NULL, 0);
                // With a zero flags argument, recv() is generally equivalent to read()
                if(ret > 0) {
                    show_time();
                    cout << "receive data from client, data:" << recvbuf << endl;
                    // 转发数据
                    long long time = show_time();
                    cout << "begin to send message to player:" << endl;
                    ret = send(self_clientfd, recvbuf, strlen(recvbuf), 0);
                    if(ret != strlen(recvbuf)) {
                        cout << "server send data to player fail." << endl;
                        return -1;
                    } else {
                        cout << "server send data to player successfully, data:" << recvbuf << endl;
                    }
                    cout << endl;
                }
            }
            
        }
        close(clientfd);
    }

    close(listenfd);

    return 0;
}