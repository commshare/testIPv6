(https://www.jianshu.com/p/1eb75b48b066)

# socket的绑定和监听
## 使用BSD Socket创建

//ipv4
  struct sockaddr_in addr4;
  bzero(&addr4, sizeof(addr4));
  addr4.sin_len = sizeof(addr4);
  addr4.sin_family = AF_INET;
  addr4.sin_port = htons(port);
  addr4.sin_addr.s_addr = bindToLocalhost ? htonl(INADDR_LOOPBACK) : htonl(INADDR_ANY);

 //ipv6
  struct sockaddr_in6 addr6;
  bzero(&addr6, sizeof(addr6));
  addr6.sin6_len = sizeof(addr6);
  addr6.sin6_family = AF_INET6;
  addr6.sin6_port = htons(port);
  addr6.sin6_addr = bindToLocalhost ? in6addr_loopback : in6addr_any;
  
  int listeningSocket = socket(useIPv6 ? PF_INET6 : PF_INET, SOCK_STREAM, IPPROTO_TCP);
  int yes = 1;
  setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    
  bind(listeningSocket, addr, length);
  listen(listeningSocket, (int)maxPendingConnections) == 0);
  if (port == 0) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    if (getsockname(listeningSocket, (struct sockaddr*)&addr, &addrlen) == 0) {
      port = ntohs(addr.sin_port);
    }
  }
上面的步骤实现了socket的绑定和监听，要实现socket数据的读写需要创建可读写的管道并连接到socket。

作者：Ctiy
链接：https://www.jianshu.com/p/1eb75b48b066
來源：简书
简书著作权归作者所有，任何形式的转载都请联系作者获得授权并注明出处。