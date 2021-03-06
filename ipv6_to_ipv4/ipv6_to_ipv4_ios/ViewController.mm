//
//  ViewController.m
//  ipv6_to_ipv4_ios
//
//  Created by wangyoucao577 on 1/10/17.
//  Copyright © 2017 wangyoucao577. All rights reserved.
//

#import "ViewController.h"
#include "ipv4_ipv6_interface.h"
#define HIIDO_SERVER_DOMAIN_NAME "ylog.hiido.com"
#define TT_IP_1 "183.146.210.37"
#define TT_IP_2 "58.215.52.186"
#define MY_UBUNTU "172.25.33.36"
@interface ViewController ()
@property (weak, nonatomic) IBOutlet UILabel *_version_lable;
@property (weak, nonatomic) IBOutlet UITextView *localNetTextView;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    // 显示版本号
    NSString * shortVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
    NSString * buildVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
 
    NSString *version = @"V";
    version = [version stringByAppendingFormat: @"%@.%@ \n", shortVersion, buildVersion];
    
    NSLog(@"version: %@\n", version);

    self._version_lable.text = version;
    
    // 获取并显示Local Network信息
    [self refreshLocalNetwork];
    showIp(HIIDO_SERVER_DOMAIN_NAME);
    showIp(TT_IP_1);
    showIp(TT_IP_2);
    SConnect(TT_IP_1,4950,CLI_UDP);
    char *msg="i am zhangbin";
    talker(TT_IP_1,msg);
    tcpclient_main(TT_IP_1);
  //  tcpclient_loopback();
    const char* ipv6_str ="64:ff9b::14.17.32.211"; //后面是点分十进制的
    /*
     14.17.32.211 变为
     0:0:0:0:0:ffff:e11:20d3
     */
    /*
     (1147) - <getAddrinfo_Main> ---create ipv6 socket 64:ff9b::14.17.32.211 mode 2
     (1158) - <getAddrinfo_Main> ---socket created ipv6 64:ff9b::e11:20d3 9887 7
     (1171) - <getAddrinfo_Main> ---inet_pton OK  v6 ip 64:ff9b::e11:20d3!!
     (1147) - <getAddrinfo_Main> ---create ipv6 socket 64:ff9b::14.17.32.211 mode 1
     (1158) - <getAddrinfo_Main> ---socket created ipv6 64:ff9b::e11:20d3 9887 8
     (1171) - <getAddrinfo_Main> ---inet_pton OK  v6 ip 64:ff9b::e11:20d3!
     */
    getAddrinfo_Main(ipv6_str,9887,CLI_UDP);
    getAddrinfo_Main(ipv6_str,9887,CLI_TCP);
}

- (void)refreshLocalNetwork {
    char local_net_info[10240] = {0};
    get_local_valid_net(NULL, 0, local_net_info, sizeof(local_net_info));
    NSString * local_net_nsstr = [NSString stringWithFormat:@"%s", local_net_info];
    NSLog(@"get_local_valid_net output");
    //NSLog(@"%@", local_net_nsstr);
    
    /*
     NSArray* net_lines = [local_net_nsstr componentsSeparatedByString:@";"];
     for (int i = 0; i < [net_lines count]; ++i){
     NSString* tmp_line = [net_lines objectAtIndex:i];
     NSLog(@"%@\n", tmp_line);
     }*/
    NSString * formatted_local_net = [local_net_nsstr stringByReplacingOccurrencesOfString:@";" withString:@"\n"];
    NSLog(@"%@", formatted_local_net);
    //self.local_network_label.text = formatted_local_net;
    //self.local_network_label.text = @"lo,127.0.0.1,AF_NET(2)";
    self.localNetTextView.text = formatted_local_net;

}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)touchDown:(id)sender {
    exported_test();
}

- (IBAction)refreshLocalTouchDown:(id)sender {
    [self refreshLocalNetwork];
}

@end
