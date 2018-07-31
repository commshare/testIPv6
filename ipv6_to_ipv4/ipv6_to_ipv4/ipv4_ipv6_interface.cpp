//
//  ipv4_ipv6_interface.c
//  ipv6_to_ipv4
//
//  Created by wangyoucao577 on 8/29/16.
//  Copyright © 2016 wangyoucao577. All rights reserved.
//

#include "ipv4_ipv6_interface.h"
#define LOGI printf
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <iostream>

#ifndef __APPLE__
#define IN_LINKLOCALNETNUM	(u_int32_t)0xA9FE0000 /* 169.254.0.0 */
#define IN_LINKLOCAL(i)		(((u_int32_t)(i) & IN_CLASSB_NET) == IN_LINKLOCALNETNUM)
#endif

sockaddr_in gm_sockaddrv4;
sockaddr_in6 gm_sockaddrv6;

//需要在getaddrinfo里获取
static uint32_t gm_sin6scopeid;

enum EIP_STATCK{
    E_IPSTACK_NONE  = 0 ,
    E_IPSTACK_IPV4  = 1 ,
    E_IPSTACK_IPV6 = 2,
    E_IPSTACK_DUAL = 3,
};

const char* PEER_IPV4 = "115.239.211.112";
const char* PEER_IPV6 = "2001:2::aab1:414d:8373:fe01:373e";
const char* LOCAL_IPV4 = "192.168.2.2";
const char* LOCAL_IPV6 = "2001:2::aab1:1c2d:6fd3:a33b:499b";
const int PEER_SERIVCE_PORT = 80;

#define LOG (printf("(%d) - <%s>\n",/*__FILE__,*/__LINE__,__FUNCTION__),printf)

//sockaddr得到ip地址
char * inet_ntop_ipv4_ipv6_compatible(const struct sockaddr *sa, char *s, unsigned int maxlen)
{
    memset(s, 0, maxlen);
    switch(sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, maxlen);
            break;
            
        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, maxlen);
            break;
            
        default:
            strncpy(s, "Unknown AF", maxlen);
            return NULL;
    }
    
    return s;
}

/*ip地址来判断是v4还是v6*/
int ip_str_family(char* ipstr)
{
    struct sockaddr_storage temp_sockaddr;
    memset(&temp_sockaddr, 0, sizeof(struct sockaddr_storage));

    int ret = inet_pton(AF_INET6, ipstr, &temp_sockaddr);
    if (1 == ret) { //succeed
        return AF_INET6;
    }

    ret = inet_pton(AF_INET, ipstr, &temp_sockaddr);
    if (1 == ret) { //succeed
        return AF_INET;
    }
    return -1;
}

#ifndef WIN32

char* get_local_valid_net(const char* dev_name, int dev_name_len, char* out_net_info, int max_out_len)
{
    LOG("get_local_valid_net devname %s \n",dev_name);
    if (NULL == dev_name){
        printf("\n\nget_local_net printf all ipv4 and ipv6 here, ignore LINKLOCAL\n");
    }
    char* ipstr = NULL;

    struct ifaddrs* addrs = 0;
    struct ifaddrs* head = 0;
    getifaddrs(&head);
    addrs = head;
    
    while(addrs) {
        //1, for ipv4, ignore the linklocal ip 169.254.0.0/16
        //2, for ipv6, ignore the linklocal ip fe80::...
        //3, only care about the indicated device, ignore other devices
        if (NULL == dev_name || (strncmp(dev_name, addrs->ifa_name, dev_name_len) == 0))
        {
            if (    (addrs->ifa_addr->sa_family == AF_INET && 0 == IN_LINKLOCAL(ntohl(((struct sockaddr_in *)addrs->ifa_addr)->sin_addr.s_addr)))
                 || (addrs->ifa_addr->sa_family == AF_INET6 && 0 == IN6_IS_ADDR_LINKLOCAL(&(((struct sockaddr_in6 *)addrs->ifa_addr)->sin6_addr)))){
                
                char tmp_out_line[256] = {0};
                
                // for ipv6 will get two ipv6 addresses, one of them is temporary ipv6 address
                // but seems we don't have any function to get the whether it's temporary on ios. so we just choose the first one.
                // I have tried that we can use each one of them to connect outside.
                // reference this link
                // http://stackoverflow.com/questions/17833765/detect-temporary-ipv6-address-crossplatform

                int ipstr_len = addrs->ifa_addr->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
                char * temp_ipstr = (char*)malloc(ipstr_len);
                assert(NULL != temp_ipstr);
                char* pstr = inet_ntop_ipv4_ipv6_compatible(addrs->ifa_addr, temp_ipstr, ipstr_len);
                assert(NULL != pstr);
                snprintf(tmp_out_line, sizeof(tmp_out_line), "%s,%s,%s(%d);", \
                         addrs->ifa_name, temp_ipstr, addrs->ifa_addr->sa_family == AF_INET6 ? "AF_INET6" : "AF_INET", addrs->ifa_addr->sa_family);
                if (out_net_info){
                    strcat(out_net_info, tmp_out_line); // for pass out
                }

                if (NULL != dev_name && NULL == ipstr){
                    
                    printf("%s dev_name:%s, first found indicated device-->ifa_name:%s, sa_family:%d(%s), addr->%s.\n", __func__, dev_name, \
                        addrs->ifa_name, addrs->ifa_addr->sa_family, \
                        addrs->ifa_addr->sa_family == AF_INET6 ? "AF_INET6" : "AF_INET", ipstr);
                    
                    ipstr = temp_ipstr; //pass out
                }
                else{
                    //others only printf

                    printf("%s ifa_name:%s, sa_family:%d(%s), addr->%s.\n", __func__, addrs->ifa_name, addrs->ifa_addr->sa_family, \
                        addrs->ifa_addr->sa_family == AF_INET6 ? "AF_INET6" : "AF_INET", temp_ipstr);
                    
                    free(temp_ipstr);   //don't need to pass out, free it
                }
                
                //printf("%s ifa_name:%s, ifa_family:%d, ifa_flags:%u, addr->%s. \n", __func__, addrs->ifa_name, \
                       addrs->ifa_addr->sa_family, addrs->ifa_flags, \
                       inet_ntop_ipv4_ipv6_compatible(addrs->ifa_addr, p_addr, sizeof(p_addr)));
            }
        }
        addrs = addrs->ifa_next;
    }
    freeifaddrs(head);
    
    if (NULL == dev_name){
        printf("\n\n");
    }
    return ipstr;
}

#endif


int easy_getaddrinfo(int ss_family, int sock_type, const char* ip, unsigned short port, struct addrinfo** output_res)
{
    struct addrinfo hints;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = ss_family;
    hints.ai_socktype = sock_type;
    
    if (ss_family == AF_INET6 || ss_family == AF_UNSPEC){
        //ipv4 mapped to ipv6, or maybe original ipv6
        hints.ai_flags = AI_ADDRCONFIG | AI_V4MAPPED;
    }
    
    static const char* kDefaultServiceName = "80";
    char* service_name = NULL;
    if (NULL == ip){
        hints.ai_flags |= AI_PASSIVE;   //automatically fill in the ip to INADDR_ANY or IN6ADDR_ANY_INIT
        service_name = (char*)kDefaultServiceName;
        assert(0 != port);
    }
    
    //getaddrinfo, at least one of ip and service_name should be set
    int error = getaddrinfo(ip, service_name, &hints, output_res);
    if (0 != error){
        printf("getaddrinfo failed, error code:%d, msg:%s.\n", error, gai_strerror(error));
        return error;
    }
    
    //set port
    if (0 != port){
        if (ss_family == AF_INET6){
            //NOTE!!! getaddrinfo can not specified port(such as "80") as the second parameter service_name, only service_name from /etc/services accept.
            //Environment: iOS 9 and Mac OS X 10.11
            //This is a bug impacting iOS 9 and Mac OS X 10.11. It has been fixed in iOS 10 and macOS 10.12 (as of beta 1), here is the reference link:
            //http://stackoverflow.com/questions/37386161/service-port-is-missed-when-using-getaddrinfo-to-convert-ipv4-addr-to-ipv6-addr
            
            struct sockaddr_in6 * sockaddr_v6 = (struct sockaddr_in6 *)((*output_res)->ai_addr);
            sockaddr_v6->sin6_port = htons(port);
        }else{
            struct sockaddr_in * sockaddr_v4 = (struct sockaddr_in *)((*output_res)->ai_addr);
            sockaddr_v4->sin_port = htons(port);
        }
    }
    
    return 0;
}

void getaddrinfo_behavior_individual_case(const char* case_str, int ss_family, const char* ip, unsigned short port)
{
    int err = 0;
    char* p_str = NULL;
    struct addrinfo *res0;
    char ipstr[INET6_ADDRSTRLEN] = {0};

    
    err = easy_getaddrinfo(ss_family, SOCK_STREAM, ip, port, &res0);
    assert(0 == err);
    assert(NULL == res0->ai_next);  //I want only one result.
    memset(ipstr, 0, sizeof(ipstr));
    p_str = inet_ntop_ipv4_ipv6_compatible(res0->ai_addr, ipstr, sizeof(ipstr));
    LOG("---IPSTR %s \n",p_str);
    assert(NULL != p_str);
    printf("{%s} %s %s ip->%s port->%d addr_len->%d.\n", case_str, res0->ai_family == AF_INET6 ? "AF_INET6" : "AF_INET", \
           res0->ai_socktype == SOCK_STREAM ? "SOCK_STREAM" : "SOCK_DGRAM", \
           ipstr, (int)ntohs(((struct sockaddr_in*)res0->ai_addr)->sin_port), res0->ai_addrlen);
           
    assert(res0->ai_family == ss_family || ss_family == AF_UNSPEC);
    assert(ntohs(((struct sockaddr_in*)res0->ai_addr)->sin_port) == port);
    if (res0->ai_family == AF_INET6){
        assert(res0->ai_addrlen == sizeof(struct sockaddr_in6));
    }else{
        assert(res0->ai_addrlen == sizeof(struct sockaddr_in));
    }
           
    freeaddrinfo(res0);
    res0 = NULL;

}

void getaddrinfo_behavior_test()
{
    
    //only ipv4
    getaddrinfo_behavior_individual_case("case1 client: ipv4 for local bind, ignore port", AF_INET, LOCAL_IPV4, 0);
    getaddrinfo_behavior_individual_case("case2 client: ipv4 for remote connect, local also ipv4", AF_INET, PEER_IPV4, PEER_SERIVCE_PORT);
    getaddrinfo_behavior_individual_case("case3 server: ipv4 for local bind and listening, ignore ip address", AF_INET, NULL, PEER_SERIVCE_PORT);

    //decide by system, output will be different depends on host ip stack
    // local ipv4 only: same as case 2
    // local ipv6 only: same as case 6
    // local dual: ... //TODO:
    getaddrinfo_behavior_individual_case("case4 client: ipv4 for remote connect, local depends on system, may ipv4 or ipv6", AF_UNSPEC, PEER_IPV4, PEER_SERIVCE_PORT);

    //use ipv6
    getaddrinfo_behavior_individual_case("case5 client: ipv6 for local bind. ignore port", AF_INET6, LOCAL_IPV6, 0);
    getaddrinfo_behavior_individual_case("case6 client: ipv4 for remote connect, local ipv6", AF_INET6, PEER_IPV4, PEER_SERIVCE_PORT);
    getaddrinfo_behavior_individual_case("case7 client: ipv6 for remote connect, local also ipv6", AF_INET6, PEER_IPV6, PEER_SERIVCE_PORT);
    getaddrinfo_behavior_individual_case("case8 server: ipv6 for local bind and listening, ignore ip address", AF_INET6, NULL, PEER_SERIVCE_PORT);

}

int test_tcp_connect(char* local_ip_str, const char* peer_ipv4, unsigned short port)
{
    int local_ss_family = AF_UNSPEC;
    if (NULL != local_ip_str) {
        local_ss_family = ip_str_family(local_ip_str);
    }
    printf("\ntest_tcp_connect_to_ipv4-->%s local %s peer %s %d.\n", local_ss_family == AF_INET6 ? "AF_INET6" : (local_ss_family == AF_INET ? "AF_INET" : "AF_UNSPEC"), \
        NULL == local_ip_str ? "0" : local_ip_str, peer_ipv4, (int)port);

    int err = 0;
    char* p_str = NULL;
    struct addrinfo *resPeer;
    char local_ipstr[INET6_ADDRSTRLEN] = {0};
    char peer_ipstr[INET6_ADDRSTRLEN] = {0};
    int local_port = 0;
    
    //get peer
    err = easy_getaddrinfo(local_ss_family, SOCK_STREAM, peer_ipv4, port, &resPeer);
    assert(0 == err);
    assert(NULL == resPeer->ai_next);  //I want only one result.
    p_str = inet_ntop_ipv4_ipv6_compatible(resPeer->ai_addr, peer_ipstr, sizeof(peer_ipstr));
    assert(NULL != peer_ipstr);
    printf("{%s} %s %s ip->%s port->%d addr_len->%d.\n", "peer", resPeer->ai_family == AF_INET6 ? "AF_INET6" : "AF_INET", \
           resPeer->ai_socktype == SOCK_STREAM ? "SOCK_STREAM" : "SOCK_DGRAM", \
           peer_ipstr, (int)ntohs(((struct sockaddr_in*)resPeer->ai_addr)->sin_port), resPeer->ai_addrlen);
    
    int s = socket(resPeer->ai_family, resPeer->ai_socktype, resPeer->ai_protocol);
    assert (s >= 0);
    
    int ret = 0;
    if (NULL != local_ip_str) {

        struct addrinfo *resLocal;

        //get local
        err = easy_getaddrinfo(local_ss_family, SOCK_STREAM, local_ip_str, 0, &resLocal);
        assert(0 == err);
        assert(NULL == resLocal->ai_next);  //I want only one result.
        p_str = inet_ntop_ipv4_ipv6_compatible(resLocal->ai_addr, local_ipstr, sizeof(local_ipstr));
        assert(NULL != p_str);
        printf("{%s} %s %s ip->%s port->%d addr_len->%d.\n", "local", resLocal->ai_family == AF_INET6 ? "AF_INET6" : "AF_INET", \
            resLocal->ai_socktype == SOCK_STREAM ? "SOCK_STREAM" : "SOCK_DGRAM", \
            local_ipstr, (int)ntohs(((struct sockaddr_in*)resLocal->ai_addr)->sin_port), resLocal->ai_addrlen);

        local_port = ntohs(((struct sockaddr_in*)resLocal->ai_addr)->sin_port);

        //bind socket to local
        ret = bind(s, resLocal->ai_addr, resLocal->ai_addrlen);
        if (ret != 0) {
            printf("bind failed, return %d errno %d.\n", ret, LAST_ERR);
            freeaddrinfo(resLocal);
            goto End;
        }
        LOG("----BIND TO LOCAL OK ---");
        freeaddrinfo(resLocal);
    }

    if (connect(s, resPeer->ai_addr, resPeer->ai_addrlen) < 0) {
        printf("connect failed, errno %d.\n", LAST_ERR);
        ret = -1;
        goto End;
    }

    printf("local %s %d try to connect peer %s(%s) %d succeed.\n\n\n", local_ipstr, local_port, \
           peer_ipv4, peer_ipstr, (int)ntohs(((struct sockaddr_in*)resPeer->ai_addr)->sin_port));
    
End:
#ifdef WIN32
    closesocket(s);
#else
    close(s);
#endif
    freeaddrinfo(resPeer);
    return ret;
}

void exported_test()
{

    getaddrinfo_behavior_test();
    
    static const char *WifiName = "en0" ;
    static const char *CellularName ="pdp_ip0";

#ifndef WIN32
    //print all ipv4 and ipv6 interface and addressed, ignore LINKLOCAL
    get_local_valid_net(NULL, 0, NULL, 0);


    char* ipstr = get_local_valid_net(WifiName, (int)strlen(WifiName), NULL, 0);
    if (NULL != ipstr) {
        test_tcp_connect(ipstr, PEER_IPV4, PEER_SERIVCE_PORT);
        free(ipstr);
    }

    ipstr = get_local_valid_net(CellularName, (int)strlen(CellularName), NULL, 0);
    if (NULL != ipstr) {
        test_tcp_connect(ipstr, PEER_IPV4, PEER_SERIVCE_PORT);
        free(ipstr);
    }
#endif

    //system decide test
    test_tcp_connect(NULL, PEER_IPV4, PEER_SERIVCE_PORT);
    test_tcp_connect(NULL, PEER_IPV6, PEER_SERIVCE_PORT);
}


#ifndef __APPLE__
int main()
{
#ifdef WIN32
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
        printf("Error at WSAStartup()\n");
#endif

    exported_test();


#ifdef WIN32
    WSACleanup();

    //wait before return
    printf("Please press any key to continue...");
    char a;
    scanf_s("%c", &a, (int)sizeof(a));
#endif
    return 0;
}

#endif

/* 解析域名
 ----SHOW IP FOR ylog.hiido.com
 IP addresses for ylog.hiido.com:
 
 IPv6: 2001:2:0:1baa::3d85:34a2
 IPv6: 2001:2:0:1baa::3db3:e702
 IPv6: 2001:2:0:1baa::9dff:e88f
 IPv6: 2001:2:0:1baa::24f8:130d
 IPv6: 2001:2:0:1baa::24f8:144a
 IPv6: 2001:2:0:1baa::6e35:a40a
 IPv6: 2001:2:0:1baa::705b:13b9
 IPv6: 2001:2:0:1baa::3d85:34a3
 IPv4: 112.91.19.185
 IPv4: 61.133.52.162
 IPv4: 61.133.52.163
 IPv4: 61.179.231.2
 IPv4: 157.255.232.143
 IPv4: 36.248.19.13
 IPv4: 36.248.20.74
 IPv4: 110.53.164.10
 */
/*加了map和config那俩flags之后
 
 2018-07-28 18:21:59.595 ipv6_to_ipv4_ios[363:231519] get_local_valid_net output
 2018-07-28 18:21:59.595 ipv6_to_ipv4_ios[363:231519] lo0,::1,AF_INET6(30)
 lo0,127.0.0.1,AF_INET(2)
 en0,2001:2::aab1:837:672:4104:bd47,AF_INET6(30)
 en0,2001:2::aab1:c904:df2b:1bde:ee22,AF_INET6(30)
 ----SHOW IP FOR ylog.hiido.com
 IP addresses for ylog.hiido.com:
 没有返回v4地址了么？？
 IPv6: 2001:2:0:1baa::9dff:e88f
 IPv6: 2001:2:0:1baa::3d85:34a2
 IPv6: 2001:2:0:1baa::3db3:e702
 IPv6: 2001:2:0:1baa::6e35:a40a
 IPv6: 2001:2:0:1baa::3d85:34a3
 IPv6: 2001:2:0:1baa::705b:13b9
 IPv6: 2001:2:0:1baa::24f8:130d
 IPv6: 2001:2:0:1baa::24f8:144a
 ====================error in __connection_block_invoke_2: Connection interrupted
 
 */
/*
 ====================----SHOW IP FOR 183.146.210.37
 IP addresses for 183.146.210.37:
 
 IPv6: 2001:2:0:1baa::b792:d225
 ====================error in __connection_block_invoke_2: Connection interrupted
 
 ====================----SHOW IP FOR 58.215.52.186
 IP addresses for 58.215.52.186:
 
 IPv6: 2001:2:0:1baa::3ad7:34ba
 */
/*
 去掉AI_DEFAULT     //hints.ai_flags = AI_ADDRCONFIG|AI_V4MAPPED;
 就有v4了
 
 ----SHOW IP FOR ylog.hiido.com
 IP addresses for ylog.hiido.com:
 
 IPv6: 2001:2:0:1baa::705b:13b9
 IPv6: 2001:2:0:1baa::24f8:144a
 IPv6: 2001:2:0:1baa::6e35:a40a
 IPv6: 2001:2:0:1baa::3d85:34a3
 IPv6: 2001:2:0:1baa::3db3:e702
 IPv6: 2001:2:0:1baa::9dff:e88f
 IPv6: 2001:2:0:1baa::24f8:130d
 IPv6: 2001:2:0:1baa::3d85:34a2
 IPv4: 36.248.19.13
 IPv4: 112.91.19.185
 IPv4: 61.133.52.162
 IPv4: 36.248.20.74
 IPv4: 110.53.164.10
 IPv4: 61.133.52.163
 IPv4: 61.179.231.2
 IPv4: 157.255.232.143
 ====================----SHOW IP FOR 183.146.210.37
 IP addresses for 183.146.210.37:
 
 IPv6: 2001:2:0:1baa::b792:d225
 IPv4: 183.146.210.37
 ====================----SHOW IP FOR 58.215.52.186
 IP addresses for 58.215.52.186:
 
 IPv6: 2001:2:0:1baa::3ad7:34ba
 IPv4: 58.215.52.186
 ====================error in __connection_block_invoke_2: Connection interrupted
 
 换到普通的网络：
 ====================----SHOW IP FOR 183.146.210.37
 IP addresses for 183.146.210.37:
 
 IPv4: 183.146.210.37
 ====================----SHOW IP FOR 58.215.52.186
 IP addresses for 58.215.52.186:
 
 IPv4: 58.215.52.186
 ====================
 
切到专用网络；
 
 ====================----SHOW IP FOR 183.146.210.37
 IP addresses for 183.146.210.37:
 
 IPv6: 2001:2:0:1baa::b792:d225
 ====================----SHOW IP FOR 58.215.52.186
 IP addresses for 58.215.52.186:
 
 IPv6: 2001:2:0:1baa::3ad7:34ba
 
 */

void showIp(char *hostname){
    printf("----SHOW IP FOR %s\n",hostname);
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];



    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;
    /*
     AI_V4MAPPED为了在非DNS64网络下，返回v4-mapped ipv6 address，不会返回EAI_NONAME失败，导致判断不准确。
     AI_ADDRCONFIG返回的地址是本地能够使用的（具体可以看文档下面的介绍）。
     如果有NAT64前缀的v6地址返回，证明当前网络是IPv6-only NAT64网络。
     */
    //hints.ai_flags = AI_ADDRCONFIG|AI_V4MAPPED;
    hints.ai_flags = AI_DEFAULT ;
    if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(status));
        return ;
    }
    printf("IP addresses for %s:\n\n", hostname);
    for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;
        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
            gm_sin6scopeid = ipv6->sin6_scope_id;
            LOG("GOT gm_sin6scopeid %d \n",gm_sin6scopeid);
        }
        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf(" %s: %s\n", ipver, ipstr);
    }
    freeaddrinfo(res); // free the linked list
    printf("====================");
}
//get sockaddr  ipv4, v6
void * get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void UdpBind(int sd, void * serveraddr, int len){
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    sa.sin_addr.s_addr = INADDR_ANY; // 使用我的ipv4地址
    sa6.sin6_addr = in6addr_any; //使用我的ipv6地址

    if (bind(sd,
             (struct sockaddr *)&serveraddr,
//             sizeof(serveraddr)) < 0
             len ) <0 )
    {
        printf("bind() failed %s\n",strerror(errno));
    }
}

void UdpBindLocal(int sd){
    //ipv4 还要补充
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    sa.sin_addr.s_addr = INADDR_ANY; // 使用我的ipv4地址
    
    //ipv6
    sa6.sin6_addr = in6addr_any; //使用我的ipv6地址
    sa6.sin6_port = htons(0);
    sa6.sin6_family = AF_INET6;
    sa6.sin6_flowinfo = 0 ;
    sa6.sin6_scope_id = gm_sin6scopeid;
    //这个为啥是apple专属的
    sa6.sin6_len = sizeof(sockaddr_in);

    if (bind(sd,
             (struct sockaddr *)&sa6,
            sizeof(sa6)
              ) <0 )
    {
        printf("bind() failed %s\n",strerror(errno));
    }else{
        LOG("---UdpBindLocal BIND %d ok \n",sd);
    }
}

std::string ipToString(const in6_addr &sin6addr)
{
    char ipStr[100];
    
#ifdef __APPLE__
    sprintf(ipStr, "[%hx.%hx.%hx.%hx.%hx.%hx.%d.%d.%d.%d]",
            sin6addr.__u6_addr.__u6_addr16[0], sin6addr.__u6_addr.__u6_addr16[1], sin6addr.__u6_addr.__u6_addr16[2],
            sin6addr.__u6_addr.__u6_addr16[3], sin6addr.__u6_addr.__u6_addr16[4], sin6addr.__u6_addr.__u6_addr16[5],
            sin6addr.__u6_addr.__u6_addr8[12], sin6addr.__u6_addr.__u6_addr8[13],
            sin6addr.__u6_addr.__u6_addr8[14], sin6addr.__u6_addr.__u6_addr8[15]);
#else
    sprintf(ipStr, "[%hx.%hx.%hx.%hx.%hx.%hx.%d.%d.%d.%d]",
            sin6addr.s6_addr16[0], sin6addr.s6_addr16[1], sin6addr.s6_addr16[2], sin6addr.s6_addr16[3],
            sin6addr.s6_addr16[4], sin6addr.s6_addr16[5],
            sin6addr.s6_addr[12], sin6addr.s6_addr[13], sin6addr.s6_addr[14], sin6addr.s6_addr[15]);
#endif
    return std::string(ipStr);
}

std::string ipToString(uint32_t ip)
{
    char ipStr[100];
    sprintf(ipStr, "[%d.%d.%d.%d]", ip & 0xFF, (ip & 0xFF00) >> 8, (ip & 0xFF0000) >> 16, ip >> 24);
    return std::string(ipStr);
}

 std::string getsockaddrname( in6_addr ipv6_addr )
{
#ifdef __APPLE__
    bool isUseSockv6 = true;
#else
    bool isUseSockv6 = true;  //(m_elocalstack == E_IPSTACK_IPV6);
#endif
    
    if (isUseSockv6)
    {
       // return ipToString(gm_sockaddrv6.sin6_addr);
        return ipToString(ipv6_addr);
    }
    else
    {
        return ipToString(gm_sockaddrv4.sin_addr.s_addr);
    }
}
int UdpSend(int sock,char *msg,int size, void *svraddr,socklen_t svraddr_len)
{
    int n = (int)::sendto(sock,msg,strlen(msg)+1,0,(struct sockaddr *)&svraddr,size);
    if(n == -1)
    {
       printf("send to server : %d %s\n",size,strerror(errno));
    }
    printf("send to server : %d \n",size);
    int nret = sendto(sock, msg,size,0,(struct sockaddr*)svraddr, svraddr_len);
    if(nret== -1  )
    {
        printf("send to fail \n");
        return false;
    }
    printf("conncet to ok sock %d \n",sock);

    return nret;
}
void UdpClose(int sock){
    ::close(sock);
}
bool SConnect(const char* domain, unsigned short port)
{
    //连接ip
    char ip[128];
    memset(ip, 0, sizeof(ip));
    strcpy(ip, domain); //默认domain就是ip吧
    
    void* svraddr = nullptr;
    int error=-1, svraddr_len;
    bool ret = true;
    struct sockaddr_in svraddr_4;
    struct sockaddr_in6 svraddr_6;
    
    //获取网络协议
    struct addrinfo *result;
    error = getaddrinfo(ip, NULL, NULL, &result);
    const struct sockaddr *sa = result->ai_addr;
    socklen_t maxlen = 128;
    /*创建socket*/
    int m_sock = -1 ;//初始化为-1？
    switch(sa->sa_family)
    {
        case AF_INET://ipv4
            printf("create ipv4 socket  %s\n",domain);
            if ((m_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("socket create v4 failed");
                ret = false;
                break;
            }
            if(inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                         ip, maxlen) == NULL)
            {
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
            printf("create ipv6 socket %s \n",domain);
            if ((m_sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
            {
                perror("socket create v6 failed");
                ret = false;
                break;
            }
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                      ip, maxlen);
            
            printf("socket created ipv6 %s\n",ip);
            
            bzero(&svraddr_6, sizeof(svraddr_6));
            svraddr_6.sin6_family = AF_INET6;
            svraddr_6.sin6_port = htons(port);
            
            if ( inet_pton(AF_INET6, ip, &svraddr_6.sin6_addr) < 0 )
            {
                perror(ip);
                ret = false;
                break;
            }
            svraddr_len = sizeof(svraddr_6);
            svraddr = &svraddr_6;
            break;
            
        default:
        {
            printf("Unknown AF\ns");
            ret = false;
        }
    }
    freeaddrinfo(result);
    if(!ret)
    {
        fprintf(stderr , "Cannot Connect the server!\n");
        return false;
    }
    printf("----connect ok %s \n",domain);
    printf("nat64sample \n");
    nat64Sample();
    printf("----bind --%d- \n",m_sock);
   // UdpBind(m_sock, svraddr, sizeof(svraddr));
    UdpBindLocal(m_sock);
    char * msg = "thechinaOne";
    printf("----send ---%s \n",msg);
    UdpSend(m_sock,msg,sizeof(msg),svraddr, sizeof(svraddr));
    return true;
}



 int getaddrinfo_compat(
    const char * hostname,
    const char * servname,
    const struct addrinfo * hints,
    struct addrinfo ** res
)
{
    int    err;
    int    numericPort;

    // If we're given a service name and it's a numeric string, set `numericPort` to that,
    // otherwise it ends up as 0.

    numericPort = servname != NULL ? atoi(servname) : 0;

    // Call `getaddrinfo` with our input parameters.

    err = getaddrinfo(hostname, servname, hints, res);

    // Post-process the results of `getaddrinfo` to work around   <rdar://problem/26365575>.

    if ( (err == 0) && (numericPort != 0) ) {
        for (const struct addrinfo * addr = *res; addr != NULL; addr = addr->ai_next) {
            in_port_t *    portPtr;

            switch (addr->ai_family) {
                case AF_INET: {
                    portPtr = &((struct sockaddr_in *) addr->ai_addr)->sin_port;
                } break;
                case AF_INET6: {
                    portPtr = &((struct sockaddr_in6 *) addr->ai_addr)->sin6_port;
                } break;
                default: {
                    portPtr = NULL;
                } break;
            }
            if ( (portPtr != NULL) && (*portPtr == 0) ) {
                *portPtr = htons(numericPort);
            }
        }
    }
    return err;
}

void nat64Sample(){
    //NAT64 address sample
    //address init
    const char* ipv6_str ="64:ff9b::14.17.32.211";
    //in6_addr是ipv6的地址
    in6_addr ipv6_addr = {0};
    int v6_r = inet_pton(AF_INET6, ipv6_str, &ipv6_addr); //ipv6 字符串地址转为int类型
    /*set it global 不行呢 */
    //gm_sockaddrv6 = ipv6_addr;
    /* TODO 注意，字节序有发生改变么？6400.9bff ？
     --inet_pton- 64:ff9b::14.17.32.211-to [6400.9bff.0.0.0.0.14.17.32.211] ret  1
     */
    LOG("-1-inet_pton- %s-to %s ret  %d \n",ipv6_str,getsockaddrname(ipv6_addr).c_str(),v6_r);
    //V6 地址
    sockaddr_in6 v6_addr = {0};
    v6_addr.sin6_family = AF_INET6;
    v6_addr.sin6_port = htons(80);  //v6的端口，怎么才能任意以下呢？
    v6_addr.sin6_addr = ipv6_addr;  //这个是转过来的int类型的
    
    //socket connect
    int v6_sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    //std::string
    char * v6_error;
    if (0 != connect(v6_sock, (sockaddr*)&v6_addr, 28)) //使用这个tcp socket 去连接远端v6服务器
    {
        v6_error = strerror(errno);
    }
    
    //get local ip
    sockaddr_in6 v6_local_addr = {0};
    socklen_t v6_local_addr_len = 28;
    char v6_str_local_addr[64] = {0};
    getpeername(v6_sock, (sockaddr*)&v6_local_addr, &v6_local_addr_len);
    inet_ntop(v6_local_addr.sin6_family, &v6_local_addr.sin6_addr, v6_str_local_addr, 64);
    LOG("local ip %s \n",v6_str_local_addr);  //!!!! 没有输出啊,字符 array数组是这么打印么？
    close(v6_sock);}
