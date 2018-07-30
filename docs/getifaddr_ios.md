(https://blog.csdn.net/gaodeying123/article/details/51674147)
  							 				
[最新] iOS——socket适配ipv6，同时兼容ipv4
gaodeying123 2016-6-14  1	

 
-----------------------------------2016年9月23日更新--------------------------------

用这种方法来区分ipv6 和 ipv4时遇到了一个特别奇怪的问题：这种方法无法识别一些极个别的地区的4g网络，
比如吉林通化地区的移动4g。我一直没有找到原因。同时，这种方法存在一个很大的问题，
就是必须先连接网络才能识别区分，比如我连接的是www.baidu.com。鉴于上面两个原因，
于是我改变了思路：通过识别本机ip地址来区分。只要该手机连接网络了，不管是4g还是wifi，
这部手机就会被分配一个ip地址。如果是ipv4的网络，则地址是ipv4格式的（192.168.1.0），
如果是ipv6的网络，则地址是ipv6格式的（2001：2::aab1:c7d:4591:b851:8376）。
废话不多说，下面是c++识别代码，你可以改成OC的。




bool CSocketTCP::isIpv4Net(){

    bool isIPV6 = true;
    string deviceIPAddressStr = deviceIPAdress();
    string::size_type idx = deviceIPAddressStr.find(":");
    if (idx != string::npos) {
        isIPV6 = true;
    }else{
        isIPV6 = false;
    }
    return !isIPV6;
}


/*!
 * 获取当前设备ip地址
 */
string CSocketTCP::deviceIPAdress()
{
    string address = "";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;
    
    success = getifaddrs(&interfaces);
    
    if (success == 0) {  // 0 表示获取成功
        
        temp_addr = interfaces;
        while (temp_addr != NULL) {
            //NSLog(@"ifa_name===%@",[NSString stringWithUTF8String:temp_addr->ifa_name]);
            // Check if interface is en0 which is the wifi connection on the iPhone
            string ifanameStr(temp_addr->ifa_name);
            string::size_type idx = ifanameStr.find("en0");
            string::size_type idx2 = ifanameStr.find("pdp_ip0");
            if (idx != string::npos || idx2 != string::npos)
            {
                //如果是IPV4地址，直接转化
                if (temp_addr->ifa_addr->sa_family == AF_INET){
                    // Get NSString from C String
                    address = formatIPV4Address(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr);
                }
                
                //如果是IPV6地址
                else if (temp_addr->ifa_addr->sa_family == AF_INET6){
                    address = formatIPV6Address(((struct sockaddr_in6 *)temp_addr->ifa_addr)->sin6_addr);
                    string addressStr = address;
                    string uppercase4Str = addressStr.substr(0,4);
                    
                    if (!(addressStr.empty()) && !(uppercase4Str == "fe80")) break;
                }
            }
            
            temp_addr = temp_addr->ifa_next;
        }
    }
    
    //以FE80开始的地址是单播地址
    string uppercase4Str = address.substr(0,4);
    if (!(address.empty()) && !(uppercase4Str == "fe80")) {
        return address;
    } else {
        return "127.0.0.1";
    }
    freeifaddrs(interfaces);
}

string CSocketTCP::formatIPV4Address(struct in_addr ipv4Addr){
    string address = "";
    
    char dstStr[INET_ADDRSTRLEN];
    char srcStr[INET_ADDRSTRLEN];
    memcpy(srcStr, &ipv4Addr, sizeof(struct in_addr));
    if(inet_ntop(AF_INET, srcStr, dstStr, INET_ADDRSTRLEN) != NULL){
        address = dstStr;
    }
    
    return address;
}

string CSocketTCP::formatIPV6Address(struct in6_addr ipv6Addr){
    string address = "";
    char dstStr[INET6_ADDRSTRLEN];
    char srcStr[INET6_ADDRSTRLEN];
    memcpy(srcStr, &ipv6Addr, sizeof(struct in6_addr));
    if(inet_ntop(AF_INET6, srcStr, dstStr, INET6_ADDRSTRLEN) != NULL){
//        address = [NSString stringWithUTF8String:dstStr];
        address = dstStr;
    }
    
    return address;
}
-----------------------------------2016年9月23日更新--------------------------------

前些天提交代码，被苹果打回了，苹果回复：Specifically, we were unable to access the app. We've attached screenshot for your
 reference. Apps are reviewed on an IPv6 network. Please ensure that your app supports IPv6 networks, as IPv6 compatibility is required才发现从2016-6-1起，所有提交APPStore的应用必须要支持IPV6网络，关于模拟IPV6网络网上有方法讲解了（os 10.11.5），下面就说说我遇到的一些问题。

在看例子之前，先做好准备工作。确保你链接是IPv6网




 
先先看看例子

[objc] view
 plain copy
  在CODE上查看代码片派生到我的代码片
struct addrinfo *result;  
struct addrinfo *res;  
  
//判定网络类型  
int error = getaddrinfo("www.baidu.com", NULL, NULL, &result);  
if (error == 0)  
{  
    struct sockaddr_in *sa;  
    for (res = result; res != NULL; res = res->ai_next)  
    {  
        if (AF_INET6 == res->ai_addr->sa_family)  
        {  
            char buf[128] = {};  
            sa = (struct sockaddr_in*)res->ai_addr;  
            inet_ntop(AF_INET6, &((reinterpret_cast<struct sockaddr_in6*>(sa))->sin6_addr), buf, 128);  
              
            m_sock = socket(res->ai_family, res->ai_socktype, 0);  
            if (m_sock == -1) {  
                log("error socket create");  
                return;  
            }  
              
            struct sockaddr_in6 svraddr;  
            svraddr.sin6_family = AF_INET6;  
            svraddr.sin6_port = htons(port);  
            if (inet_pton(AF_INET6,buf,&svraddr.sin6_addr) < 0)  
            {  
                log("error addr");  
            }  
            int ret = connect(m_sock, (struct sockaddr*) &svraddr, sizeof(svraddr));  
            //TODO....  
            break;  
        }  
        else if (AF_INET == res->ai_addr->sa_family)  
        {  
            char buf[32] = {};  
            sa = (struct sockaddr_in*)res->ai_addr;  
            inet_ntop(AF_INET, &sa->sin_addr), buf, 32);  
              
            m_sock = socket(res->ai_family, res->ai_socktype, 0);  
            if (m_sock == -1) {  
                log("error socket create");  
                return;  
            }  
              
            struct sockaddr_in svraddr;  
            svraddr.sin_family = AF_INET;  
            svraddr.sin_addr.s_addr = inet_addr(buf);  
            svraddr.sin_port = htons(port);  
            int ret = connect(m_sock, (struct sockaddr*) &svraddr, sizeof(svraddr));  
            //TODO....  
            break;  
        }  
    }  
}  

错误1:int error
 = getaddrinfo("www.baidu.com",NULL,NULL,
 &result);没有指定端口号，和期望返回的套接字接口信息，即函数中2，3参数。

运行结果：上面的result会得到所有支持TCP UDP可用的IPv6网。所以一定要指定特定的接口信息。

（细心的伙伴就会发现ai_socktype
 不一样，这不是我们想要的。）



下面我们改下代码：

[objc] view
 plain copy
  在CODE上查看代码片派生到我的代码片
struct addrinfo addrCriteria;  
memset(&addrCriteria,0,sizeof(addrCriteria));  
addrCriteria.ai_family=AF_UNSPEC;  
addrCriteria.ai_socktype=SOCK_STREAM;  
addrCriteria.ai_protocol=IPPROTO_TCP;  
             
//判定网络类型  
int error = getaddrinfo("www.baidu.com", "1100", &addrCriteria, &result);  

我们期望的是只返回ai_socktype = SOCK_STREAM的地址信息，并不期望返回ai_socktype = SOCK_DGRAM。
错误2:2个if 中的 break；这样result地址集里用IPV6的网可能去连的是IPV4，你应该要先检查有ai_family = AF_INET6，不然地址集如果第一个地址信息时AF_INET时，那么就算有AF_INET6地址信息，也会break出来。

运行结果：现在返回的地址信息中，AF_INET6只有SOCK_STREAM的地址信息了，最好在创建socket之前检查地址集中，有没有AF_INET6或者


 
elseif (AF_INET ==
 res->ai_addr->sa_family)中的break；除掉！在没有遇到AF_INET6之前时，循环不退出。





错误3:代码是直接 run 在iPhone上是没有问题的，但是提交就一定链接不上了。用testflight软件（苹果审核也是用这个）进行测试。

解决方法：

[objc] view
 plain copy
  在CODE上查看代码片派生到我的代码片
struct sockaddr_in6 svraddr;  
memset(&svraddr, 0, sizeof(svraddr));  
svraddr.sin6_family = AF_INET6;  
svraddr.sin6_port = htons(port);  
少了一行初始化代码 memset(&svraddr,0,sizeof(svraddr)); 当时为了这句花费了半天时间，还是道行不够。
1）testflight肯定得不到任何信息，如果可以请告诉我。

2）打日志？苹果安全系数高取出日志都不知道，如果可以请告诉我。

3）开模拟器，我也没想到用模拟器可以模拟出这个情况来，Xcode自带模拟器，如果testflight连接不上，那么你的模拟器也许跟此一样，我的就是这样发现问题，并解决的。



完整事例：


[objc] view
 plain copy
  在CODE上查看代码片派生到我的代码片
struct addrinfo *result;  
struct addrinfo *res;  
  
struct addrinfo addrCriteria;  
memset(&addrCriteria,0,sizeof(addrCriteria));  
addrCriteria.ai_family=AF_UNSPEC;  
addrCriteria.ai_socktype=SOCK_STREAM;  
addrCriteria.ai_protocol=IPPROTO_TCP;  
  
//判定网络类型    指定特定接口信息  
int error = getaddrinfo("www.baidu.com", "1100", &addrCriteria, &result);  
if (error == 0)  
{  
    struct sockaddr_in *sa;  
    for (res = result; res != NULL; res = res->ai_next)  
    {  
        if (AF_INET6 == res->ai_addr->sa_family)  
        {  
            char buf[128] = {};  
            sa = (struct sockaddr_in*)res->ai_addr;  
            inet_ntop(AF_INET6, &((reinterpret_cast<struct sockaddr_in6*>(sa))->sin6_addr), buf, 128);  
              
            m_sock = socket(res->ai_family, res->ai_socktype, 0);  
            if (m_sock == -1) {  
                log("error socket create");  
                return;  
            }  
              
            struct sockaddr_in6 svraddr;  
            memset(&svraddr, 0, sizeof(svraddr)); //注意初始化  
            svraddr.sin6_family = AF_INET6;  
            svraddr.sin6_port = htons(port);  
            if (inet_pton(AF_INET6,buf,&svraddr.sin6_addr) < 0)  
            {  
                log("error addr");  
            }  
            int ret = connect(m_sock, (struct sockaddr*) &svraddr, sizeof(svraddr));  
            //TODO....遇到IPv6就退出  
            break;  
        }  
        else if (AF_INET == res->ai_addr->sa_family)  
        {  
            char buf[32] = {};  
            sa = (struct sockaddr_in*)res->ai_addr;  
            inet_ntop(AF_INET, &sa->sin_addr), buf, 32);  
              
            m_sock = socket(res->ai_family, res->ai_socktype, 0);  
            if (m_sock == -1) {  
                log("error socket create");  
                return;  
            }  
              
            struct sockaddr_in svraddr;  
            svraddr.sin_family = AF_INET;  
            svraddr.sin_addr.s_addr = inet_addr(buf);  
            svraddr.sin_port = htons(port);  
            int ret = connect(m_sock, (struct sockaddr*) &svraddr, sizeof(svraddr));  
            //TODO.... break除掉  
            //break;  
        }  
    }  
}  











[objc] view
 plain copy
  


本文引用地址
http://blog.csdn.net/msdb198901/article/details/51654852


兼容ipv6原理
http://www.ibm.com/developerworks/cn/linux/l-cn-ipv4v6-sockapp/


官方翻译文档
http://www.jianshu.com/p/54b989098537


Application Aspects of IPv6 Transition 全英文原理
http://tools.ietf.org/html/rfc4038


apple官方文档
http://developer.apple.com/library/mac/documentation/NetworkingInternetWeb/Conceptual/NetworkingOverview/UnderstandingandPreparingfortheIPv6Transition/UnderstandingandPreparingfortheIPv6Transition.html#//apple_ref/doc/uid/TP40010220-CH213-SW1