#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#define LOG (printf("(%d) - <%s> ",/*__FILE__,*/__LINE__,__FUNCTION__),printf)


#define MAXDATASIZE 100 // max number of bytes we can get at once
int main()
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

#if 0

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; //这里如果没有强制的话，可能会是ipv4的哦
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host,TCP_SERVER_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can 连接到第一个
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if(p->ai_family == AF_INET6){
            LOG("--V6 \n");
        }else{
            LOG("--V4 \n");
        }
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket fail ");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect fail");
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

#endif
/*
 * 本地发给本地(macos , 服务器和客户端都强制用v6)

 服务端的打印：
(129) - <main>  bind sockfd 3 OK
(25) - <get_in_addr> this sockaddr is V6
selectserver: new connection from ::1 on socket 4
selectserver: socket 4 hung up

增加服务端发送msg到客户端之后：
 bogon:ref lijin$ ./tcp_loopback_client
address created 3
server connected/nclient: received 'loopbacksocketsendMsgToClient'

 * */
    struct sockaddr_in6 addr;
    bzero(&addr,sizeof(addr));
    addr.sin6_len = sizeof(struct sockaddr_in6); //TODO 这个对不对呢？
    addr.sin6_family = AF_INET6;
    char *port = "9024";
    addr.sin6_port = htons(atoi(port));
    addr.sin6_addr = in6addr_loopback;
    sockfd = socket(PF_INET6,SOCK_STREAM,IPPROTO_TCP);
    if(sockfd < 0 ){
        LOG("SOCKET FAIL %d  %s \n",sockfd,strerror(errno));
    }
    printf("address created %d \n",sockfd);

    /*
     int    connect(int, const struct sockaddr *, socklen_t) __DARWIN_ALIAS_C(connect);
     */
    int ret = connect(sockfd,(struct sockaddr *)&addr,sizeof(struct sockaddr_in6));
    if(ret  != 0 ){
        LOG(" CONNCET FAIL %d %s \n",ret,strerror(errno));
    }
    printf("server connected/n");

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);

    return 0;
}


