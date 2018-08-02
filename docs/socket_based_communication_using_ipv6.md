# Chapter 3. Sockets-based Communication Using IPv6
(http://retrogeeks.org/sgi_bookshelves/SGI_Developer/books/IRIX_NetPG/sgi_html/ch03.html)
Prev 	 	 Next
# Chapter 3. Sockets-based Communication Using IPv6
This chapter describes the sockets-based communication facilities using IPv6.

# Topics in this chapter include:

- Introduction of the AF_INET6 socket and related address structures

- Client/Server programs for connection-based and connectionless sockets

- Socket options for unicasting and multicasting

- Multicasting


# Creation of an IPv6 Socket
The socket() call creates a socket in the specified domain of the specified type, as in the following example:
```asm
#include <sys/socket.h>
s = socket(domain, type, protocol);
```

A number of domains are available from the sys/socket.h file, including the AF_INET6 domain. 
Following are all of the domains that are available:
```asm
AF_UNIX	 	
UNIX domain

AF_INET6	 	
Internet domain for IPv6 and IPv4

AF_INET	 	
Internet domain for IPv4 only

AF_RAW	 	
Raw domain
```


The socket types, SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW are supported by the domains AF_INET6, AF_INET, and AF_UNIX.
 For example, to create a stream socket in the IPv6 domain, use the following entry:

```asm
s = socket(AF_INET6, SOCK_STREAM, 0);

```

# IPv6 Address Structures
The in6_addr structure, defined in netinet/in.h, stores IPv6 addresses. 
Also defined in netinet/in.h is the sockaddr_in6 structure, a protocol-specific socket address data structure for IPv6.

# IPv6 Unspecified Address
The global variable in6addr_any is defined as an in6_addr structure and initialized to the “unspecified address” in netinet/in.h. 
After opening an AF_INET6 socket, for the system to select the source address, 
this “unspecified address” can be used in the bind() call.


For example, you can use the following code to bind an AF_INET6 socket to a port number but let the system select the source address. 
Here the global variable in6addr_any is used.

```asm

struct sockaddr_in6 sin6;
...
bzero(&sin6, sizeof(struct sockaddr_in6));
sin6.sin6_family = AF_INET6;
sin6.sin6_port = htons(23);
sin6.sin6_addr = in6addr_any;
...
if(bind(s, (struct sockaddr *)&sin6, sizeof(sin6)) == -1)
...

```

You can use the symbolic constant IN6ADDR_ANY_INIT, which is also defined in netinet/in.h, 
to initialize an object of type struct in6_addr. 

The object will then contain the same address as the global variable in6addr_any.
 Consider the following example:

```asm
struct in6_addr anyaddr = IN6ADDR_ANY_INIT;

```

This constant can be used only at declaration time. 
It cannot be used to assign a previously declared in6_addr structure. 
For example, the following code will not work:

```asm
struct sockaddr_in6 sin6;
...
sin6.sin6_addr = IN6ADDR_ANY_INIT;  /* will NOT compile */
```


# IPv6 Loopback Address
Like the unspecified address, the IPv6 loopback address is also provided in two forms: 
- a global variable and a symbolic constant. 
- The global variable is defined as an in6_addr structure named in6addr_loopback. 

- The symbolic constant is named IN6ADDR_LOOPBACK_INIT.
 Both are defined in netinet/in.h.
  Like IN6ADDR_ANY_INIT, IN6ADDR_LOOPBACK_INIT cannot be used in an assignment to a previously declared IPv6 address variable.


## The following example shows the use of in6addr_loopback:

```asm
struct sockaddr_in6 sin6;
...
sin6.sin6_addr = in6addr_loopback;
...
if(connect(s, (struct sockaddr *)&sin6, sizeof(sin6)) == -1)
...
```

The following example shows the use of IN6ADDR_LOOPBACK_INIT:

```
struct in6_addr loopbackaddr = IN6ADDR_LOOPBACK_INIT;
```

# Protocol Independent Socket Address Structure
You can use the sockaddr_storage structure, defined in sys/socket.h, 
to write protocol-independent application programs.

 - This data structure is large enough to accommodate both AF_INET and AF_INET6 protocol-specific address structures. 
 It is also aligned at an appropriate boundary so that pointers to it can be cast as pointers to protocol specific
  address structures and they can be used to access the fields of those structures without alignment problems.

For example, it can be used in a server code in the following way:

下面是一个服务端的代码：
```asm
struct sockaddr_storage from;
...
struct addrinfo hints, *res, *ressave;
int oldsock, newsock, err_num;
...
hints.ai_family = AF_UNSPEC;  //没指定哦
hints.ai_family = AI_PASSIVE | AI_ADDRCONFIG;
...
err_num = getaddrinfo(NULL, argv[1], &hints, &res);
ressave = res;
do {
    /* do socket(), bind() and listen() calls here */
    ...
    if((newsock = accept(oldsock, (struct sockaddr *)&from,
               sizeof(from))) == -1) {
        perror("accept");
        exit(1);
    }
    else {
        doit(&from);
    }
}
```
...

...

# Server and Client Programs
For the server and client examples given in this section, 
if the ai_family field of the hints structure is replaced with AF_UNSPEC and if AF_INET6 is not supported, 
the program tries for the AF_INET family.

如果，在hints结构里，ai_familiy 不使用AF_UNSPEC 取代，并且如果AF_INET6 不支持，
那么默认尝试AF_INET 

## Connection-based Server and Client
A server process normally opens a stream socket and listens at a particular port number for sevice requests.
 The client initiates a connection to the server's address and port number by calling connect(). 
 At this point, the server wakes up and services the client request.

## Connection-based Server
Example 3-1 is an example of a server program that creates an IPv6 socket and binds a name to that socket.
 The port number on which the socket is to be bound is provided on the command line.
  The program calls listen() to mark the socket as ready to accept incoming connections. 
  In this example, the number of connections that the server can accept is set to five. 
  Each pass of the loop accepts a new connection and creates a new socket. 
  The server reads and displays the messages from the socket and closes it.

# The command-line format for the server program is as follows:

program_name port_number

# Example 3-1. Connection-based Server
```asm

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#define MAX_CONN        5

main(argc, argv)
    int argc;
    char *argv[];
    {
    struct addrinfo hints, *res, *ressave;
    struct sockaddr_storage from;
    int fromlen;
    int err_num;
    int oldsock, newsock;
    int readval;
    char buf[1024];
    int conn_num = 0;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6; //强制  
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;  // 系统给ip
    hints.ai_socktype = SOCK_STREAM;

    err_num = getaddrinfo(NULL, argv[1], &hints, &res);
    if(err_num) {
        fprintf(stderr, "getaddrinfo: %s", gai_strerror(err_num));
        exit(1);
    }
    ressave = res;
    do {
       oldsock = socket(res->ai_family, res->ai_socktype,
                  res->ai_protocol);
       if(oldsock < 0)
              continue;
       if(bind(oldsock, res->ai_addr, res->ai_addrlen) < 0) {
             perror("bind");
             close(oldsock);
             continue;
       }
       if(listen(oldsock, 5) < 0) {
             perror("listen");
             close(oldsock);
             continue;
       }
       do    {
             if((newsock = accept(oldsock, (struct sockaddr *)&from,
                              &fromlen)) < 0) {
                   perror("accept");
                   close(oldsock);
             }
             else {
               do {
                  memset(&buf, 0, sizeof(buf));
                  if((readval = read(newsock, buf, 1024)) == -1)
                      perror("read");
                  if(readval == 0)
                     printf("Ending connection\n");
                  else
                     printf("---> %s\n", buf);
               } while (readval > 0);
               conn_num++;
             }
             close(newsock);
      } while (conn_num < MAX_CONN);
      /* break after establishing the required no. of connections */
      close(oldsock);
      break;
   } while ((res = res->ai_next) != NULL);
   if(!res) {
      fprintf(stderr, "bind/listen/accept failed for all addresses\n");
      freeaddrinfo(ressave);
      exit(1);
   }
   freeaddrinfo(ressave);
} /* end of main */
```

# Connection-based Client
Example 3-2 is an example of a client program that creates an AF_INET6 stream socket and calls connect() 
with the socket address for connection. If the request is accepted, the connection is complete and the
 program can send data. The port number provided on the command line should match the port number provided on the
  command line for the server program.

## The command-line format for this program is as follows:

 program_name server_name port_number

Example 3-2. Connection-based Client

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#define DATA "Data received through connection-based socket from client"
main(argc, argv)
   int argc;
   char *argv[];
{
   struct addrinfo hints, *res, *ressave;
   int s, err_num;

   bzero(&hints, sizeof(struct addrinfo));
   hints.ai_family = AF_INET6;
   hints.ai_flags = AI_ADDRCONFIG | AI_CANONNAME; //这个是？
   hints.ai_socktype = SOCK_STREAM;

   err_num = getaddrinfo(argv[1], argv[2], &hints, &res);
   if(err_num) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err_num));
      exit(1);
}
ressave = res;
do {
   s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   if (s < 0)
         continue;
   if(connect(s, res->ai_addr, res->ai_addrlen) < 0) {
         perror("connect");
               close(s);
      }
      else
               break;
   } while ((res = res->ai_next) != NULL);
   if (!res) {
      fprintf(stderr, "socket/connect failed for all addresses\n");
      freeaddrinfo(ressave);
      exit(1);
   }
   if (write(s, DATA, sizeof DATA) == -1)
      perror("write");
   close(s);
   freeaddrinfo(ressave);
} /* end of main */


# Connectionless Server and Client
With datagram sockets, data can be exchanged without requiring connection establishment. 
The sendto() call sends data on an unconnected datagram socket. 
You can use recvfrom() to receive data on a datagram socket.

## Connectionless Server
Example 3-3 is an example program that creates a datagram socket, binds a name to it, then reads from the socket.

The command-line format for this server program is as follows:

program_name port_number

Example 3-3. Connectionless Server

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

main(argc, argv)
   int argc;
   char *argv[];
{
   struct addrinfo hints, *res, *ressave;
   int s, err_num;
   char buf[1024];

   bzero(&hints, sizeof(hints));
   hints.ai_family = AF_INET6;
   hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
   hints.ai_socktype = SOCK_DGRAM;

   err_num = getaddrinfo(NULL, argv[1], &hints, &res);
   if(err_num) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err_num));
      exit(1);
   }
   ressave = res;
   do {
      s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
      if(s < 0)
            continue;
            //绑定获取到的本地的地址
      if(bind (s, res->ai_addr, res->ai_addrlen) == -1) {
             perror("bind");
             close(s);
             continue;
      }
      if(read(s, buf, 1024) == -1) {
             perror("read");
             close(s);
             continue;
      }
      else {
             printf("---> %s\n", buf);
             close(s);
             break;
      }
   } while ((res = res->ai_next) != NULL);
   if(!res){
      fprintf(stderr, "socket/bind/read failed for all addresses\n");
      freeaddrinfo(ressave);
      exit(1);
   }
   freeaddrinfo(ressave);
} /* end of main */


## Connectionless Client
Example 3-4 is an example of a program that opens an AF_INET6 datagram socket and sends a datagram to the server whose name is obtained from the command line argument. The port number provided on the command line should match the port number that is provided on the command line for the server program.

The command line format for this program is as follows:

program_name server_name port_number

Example 3-4. Connectionless Client

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#define DATA "Data received through connection-less socket from client"
main(argc, argv)
   int argc;
   char *argv[];
{
   struct addrinfo hints, *res, *ressave;
   int s, err_num;

   bzero(&hints, sizeof(hints));
   hints.ai_family = AF_INET6;
   hints.ai_flags = AI_ADDRCONFIG | AI_CANONNAME;
   hints.ai_socktype = SOCK_DGRAM;

   //这里是服务端的 ip和port
   err_num = getaddrinfo(argv[1], argv[2], &hints, &res);
   if(err_num) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err_num));
      exit(1);
   }
   ressave = res;
   do {
      s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
      if(s < 0)
            continue;
      if(sendto(s, DATA, sizeof DATA, 0, res->ai_addr, res->ai_addrlen)
                == -1) {
            perror("send");
            close(s);
             continue;
      }  
      else {
             close(s);
             break;
      }
   } while ((res = res->ai_next) != NULL);
   if(!res) {
      fprintf(stderr, "socket/sendto failed for all addresses\n");
      freeaddrinfo(ressave);
      exit(1);
   }
   freeaddrinfo(ressave);
} /* end of main */


# Socket Options
For IPv6, there are a number of socket options that are defined at the IPPROTO_IPV6 level. 
When you use these socket options, set the level parameter to IPPROTO_IPV6.
 You can obtain these socket options and the declaration for IPPROTO_IPV6 by including the header netinet/in.h.


# Unicast Socket Options
Use the socket option IPV6_UNICAST_HOPS at the IPPROTO_IPV6 level to control the hop limit used in outgoing IPv6 unicast packets. 
Example 3-5 shows its usage:

Example 3-5. Using IPV6_UNICAST_HOPS

int hoplimit = 10;

if(setsockopt(s, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (char *)&hoplimit,
             sizeof(hoplimit)) == -1)
       perror("setsockopt IPV6_UNICAST_HOPS");

If this option is not set, the system selects a default value.

#  Multicast Socket Options
Several socket options are available for sending and receiving IPv6 multicast packets. The following sections describe those options.

Sending Packets
The following socket options at the IPPROTO_IPV6 level control some of the parameters for sending IPv6 multicast packets:

IPV6_MULTICAST_IF

IPV6_MULTICAST_HOPS

IPV6_MULTICAST_LOOP

The IPV6_MULTICAST_IF socket option sets the interface to use for outgoing multicast packets. If the interface index is specified as 0, the system selects the interface to use. Here the argument type is unsigned int. In the following example, ifindex is the interface index for the desired outgoing interface.

unsigned int ifindex;
ifindex = if_nametoindex("ef0");
setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex,sizeof(ifindex));
The IPV6_MULTICAST_HOPS socket option sets the hop limit to use for outgoing multicast packets. If it is not set, the default is 1. Here the argument type is int.

If a multicast datagram is sent to a group to which the sending host itself belongs (on the outgoing interface) and theIPV6_MULTICAST_LOOP option is set to 1, a copy of the datagram is looped back for local delivery. If this option is set to 0, a copy is not looped back. If this option is not set, the default is 1. This socket option in IPv6 is similar to IP_MULTICAST_LOOP in IPv4. The argument type is unsigned int. In the following example, loop is 0 to disable loopback and 1 to enable loopback.

unsigned int loop;
setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loop, sizeof(loop));

Receiving Packets
The following socket options control the reception of IPv6 multicast packets:

IPV6_JOIN_GROUP

IPV6_LEAVE_GROUP

The IPV6_JOIN_GROUP socket option allows you to join a multicast group on a specified local interface. The ipv6_mreq structure, defined in netinet/in.h, is to be used for this purpose, as shown in the following example. If ipv6mr_interface is set to 0, the kernel chooses the local interface.

struct ipv6_mreq {
struct in6_addr ipv6mr_multiaddr;  /* IPv6 multicast address of group */
unsigned int    ipv6mr_interface;  /* interface index */
};
The IPV6_LEAVE_GROUP socket option allows you to leave a multicast group on a specified interface. If the ipv6mr_interface field of the ipv6_mreq structure is set to 0, the system chooses a multicast group membership to drop by matching the multicast address.

Socket Option for IPv6 Only
The IPV6_V6ONLY socket option restricts AF_INET6 sockets to IPv6 communication only. Normally, AF_INET6 sockets can be used for both IPv4 and IPv6 communications. But if an application wishes to restrict the use of AF_INET6 sockets to IPv6 communications only, this socket option can be used, as in the following example:

int on = 1;
setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&on, sizeof(on));

Using Multicasting
IP multicasting is supported both on AF_INET6 and AF_INET sockets for connectionless protocols, that is, for sockets of types other than SOCK_STREAM, and only on subnetworks for which the interface driver supports multicasting. The following sections describe sending and receiving IPv6 multicast datagrams.

	Note: Broadcasting is not supported in IPv6. It is supported only in IPv4.

Sending IPv6 Multicast Datagrams
To send an IPv6 multicast datagram, specify an IP multicast address in the range ff00::0/8 as the destination address in a sendto() call.

The definitions required for the multicast-related socket options are found in netinet/in.h.

By default, IPv6 multicast datagrams are sent with a hop limit of 1, which prevents it from being forwarded beyond a single subnetwork. The socket option IPV6_MULTICAST_HOPS allows the hop limit for subsequent multicast datagrams to be set to any value from 0 to 255, to control the scope of the multicasts, as in the following example:

int hops;
setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops,sizeof(hops));
IPv6 multicast datagrams with a hop limit of 0 are not transmitted on any subnet but can be delivered locally if the following conditions exist:

The sending node belongs to the destination group.

Multicast loopback on the sending socket is enabled.

Multicast datagrams with a hop limit greater than 1 might be delivered to more than one subnetwork if there is at least one multicast router attached to the first-hop subnetwork.

The IPv6 multicast addresses contain scope information encoded in the first part of the address. The scopes are defined in netinet/in.h.

Receiving IPv6 Multicast Datagrams
Before a node can receive IP multicast datagrams for a given multicast address, it must become a member of the associated IP multicast group. A process can ask the node to join an IPv6 multicast group by using the IPV6_JOIN_GROUP socket option, as follows:

struct ipv6_mreq mreq;
setsockopt(s, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq));
Each membership is associated with a single interface and it is possible to join the same group on more than one interface. If ipv6mr_interface is 0, the default multicast interface is chosen for the membership. Otherwise, one of the node's interface indices that is specified as ipv6mr_interface is chosen for the multicast membership.

To leave a group, use the following code:

struct ipv6_mreq mreq;
setsockopt(s, IPPROTO_IPV6, IP_LEAVE_GROUP, &mreq, sizeof(mreq));
The mreq argument contains the same values as those used to add the membership. The socket drops associated memberships when the socket is closed or the process holding the socket is killed. However, more than one socket may claim a membership in a particular group, and the node will remain a member of that group until the last claim is dropped.

Prev 	Table of Contents	 Next
Chapter 2. Sockets-based Communication 	 	Chapter 4. Introduction to RPC Programming 