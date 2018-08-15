#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
using namespace std;
// 所需库连接
#pragma comment (lib,"Ws2_32.lib")

int main(int argc, char *argv[])
{
    WSADATA wsaData;  //版本协商
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout<<"装载socket失败!"<<endl;
        return -1;
    }

    SOCKET server_sock, client_sock;  //服务器端和客户端套接字
    //addrinfo结构体是为了消除IPv6协议与IPv4协议之间的差异，编制统一的程序而追加的结构。并且允许多个IPv4地址或IPv6地址链成链表
    struct addrinfo server_addr , //服务器端地址信息
            *res=NULL; //若有多个地址，res是地址信息链表指针
    memset(&server_addr,0,sizeof(server_addr));//如果没有这句话就会出现绑定错误,也就是在调用getaddrinfo()之前该参数必须清0
    server_addr.ai_family = AF_INET6;  //地址簇,这里指定是ipv6协议,其值可以是 AF_INET:ipv4, AF_INET6:ipv6
    server_addr.ai_socktype = SOCK_STREAM;  //套接字类型,这里是流式,其值可以是 SOCK_STREAM:流式, SOCK_DGRAM:数据报, SOCK_RAW:原始套接字
    server_addr.ai_protocol = IPPROTO_TCP;  //传输层协议,这里是TCP协议,其值可以是: IPPROTO_TCP:TCP, IPPROTO_UDP:UDP, 若为0系统根据套接字类型自动选择
    int result;
    result = getaddrinfo("::1", "2000", &server_addr, &res); //解析本机地址, 服务器地址 监听端口
    if(result != 0)//地址解析正确时返回0
    {
        cout<<"服务器地址解析错误!"<<endl;
        return -1 ;
    }

    server_sock = socket(res->ai_family,res->ai_socktype,res->ai_protocol); //创建服务器端socket
    if(server_sock == INVALID_SOCKET)
    {
        cout<<"创建服务器socket失败!"<<endl;
        return -1;
    }

    if(bind(server_sock, res->ai_addr, res->ai_addrlen) == SOCKET_ERROR)  //绑定
    {
        cout<<"服务器端绑定失败!"<<endl;
        return -1;
    }

    if( listen(server_sock ,2) == SOCKET_ERROR )  //监听
    {
        cout<<"服务器端监听失败!"<<endl;
        return -1;
    }

    //  struct sockaddr_in6 client_addr; //客户端地址
    struct sockaddr_storage client_addr; //结构体sockaddr_storage有足够大的空间来存储任何类型的sockaddr
    int clientaddr_len = sizeof(client_addr);  //客户端地址长度
    while(1)
    {
        client_sock = accept(server_sock,(struct sockaddr*)&client_addr,&clientaddr_len);//接受客户端连接
        if(client_sock == INVALID_SOCKET)
        {
            cout<<"接受客户端连接失败!"<<endl;
            continue;   //继续
        }
        char buf[256];//发送和接收缓冲区
        int nRecv=0;

        while(1)
        {
            cout<<"服务器端请输入:";
            cin>>buf;
            send(client_sock,buf,256,0);
            nRecv = recv(client_sock,buf,256,0);
            if(nRecv>0)
            {
                buf[nRecv]='/0';
                cout<<"服务器收到:"<<buf<<endl;
            }
        }
    }
    closesocket(server_sock);//关闭服务器端套接字
    closesocket(client_sock);//关闭客户端套接字
    
    return 1;
}
