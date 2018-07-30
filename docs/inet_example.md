
网络地址（v4或者v6）转字符串？
+ (NSString *)getSockAddrIPString:(const struct sockaddr *)sa
{
    NSString *result = @"";
    sa_family_t netType = sa->sa_family;
    if(netType == AF_INET) //V4d的环境
    {
        char ipv4_str_buf[INET_ADDRSTRLEN] = {0};
        struct sockaddr_in *v4sa = (struct sockaddr_in *)sa;

        inet_ntop(AF_INET,//
                  &(v4sa->sin_addr),//struct in_addr  是src
                  ipv4_str_buf,//
                  sizeof(ipv4_str_buf));

        result = [[NSString alloc] initWithCString:ipv4_str_buf encoding:NSUTF8StringEncoding];
    }
    else if(netType == AF_INET6)
    {
        char ipv6_str_buf[INET6_ADDRSTRLEN] = {0};  //这个最大的长度有6
        struct sockaddr_in6 *v6sa = (struct sockaddr_in6 *)sa;

        inet_ntop(AF_INET6,//
                  &(v6sa->sin6_addr),//struct in6)addr  是src
                  ipv6_str_buf,//
                  sizeof(ipv6_str_buf));

        result = [[NSString alloc] initWithCString:ipv6_str_buf encoding:NSUTF8StringEncoding];
    }
    return result;
}