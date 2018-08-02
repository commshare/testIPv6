sockaddr_in and sockaddr_in6 are both structures where first member is a sockaddr structure.

According to the C standard, the address of a structure and its first member are the same, so you can cast the pointer to sockaddr_in(6) in a pointer to sockaddr.

Functions taking sockaddr_in(6) as parameter may modify the sockaddr part, and functions taking sockaddr as parameter just care about that part.

down vote
accepted
I don't want to answer my question. But to give more information here which might be useful to other people, I decide to answer my question.

After dig into the source code of linux.

Following is my finding, there are possible multiple protocol which all implement the getsockname. And each have themself underling address data structure, for example, for IPv4 it is sockaddr_in, and IPV6 sockaddr_in6, and sockaddr_un for AF_UNIX socket. sockaddr are used as the common data strut in the signature of those APIs.

Those API will copy the socketaddr_in or sockaddr_in6 or sockaddr_un to sockaddr base on another parameter length by memcpy.

And all of the data structure begin with same type field sa_family.

Base on those reason, the code snippet is valid,
because both sockaddr_in and sockaddr_in6 have sa_family and then we can cast it to the correct data structure for usage after check sa_family.

BTY, I'm not sure why the sizeof(sockaddr_in6) > sizeof(sockaddr),
which cause allocate memory base on size of sockaddr is not enough for ipv6( that is error-prone), but I guess it is because of history reason.




首页新随笔管理
网络编程中常见地址结构与转换（IPv4/IPv6)
1. sockaddr/sockaddr_in/in_addr (IPv4)、sockaddr6_in/in6_addr/addrinfo (IPv6)

struct sockaddr {
      unsigned short sa_family;     　　　　/* address family, AF_xxx */
      char sa_data[14];                  　/* 14 bytes of protocol address */
};

struct sockaddr_in {
      short int sin_family;                /* Address family AF_INET */
      unsigned short int sin_port;    　　　/* Port number */
      struct in_addr sin_addr;         　　/* Internet address */
      unsigned char sin_zero[8];     　　　/* Same size as struct sockaddr */
};  <br>
struct in_addr {
      unsigned long s_addr;           　　/* Internet address */
};

struct sockaddr_in6 {
      sa_family_t sin6_family;         　　/* AF_INET6 */
      in_port_t sin6_port;                /* transport layer port # */
      uint32_t sin6_flowinfo;           　 /* IPv6 traffic class & flow info */
      struct in6_addr sin6_addr;    　　　 /* IPv6 address */
      uint32_t sin6_scope_id;        　　　/* set of interfaces for a scope */
};
struct in6_addr {
      uint8_t s6_addr[16];            　　/* IPv6 address */
};

struct addrinfo{
      int ai_flags;                         /* AI_PASSIVE,AI_CANONNAME,AI_NUMERICHOST */
      int ai_family;                        /* AF_INET,AF_INET6 */
      int ai_socktype;                   　　/* SOCK_STREAM,SOCK_DGRAM */
      int ai_protocol;                   　　/* IPPROTO_IP, IPPROTO_IPV4, IPPROTO_IPV6 */
      size_t ai_addrlen;               　　　/* Length */
      char *ai_cannoname;         　　　　　　/* */
      struct sockaddr *ai_addr;  　　　　　　 /* struct sockaddr */
      struct addrinfo *ai_next;     　　　　 /* pNext */
}
2. 与IP地址相关的常用网络编程函数

2.1 地址转化函数

IPv4中，可使用inet_ntoa/inet_aton来转化字符串形式表示的IPv4地址和数字形式表示的IPv4地址。此两函数不适用于IPv6地址转换。在Linux环境下使用inet_ntoa/inet_atoa时，需加头文件：

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
函数原型：

int inet_aton(const char * cp,struct in_addr *inp);
char * inet_ntoa(struct in_addr in);
举例:

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main(int aargc, char* argv[])
{
    struct in_addr addr1;
    ulong l1;
    l1 = inet_addr("192.168.0.74");
    memcpy(&addr1, &l1, sizeof(l1));
    printf("%s\n", inet_ntoa(addr1));
　　 if(inet_aton("127.0.0.1", &addr1)){
　　　　printf("inet_aton:ip=%lu\n",ntohl(inp.s_addr));
　　 }
　　 else{
　　　　printf("inet_aton return -1 when 255.255.255.255\n");
　　 }
　　 return 0;
}
IPv6中，使用inet_ntop/inet_pton来转化字符串形式表示的IPv6地址和数字形式表示的IPv6地址。IPv4中也可使用这两个函数。

函数原型：

int inet_pton(int af, const char *src, void *dst);         <br>//这个函数转换字符串到网络地址，第一个参数af是地址族，转换后存在dst中 af的值可为AF_INET (代表使用IPv4协议）或AF_INET6（代表作用IPv6协议）

const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt);
//这个函数转换网络二进制结构到ASCII类型的地址，参数的作用和上面相同，只是多了一个参数socklen_t cnt,
//他是所指向缓存区dst的大小，避免溢出，如果缓存区太小无法存储地址的值，则返回一个空指针，并将errno置为ENOSPC
 举例：

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    unsigned char buf[sizeof(struct in6_addr)];
    int domain, s;
    char str[INET6_ADDRSTRLEN];

    if(argc != 3){
        fprintf(stderr, "usage: %s {i4|i6|<num>} string/n", argv[0]);
        exit(EXIT_FAILURE);
    }

    domain = (strcmp(argv[1], "i4") == 0) ? AF_INET:(strcmp(argv[1], "i6") == 0) ? AF_INET6 : atoi(argv[1]);

    //IP字符串 ——》网络字节流
    s = inet_pton(domain, argv[2], buf);
    if(s<=0){
        if(0 == s)
            fprintf(stderr, "Not in presentation format/n");
        else
            perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    //网络字节流 ——》IP字符串
    if(inet_ntop(domain, buf, str, INET6_ADDRSTRLEN) == NULL){
        perror("inet ntop/n");
        exit(EXIT_FAILURE);
    }

    printf("%s/n", str);
    exit(EXIT_SUCCESS);
}
