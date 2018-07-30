解决App Store 上架 IOS 程序必须支持IPV6
(https://blog.csdn.net/joelcat/article/details/78824397)
2017年12月17日 12:27:23
阅读数：4413
最近在将自己的ios程序发布到app store, 其中遇到不少坑, 这里记录一下关于app 的纯ipv6环境下网络访问的问题;
首先声明一下, 纯ipv6环境下是否能访问到你的服务,和你的服务器有没有ipv6地址没多大关系,这个问题不应该从服务器着手处理,而是我们写的代码没有支持ipv6 !;
下面提到的方法同样适用与mac os x 程序;
如果你的程序是因为不支持纯ipv6 环境被拒绝的话, 那么收到的邮件应该如下:

Guideline 2.1 - Performance - App Completeness


We discovered one or more bugs in your app when reviewed on iPad running iOS 11.2 on Wi-Fi connected to an IPv6 network.

Specifically, we were unable to login during the review.

Please see attached screenshots for details.

Next Steps

To resolve this issue, please run your app on a device to identify any issues, then revise and resubmit your app for review.

If we misunderstood the intended behavior of your app, please reply to this message in Resolution Center to provide information on how these features were intended to work.

For new apps, uninstall all previous versions of your app from a device, then install and follow the steps to reproduce the issue. For updates, install the new version as an update to the previous version, then follow the steps to reproduce the issue.

Resources

For information about testing your app and preparing it for review, please see Technical Note TN2431: App Testing Guide. 

For a networking overview, please review About Networking. For a more specific overview of App Review’s IPv6 requirements, please review the IPv6 and App Review discussion on the Apple Developer Forum.
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
我的服务器是使用的阿里云的windows server 2008,在国内的网络环境(ipv4)不论什么姿势都是能访问的, 可是国外的网络大多数已经切换到ipv6下;

直接上解决方案吧, 我没东西可以扯了, 初写博客,文笔不行!
首先,如果你的代码中访问服务器的地址是ip地址, 如:

NSString *encodedValue =
    [@"http://123.123.88.127:8080/test"
     stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    NSURLRequest *urlRequest =
    [NSURLRequest requestWithURL:
     [NSURL URLWithString:encodedValue]];

    NSURLResponse *response = nil;
    NSError *error = nil;
    NSData *data = [NSURLConnection
                    sendSynchronousRequest:urlRequest
                                          returningResponse:&response
                                                      error:&error];
    if(error)
    {
        int a = 0;

那么, 请为你的服务器配置一个域名就ok了;
我的程序中还用到了socket 通信,底层用c++编写了部分功能, 这个时候只要在访问的时候将ipv4地址转换成ipv6的就OK啦;
下面展示了我测试的代码片段;
TestIPV6.h
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <err.h>

@interface TestIPV6 : NSObject
+ (void)testMain;
@end

TestIPV6.m
#import "TestIPV6.h"

@interface TestIPV6()
{

}
//获取不同网络环境的IP地址
+ (NSString *)getSockAddrIPString:(const struct sockaddr *)sa;
//根据不同的网络环境设置端口
+ (void)setPort:(uint16_t)port forSockAddr:(const struct sockaddr *)sa;
@end

@implementation IPv6Tester

+ (NSString *)getSockAddrIPString:(const struct sockaddr *)sa
{
    NSString *result = @"";
    sa_family_t netType = sa->sa_family;
    if(netType == AF_INET) //V4d的环境
    {
        char ipv4_str_buf[INET_ADDRSTRLEN] = {0};
        struct sockaddr_in *v4sa = (struct sockaddr_in *)sa;

        inet_ntop(AF_INET,//
                  &(v4sa->sin_addr),//
                  ipv4_str_buf,//
                  sizeof(ipv4_str_buf));

        result = [[NSString alloc] initWithCString:ipv4_str_buf encoding:NSUTF8StringEncoding];
    }
    else if(netType == AF_INET6)
    {
        char ipv6_str_buf[INET6_ADDRSTRLEN] = {0};
        struct sockaddr_in6 *v6sa = (struct sockaddr_in6 *)sa;

        inet_ntop(AF_INET6,//
                  &(v6sa->sin6_addr),//
                  ipv6_str_buf,//
                  sizeof(ipv6_str_buf));

        result = [[NSString alloc] initWithCString:ipv6_str_buf encoding:NSUTF8StringEncoding];
    }
    return result;
}

struct sockaddr 根据网络类型设置端口
+ (void)setPort:(uint16_t)port forSockAddr:(const struct sockaddr *)sa
{
    sa_family_t netType = sa->sa_family;
    if(netType == AF_INET)
    {
        struct sockaddr_in *v4sa = (struct sockaddr_in *)sa;
        v4sa->sin_port = htons(port);
    }
    else if(netType == AF_INET6)
    {
        struct sockaddr_in6 *v6sa = (struct sockaddr_in6 *)sa;
        v6sa->sin6_port = htons(port);
    }
}

+ (void)testMain
{
    struct addrinfo res0;  用这个addrinfo
    struct addrinfo *res1 = 0;
    struct addrinfo *res2 = 0;

    const char *cause = 0;
    //这里更换成你的服务器ip地址
    const char *address = "xxx.xx.xx.xxx";  服务器地址
    //这里是服务端口
    NSUInteger port = 8084;  服务器端口

    memset(&res0, 0, sizeof(res0));
    res0.ai_family = PF_UNSPEC;
    res0.ai_socktype = SOCK_STREAM;
    res0.ai_flags = AI_DEFAULT;  //加这个会过滤ipv4地址呢

    int error = getaddrinfo(address, NULL, &res0, &res1); 这是ios的接口，可以获取到本地的addrinfo，res1是返回值，address可以是ip地址呢
    if (error)
    {
        //没有连接上
        errx(1, "%s", gai_strerror(error));   获取失败了
    }

    int s = -1;
    for (res2 = res1; res2; res2 = res2->ai_next)   获取远端地址的信息成功了， 对每个地址，都建立1个socket 用于connect？
    {
        s = socket(res2->ai_family,
                   res2->ai_socktype,
                   res2->ai_protocol);
        if (s < 0)  创建客户端socket失败
        {
            cause = "socket";
            continue;
        }

        struct sockaddr *addr =  (struct sockaddr *)res2->ai_addr;  地址设置端口
        [TestIPV6 setPort:(uint16_t)port forSockAddr:addr];

        NSString *ipString = [TestIPV6 getSockAddrIPString:addr];  地址转字符串
        NSLog(@"尝试连接地址 : %@", ipString);

        if (connect(s, res2->ai_addr, res2->ai_addrlen) < 0)   将创建的这个socket连接到远端服务器
        {
            cause = "connect";
            close(s); 链接服务器失败了
            s = -1;
            continue;
        }
        break;
    }

    if(s >= 0)
    {
        UIAlertView *alertview = [[UIAlertView alloc]
                                  initWithTitle:@"提示"
                                  message:@"连接成功"
                                  delegate:self
                                  cancelButtonTitle:@"取消"
                                  otherButtonTitles:@"好的", nil];
        [alertview show];
    }
    else
    {
        //没有连接上
        UIAlertView *alertview = [[UIAlertView alloc]
                                  initWithTitle:@"提示"
                                  message:@"连接失败"
                                  delegate:self
                                  cancelButtonTitle:@"取消"
                                  otherButtonTitles:@"好的", nil];
        [alertview show];
        err(1, "%s", cause);
    }
    freeaddrinfo(res1);
}

@end
1

使用这样的方式就可以连接到我的阿里云服务器了;
如果你的服务器没有域名的话就需要采用第二种方法去访问
版权声明：本文为博主原创文章，未经博主允许不得转载。	https://blog.csdn.net/joelcat/article/details/78824397