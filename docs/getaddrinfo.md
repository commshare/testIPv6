socket编程之addrinfo结构体与getaddrinfo函数
2013年04月23日 23:05:32
阅读数：6875
(https://blog.csdn.net/DLUTBruceZhang/article/details/8842344)

1. 概述

IPv4中使用gethostbyname()函数完成主机名到地址解析，这个函数仅仅支持IPv4，且不允许调用者指定所需地址类型的任何信息，返回的结构只包含了用于存储IPv4地址的空间。IPv6中引入了getaddrinfo()的新API，它是协议无关的，既可用于IPv4也可用于IPv6。getaddrinfo函数能够处理名字到地址以及服务到端口这两种转换，返回的是一个addrinfo的结构（列表）指针而不是一个地址清单。这些addrinfo结构随后可由套接口函数直接使用。如此以来，getaddrinfo函数把协议相关性安全隐藏在这个库函数内部。应用程序只要处理由getaddrinfo函数填写的套接口地址结构。该函数在 POSIX规范中定义了。


2. 函数说明

包含头文件
#include<netdb.h>

函数原型
int getaddrinfo( const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result );

参数说明
hostname:一个主机名或者地址串(IPv4的点分十进制串或者IPv6的16进制串)
service：服务名可以是十进制的端口号，也可以是已定义的服务名称，如ftp、http等
hints：可以是一个空指针，也可以是一个指向某个addrinfo结构体的指针，调用者在这个结构中填入关于期望返回的信息类型的暗示。举例来说：如果指定的服务既支持TCP也支持UDP，那么调用者可以把hints结构中的ai_socktype成员设置成SOCK_DGRAM使得返回的仅仅是适用于数据报套接口的信息。
result：本函数通过result指针参数返回一个指向addrinfo结构体链表的指针。
返回值：0——成功，非0——出错


3. 参数设置

在getaddrinfo函数之前通常需要对以下6个参数进行以下设置：nodename、servname、hints的ai_flags、ai_family、ai_socktype、ai_protocol。
在6项参数中，对函数影响最大的是nodename，sername和hints.ai_flag，而ai_family只是有地址为v4地址或v6地址的区别。ai_protocol一般是为0不作改动。

getaddrinfo在实际使用中的几种常用参数设置
一般情况下，client/server编程中，server端调用bind（如果面向连接的还需要listen），client则不用掉bind函数，解析地址后直接connect（面向连接）或直接发送数据（无连接）。因此，比较常见的情况有
（1）    通常服务器端在调用getaddrinfo之前，ai_flags设置AI_PASSIVE，用于bind；主机名nodename通常会设置为NULL，返回通配地址[::]。
（2）    客户端调用getaddrinfo时，ai_flags一般不设置AI_PASSIVE，但是主机名nodename和服务名servname（更愿意称之为端口）则应该不为空。
（3）    当然，即使不设置AI_PASSIVE，取出的地址也并非不可以被bind，很多程序中ai_flags直接设置为0，即3个标志位都不设置，这种情况下只要hostname和servname设置的没有问题就可以正确bind。

上述情况只是简单的client/server中的使用，但实际在使用getaddrinfo和参考国外开源代码的时候，曾遇到一些将servname（即端口）设为NULL的情况(当然，此时nodename必不为NULL，否则调用getaddrinfo会报错)。
以下分情况进行了测试：
（1）    如果nodename是字符串型的IPv6地址，bind的时候会分配临时端口；
（2）    如果nodename是本机名，servname为NULL，则根据操作系统的不同略有不同，本文仅在WinXP和Win2003上作了测试。
        a)    WinXP系统（SP2）返回loopback地址[::1]
        b)    Win2003则将本机的所有IPv6地址列表加以返回。因为通常一台IPv6主机都有可能不止一个IPv6地址，比如fe80::1（本机 loopback地址）、fe80::***的Link-Local地址、3ffe:***的全局地址等等。这种情况下调用getaddrinfo会将这些地址全部返回，调用者应该注意如何使用这些地址。另外要注意的是，对于fe80::的地址在绑定的时候必须标明接口地址，即使用 fe80::20d:60ff:fe78:51c2%4或fe80::1%1这样的地址格式，通过getaddrinfo直接取出fe80地址好像无法直接bind。
        
        
4. 使用细节

如果本函数返回成功，那么由result参数指向的变量已被填入一个指针，它指向的是由其中的ai_next成员串联起来的addrinfo结构链表。可以导致返回多个addrinfo结构的情形有以下2个：
    1.    如果与hostname参数关联的地址有多个，那么适用于所请求地址簇的每个地址都返回一个对应的结构。
    2.    如果service参数指定的服务支持多个套接口类型，那么每个套接口类型都可能返回一个对应的结构，具体取决于hints结构的ai_socktype成员。

我们必须先分配一个hints结构，把它清零后填写需要的字段，再调用getaddrinfo，然后遍历一个链表逐个尝试每个返回地址。
        
getaddrinfo解决了把主机名和服务名转换成套接口地址结构的问题。

其中，如果getaddrinfo出错，那么返回一个非0的错误值。
#include<netdb.h>
const char *gai_strerror( int error );
该函数以getaddrinfo返回的非0错误值的名字和含义为他的唯一参数，返回一个指向对应的出错信息串的指针。

由getaddrinfo返回的所有存储空间都是动态获取的，这些存储空间必须通过调用freeaddrinfo返回给系统。
#include< netdb.h >
void freeaddrinfo( struct addrinfo *ai );
ai参数应指向由getaddrinfo返回的第一个addrinfo结构。这个连表中的所有结构以及它们指向的任何动态存储空间都被释放掉。



addrinfo结构体的定义如下：


struct addrinfo {
     int ai_flags; /* customize behavior */
     int ai_family; /* address family */
     int ai_socktype; /* socket type */
     int ai_protocol; /* protocol */
     socklen_t ai_addrlen; /* length in bytes of address */
     struct sockaddr *ai_addr; /* address */
     char *ai_canonname; /* canonical name of host */
     struct addrinfo *ai_next; /* next in list */
     .
     .
     .
   };

ai_family指定了地址族，可取值如下： 
AF_INET          2            IPv4 
AF_INET6        23            IPv6 
AF_UNSPEC        0            协议无关
ai_socktype指定我套接字的类型 
SOCK_STREAM        1            流 
SOCK_DGRAM        2            数据报

在AF_INET通信域中套接字类型SOCK_STREAM的默认协议是TCP（传输控制协议）
在AF_INET通信域中套接字类型SOCK_DGRAM的默认协议是UDP（用户数据报协议）

ai_protocol指定协议类型。可取的值取决于ai_address和ai_socktype的值

ai_flags指定了如何来处理地址和名字,可取值如下：

QQ截图20120409193145

getaddrinfo函数 定义及需要的头文件如下：

#include <sys/socket.h>
#include <netdb.h>
 
int getaddrinfo(const char *restrict host,
                const char *restrict service,
                const struct addrinfo *restrict hint,
                struct addrinfo **restrict res);
 
 
 
 
Returns: 0 if OK, nonzero error code on error
 
 
void freeaddrinfo(struct addrinfo *ai);

getaddrinfo函数允许将一个主机名字和服务名字映射到一个地址。


实例：


#include<stdio.h>
#include<stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
 
void
print_family(struct addrinfo *aip)
{
    printf(" family ");
    switch (aip->ai_family) {
    case AF_INET:
        printf("inet");
        break;
    case AF_INET6:
        printf("inet6");
        break;
    case AF_UNIX:
        printf("unix");
        break;
    case AF_UNSPEC:
        printf("unspecified");
        break;
    default:
        printf("unknown");
    }
}
 
void
print_type(struct addrinfo *aip)
{
    printf(" type ");
    switch (aip->ai_socktype) {
    case SOCK_STREAM:
        printf("stream");
        break;
    case SOCK_DGRAM:
        printf("datagram");
        break;
    case SOCK_SEQPACKET:
        printf("seqpacket");
        break;
    case SOCK_RAW:
        printf("raw");
        break;
    default:
        printf("unknown (%d)", aip->ai_socktype);
    }
}
 
void
print_protocol(struct addrinfo *aip)
{
    printf(" protocol ");
    switch (aip->ai_protocol) {
    case 0:
        printf("default");
        break;
    case IPPROTO_TCP:
        printf("TCP");
        break;
    case IPPROTO_UDP:
        printf("UDP");
        break;
    case IPPROTO_RAW:
        printf("raw");
        break;
    default:
        printf("unknown (%d)", aip->ai_protocol);
    }
}
 
void
print_flags(struct addrinfo *aip)
{
    printf("flags");
    if (aip->ai_flags == 0) {
        printf(" 0");
    } else {
        if (aip->ai_flags & AI_PASSIVE)
            printf(" passive");
        if (aip->ai_flags & AI_CANONNAME)
            printf(" canon");
        if (aip->ai_flags & AI_NUMERICHOST)
            printf(" numhost");
#if defined(AI_NUMERICSERV)
        if (aip->ai_flags & AI_NUMERICSERV)
            printf(" numserv");
#endif
#if defined(AI_V4MAPPED)
        if (aip->ai_flags & AI_V4MAPPED)
            printf(" v4mapped");
#endif
#if defined(AI_ALL)
        if (aip->ai_flags & AI_ALL)
            printf(" all");
#endif
    }
}
 
int
main(int argc, char *argv[])
{
    struct addrinfo        *ailist, *aip;
    struct addrinfo        hint;
    struct sockaddr_in    *sinp;
    const char             *addr;
    int                 err;
    char                 abuf[INET_ADDRSTRLEN];
 
    if (argc != 3)
        printf("usage: %s nodename service", argv[0]);
    hint.ai_flags = AI_CANONNAME;
    hint.ai_family = 0;
    hint.ai_socktype = 0;
    hint.ai_protocol = 0;
    hint.ai_addrlen = 0;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;
    if ((err = getaddrinfo(argv[1], argv[2], &hint, &ailist)) != 0)
        printf("getaddrinfo error: %s", gai_strerror(err));
    for (aip = ailist; aip != NULL; aip = aip->ai_next) {
        print_flags(aip);
        print_family(aip);
        print_type(aip);
        print_protocol(aip);
        printf("\n\thost %s", aip->ai_canonname?aip->ai_canonname:"-");
        if (aip->ai_family == AF_INET) {
            sinp = (struct sockaddr_in *)aip->ai_addr;
            addr = inet_ntop(AF_INET, &sinp->sin_addr, abuf,INET_ADDRSTRLEN);
            printf(" address %s", addr?addr:"unknown");
            printf(" port %d", ntohs(sinp->sin_port));
        }
        printf("\n");
    }
    exit(0);
}

代码说明：sinp = (struct sockaddr_in *)aip->ai_addr;会将struct sockaddr 变量强制转化为struct sockaddr_in 类型
inet_ntop函数用于在二进制格式与点分十进制格式表示（a.b.c.d）之间进行转换


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
int main(int argc, char **argv)
{
if (argc != 2) {
fprintf(stderr, "Usage: %s hostname\n",
argv[1]);
exit(1);   
}
 
struct addrinfo *answer, hint, *curr;
char ipstr[16];   
bzero(&hint, sizeof(hint));
hint.ai_family = AF_INET;
hint.ai_socktype = SOCK_STREAM;
 
int ret = getaddrinfo(argv[1], NULL, &hint, &answer);
if (ret != 0) {
fprintf(stderr,"getaddrinfo: &s\n",
gai_strerror(ret));
exit(1);
}
 
for (curr = answer; curr != NULL; curr = curr->ai_next) {
inet_ntop(AF_INET,
&(((struct sockaddr_in *)(curr->ai_addr))->sin_addr),
ipstr, 16);
printf("%s\n", ipstr);
}
 
freeaddrinfo(answer);
exit(0);
}


版权声明：本文为博主原创文章，未经博主允许不得转载。	https://blog.csdn.net/DLUTBruceZhang/article/details/8842344