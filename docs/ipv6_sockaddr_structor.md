
DOC HOME	SITE MAP	MAN PAGES	GNU INFO	SEARCH	PRINT BOOK
 (http://osr600doc.xinuos.com/en/SDK_netapi/sockC.TheIPv6sockaddrstructure.html)

# Programming with sockets
The IPv6 sockaddr structure
Due to the increased size of the IPv6 address, two new structures are needed. These structures are:

in6_addr
sockaddr_in6

#The in6_addr structure
The in6_addr structure holds a single IPv6 address. The structure is shown below.
```asm
struct in6_addr {
           u_int8_t  s6_addr[16];  /* IPv6 address */
   }
```
   十六个八位元素，组成单个128位的ipv6地址， 这个地址通常以网络字节序存储。
The structure contains an array of sixteen 8-bit elements, that together make up a single 128-bit IPv6 address. 
The address is usually stored in network byte order. 
This structure is declared in netinet/in.h.

# The sockaddr_in6 structure
The sockaddr_in6 structure is used to pass address information to the socket function calls that 
require network address information. The structure is shown below.
这个是给socket函数用的网络地址信息结构体
```asm
  # define SIN6_LEN
   

   struct sockaddr_in6 {
          u_char           sin6_len;      /* length of this structure */
          u_char           sin6_family;   /* AF_INET6                 */
          u_int16m_t       sin6_port;     /* Transport layer port #   */
          u_int32m_t       sin6_flowinfo; /* IPv6 flow information    */
          struct in6_addr  sin6_addr;     /* IPv6 address             */
   };
```
 
对于sockaddr_in,支持可变长度的sockets， sin6_famliy成员从16位变为了8位的数据类型。
sin6_family 从16位变为了8位
NOTE: For those familiar with sockaddr_in, note that to support variable length sockets,
 the sin6_family member has been changed from a 16-bit to an 8-bit data type. 
 This change has also been effected in the sockaddr_in data structure.
  Binaries built using the older data structures will still work, however,
   new applications will use the newer version of sockaddr_in (for IPv4 applications) 
   or sockaddr_in6 (for IPv6 applications).
   
   如果给sockaddr_in6 结构分配内存，通常用sizeof（struct sockaddr_in6）
   这是因为sockaddr_in6 比sockaddr
When allocating a buffer to hold the sockaddr_in6 structure always use sizeof(struct sockaddr_in6) 
because the sockaddr_in6 structure is larger than sockaddr.

在ipv4里，INADDR_ANY和INADDR_LOOPBACK是用来对源地址和回环地址选择的。
In IPv4 two constants, INADDR_ANY and INADDR_LOOPBACK, are provided for system selection of source addresses and loopback. 
In IPv6, because the address is stored in the in6_addr structure, a different mechanism has to be used as described below.
对于IPV6，地址存储在in6_addr里，所以要安昭下面的搞：

# System selection of source addresses 源地址的系统选择
To enable the system to select the source address for your IPv6 application, use the global variable in6addr_any.
 For example, to bind to port number 23, and let the system select the source address, you could use the following code:

```asm
   struct sockaddr_in6 sin6;
   ...
   sin6.sin6_len = sizeof(sin6);
   sin6.sin6_family = AF_INET6;
   sin6.sin6_flowinfo = 0;
   sin6.sin6_port = htons(23);
   sin6.sin6_addr = in6addr_any;//让系统去选择源地址
   ...
   if (bind(s, (struct sockaddr *) &sin6, sizeof(sin6)) == -1)
   {
   ...
   }
```

If you just want to initialize a in6_addr structure, you can use the symbolic constant IN6ADDR_ANY_INIT 
as shown in the code below.
下面用于初始化一个in6_addr ：
```asm
   struct in6_addr anyaddr = IN6ADDR_ANY_INIT;

```
NOTE: You cannot use this constant to assign a previously declared in6_addr structure.
 For example, the following would not compile.
 ```asm
   struct sockaddr_in6 sin6;
   ...
   sin6.sin6_addr = INADDR_ANY_INIT;
```

INADDR_ANY_INIT and in6addr_any are declared in netinet/in.h.


# Loopback

本地的upd或者tcp服务。使用in6addr_loopback 。

To enable your application to use local UDP and TCP services, 
use the global variable in6addr_loopback. For example, to open a TCP connection to the local telnet server, 
you could use the following code:

打开一个到本地的telnet的连接
```asm
   struct sockaddr_in6 sin6;
   ...
   sin6.sin6_len = sizeof(sin6);
   sin6.sin6_family = AF_INET6;
   sin6.sin6_flowinfo = 0;
   sin6.sin6_port = htons(23);
   sin6.sin6_addr = in6addr_loopback;
   ...
   if (bind(s, (struct sockaddr *) &sin6, sizeof(sin6)) == -1)
   {
   ...
   }
```

If you just want to initialize a in6_addr structure, 
you can use the symbolic constant IN6ADDR_LOOPBACK_INIT as shown in the code below.
```asm
   struct in6_addr anyaddr = IN6ADDR_LOOPBACK_INIT;

```
# 注意，不能使用这个常量，去assign给一个先前声明过的in6_addr 结构
NOTE: You cannot use this constant to assign a previously declared in6_addr structure.
 For example, the following would not compile.
 
 下面的这种是编译不过的：
 ```asm
   struct sockaddr_in6 sin6;
   ...
   sin6.sin6_addr = INADDR_LOOPBACK_INIT;
```

INADDR_LOOPBACK_INIT and in6addr_loopback are declared in netinet/in.h.

© 2005 The SCO Group, Inc. All rights reserved. 
SCO OpenServer Release 6.0.0 -- 02 June 2005 