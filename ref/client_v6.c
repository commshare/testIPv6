/************************************************************
Copyright (C), 2017, Leon, All Rights Reserved.
FileName: client_v6.c
Description: IPV6网络编程练习
Author: Leon
Version: 1.0
Date: 2017-2-9 11:09:39
Function:
History:
<author>    <time>  <version>   <description>
 Leon
************************************************************/

// connect时地址长度填错，一直报这个错 connect error:: Invalid argument

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUF_LEN 2048
#define PORT 10023

int main(int argc, char *argv[])
{
    int sock = -1;
    socklen_t addr_len = 0;
    struct sockaddr_in6 serv_addr = {0};
    char buf[BUF_LEN] = {0};
    int err = -1;

    /* 建立socket */
    sock = socket(AF_INET6, SOCK_STREAM, 0);
    if(-1 == sock)
    {
        perror("socket error: ");
        return -1;
    }

    memset(&serv_addr, 0x0, sizeof(serv_addr));
    /* 填充地址结构 */
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(PORT);
    serv_addr.sin6_addr = in6addr_loopback;    /* 连接到环回地址 */

    // 不能connect到链路本地地址
    // inet_pton(AF_INET6, "fe80::20c:29ff:fe8f:c168", &serv_addr.sin6_addr);

    addr_len = sizeof(serv_addr);
    err = connect(sock, (struct sockaddr *)&serv_addr, addr_len);
    if(-1 == err)
    {
        perror("connect error:");
        close(sock);
        return -1;
    }

    /* 发送消息 */
    memset(buf, 0x0, BUF_LEN);
    snprintf(buf, BUF_LEN - 1, "hello server, I'm client\n");
    err = send(sock, buf, strlen(buf), 0);
    if(err < 0)
    {
        perror("send error:");
        close(sock);
        return -1;
    }

    /* 接收消息 */
    memset(buf, 0x0, BUF_LEN);
    err = recv(sock, buf, BUF_LEN, 0);
    if(err < 0)
    {
        perror("recv error:");
        close(sock);
        return -1;
    }
    printf("recv %d bytes: %s\n", err, buf);

    close(sock);

    return 0;
}