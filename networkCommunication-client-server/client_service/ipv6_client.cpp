#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
using namespace std;
// 所需库连接
#pragma comment (lib,"Ws2_32.lib")

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout<<"装载socket失败!"<<endl;
        return -1 ;
    }
    SOCKET client_sock;  //客户端本地套接字
    struct addrinfo server_addr, *addrinfo=NULL; //服务器端地址
    memset(&server_addr,0,sizeof(server_addr));//在getaddrinfo()之前必须清0
    server_addr.ai_family = AF_INET6;  //指定是ipv6协议
    server_addr.ai_socktype = SOCK_STREAM; //流式
    server_addr.ai_protocol = IPPROTO_TCP;  //TCP协议,也可以是0，让系统自己选
    int result;
    result = getaddrinfo("::1","2000",&server_addr,&addrinfo);  //服务器端ipv6地址, 服务器监听的端口
    if(result !=0)//地址解析正确时返回0
    {
        cout<<"客户端解析服务器地址失败!"<<endl;
        return -1;
    }
    client_sock = socket(addrinfo->ai_family,addrinfo->ai_socktype,addrinfo->ai_protocol); //创建客户端套接字
    if(client_sock == INVALID_SOCKET)
    {
        cout<<"创建客户端socket失败!"<<endl;
        return -1;
    }
    if( connect(client_sock,addrinfo->ai_addr,addrinfo->ai_addrlen) == SOCKET_ERROR)  //连接服务器
    {
        cout<<"连接服务器出错!"<<endl;
        return -1;
    }
    char buf[256];  //发送和接收缓冲区
    int nRecv=0;

    while(1)
    {
        nRecv=recv(client_sock,buf,256,0);
        if(nRecv>0)
        {
            buf[nRecv]='/0';
            cout<<"客户端收到:"<<buf<<endl;
        }
        cout<<"客户端请输入:";
        cin>>buf;
        send(client_sock,buf,256,0);
    }
    closesocket(client_sock); //关闭客户端套接字
    return 1;
}
