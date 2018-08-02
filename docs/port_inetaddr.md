#inet_addr
()的功能是将一个点分十进制的IP转换成一个长整数型数（u_long类型）
将字符串形式的IP地址转换为按网络字节顺序的整型值

原型：in_addr_t inet_addr(const char *cp);
返回值：
如果正确执行将返回一个无符号长整数型数。如果传入的字符串不是一个合法的IP地址，将返回INADDR_NONE。


#  inet_pton
inet_pton：将“点分十进制” －> “二进制整数”
int inet_pton(int af, const char *src, void *dst);

这个函数转换字符串到网络地址，第一个参数af是地址簇，第二个参数*src是来源地址，第三个参数* dst接收转换后的数据。


inet_pton 是inet_addr的扩展，支持的多地址族有下列：
af = AF_INET
src为指向字符型的地址，即ASCII的地址的首地址（ddd.ddd.ddd.ddd格式的），函数将该地址转换为in_addr的结构体，并复制在*dst中。

af = AF_INET6
src为指向IPV6的地址，函数将该地址转换为in6_addr的结构体，并复制在*dst中。

如果函数出错将返回一个负值，并将errno设置为EAFNOSUPPORT，如果参数af指定的地址族和src格式不对，函数将返回0。

#
(https://stackoverrun.com/cn/q/10572776/ios%E5%A5%97%E6%8E%A5%E5%AD%97ipv6%E6%94%AF%E6%8C%81)

其实我保存在一个私有变量，m_nIP长数据类型的IP（address.sin_addr.s_addr）。
问题是我不需要IPv6，因为我们的整个产品组都使用IPv4。下面的代码解决了这个问题。


const uint8_t *bytes = ((const struct sockaddr_in6 *)addrPtr)->sin6_addr.s6_addr; 
bytes += 12; 
struct in_addr addr = { *(const in_addr_t *)bytes }; 
m_nIP = ntohl(addr.s_addr); 


IP地址转换  https://github.com/SeanXP/UNP/blob/master/unp/inet_pton/readme.md
====

## 过时API（只能处理IPv4地址，参数简单）

    #include <arpa/inet.h>

    int inet_aton(const char *cp, struct in_addr *pin);
    in_addr_t inet_addr(const char *cp);

    char * inet_ntoa(struct in_addr in);

* `inet_aton()`，将点分十进制数串`cp`（例如"192.168.1.100"）转换为32-bit网络字节序的二进制值`pin`。成功返回1，否则返回0；
* `inet_addr()`，与`inet_aton()`功能相同，返回二进制值。不过该函数存在一个问题：函数出错返回INADDR_NONE常值（通常为32位全1的值），因此该函数不能处理地址`255.255.255.255`；「该函数如今已被废弃」
* `inet_ntoa`，将32-bit的网络字节序的二进制值`in`转换为点分十进制数串并返回；返回的字符串驻留在静态内存中，因此该函数不可重入。

## 通用API（可以处理IPv4/IPv6地址，参数复杂）

p，表达（presentation），代表ASCII字符地址串（例如"192.168.1.100"，"2001:0db8:85a3:08d3:1319:8a2e:0370:7344"）；
n，数值（numeric），网络字节序的二进制值（32-bit IPv4 or 128-bit IPv6）；

    int inet_pton(int af, const char * src, void * dst);

    const char * inet_ntop(int af, const void * src, char * dst, socklen_t size);

* `af`，地址族协议，`AF_INET` or `AF_INET6`；其他值返回错误，置errno为EAFNOSUPPORT；
* `inet_pton()`尝试转换ASCII字符地址串`src`，并通过`dst`存放二进制结果；成功返回1，失败返回0；
* `inet_ntop()`尝试转换网络字节序的二进制值`src`，并通过`dst`存放ASCII字符地址串，`len`为`dst`缓冲区大小，避免溢出。`len`太小，则返回空指针并置errno为ENOSPC。