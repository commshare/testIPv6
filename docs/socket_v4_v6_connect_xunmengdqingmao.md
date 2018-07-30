
(https://blog.csdn.net/XunMengdQingMao/article/details/51603825)

socket兼容IPV4、IPV6网络底层connect写法
2016年06月07日 15:02:41
阅读数：6819
由于IOS苹果审核需要支持ipv6，我公司手游用的socket底层调用的是C标准库，故在此记录下需要调整修改的地方

1、网络环境配置方面，具体参见这篇帖子，：http://www.code4app.com/thread-8513-1-1.html

补充说明，由于IPV6需要设备支持，搭建好的IPV6的WIFI，可能电脑跟安卓手机都连接不上，但用苹果手机连接却可以浏览上网，
那证明网络OK，可以在真机上进行下一步的代码调试了。

2、原理说明：在访问服务器前，通过getaddrinfo创建对应的网络协议，
ipv6网络下可以得到服务器的IPV6地址（其实是IPV4下IP的IPV6写法），通过IPV6地址去访问



这里贴出主要的Connect方法，Connect能连接上了，后面send，recv都是可以的不用修改

android、IOS环境下代码，兼容IPV4跟IPV6：

bool ODSocket::Connect(const char* domain, unsigned short port)
{
    //连接ip
    char ip[128];
    memset(ip, 0, sizeof(ip));
    strcpy(ip, domain);
 
	void* svraddr = nullptr;
    int error=-1, svraddr_len;
    bool ret = true;
    struct sockaddr_in svraddr_4;
    struct sockaddr_in6 svraddr_6;
 
    //获取网络协议
    struct addrinfo *result;
    error = getaddrinfo(ip, NULL, NULL, &result);
    //TODO 问题是这里只有一个地址返回么？而且上没hints是用NULL，所以回有ipv4返回，break，这样就没v6的事儿了
    const struct sockaddr *sa = result->ai_addr;
    socklen_t maxlen = 128;
    switch(sa->sa_family) {
        case AF_INET://ipv4
			if ((m_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket create failed");
                ret = false;
                break;
            }
            if(inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                         ip, maxlen) < 0){
                perror(ip);
                ret = false;
                break;
            }
            svraddr_4.sin_family = AF_INET;
            svraddr_4.sin_addr.s_addr = inet_addr(ip);
            svraddr_4.sin_port = htons(port);
            svraddr_len = sizeof(svraddr_4);
            svraddr = &svraddr_4;
            break;
        case AF_INET6://ipv6
            if ((m_sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
                perror("socket create failed");
                ret = false;
                break;
            }
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                      ip, maxlen);
            
            printf("socket created ipv6/n");
            
            bzero(&svraddr_6, sizeof(svraddr_6));
            svraddr_6.sin6_family = AF_INET6;
            svraddr_6.sin6_port = htons(port);
            if ( inet_pton(AF_INET6, ip, &svraddr_6.sin6_addr) < 0 ) {
                perror(ip);
                ret = false;
                break;
            }
            svraddr_len = sizeof(svraddr_6);
            svraddr = &svraddr_6;
            break;
            
        default:
            printf("Unknown AF\ns");
			ret = false;
    }
	freeaddrinfo(result);
	if(!ret)
	{
		fprintf(stderr , "Cannot Connect the server!n");
		return false;
	}
	int nret = connect(m_sock, (struct sockaddr*)svraddr, svraddr_len);
	if(nret==SOCKET_ERROR )
    {
		return false;
	}
 
    return true;
}

