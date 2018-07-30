/* 这个是我本地的ifconfig，服务器识别到的是172.25.33.8
 
 /Users/lijin/Documents/zqyaccsdk/ipv6tt/ipv6_to_ipv4
 bogon:ipv6_to_ipv4 lijin$ ifconfig
 lo0: flags=8049<UP,LOOPBACK,RUNNING,MULTICAST> mtu 16384
 options=1203<RXCSUM,TXCSUM,TXSTATUS,SW_TIMESTAMP>
 inet 127.0.0.1 netmask 0xff000000
 inet6 ::1 prefixlen 128
 inet6 fe80::1%lo0 prefixlen 64 scopeid 0x1
 nd6 options=201<PERFORMNUD,DAD>
 gif0: flags=8010<POINTOPOINT,MULTICAST> mtu 1280
 stf0: flags=0<> mtu 1280
 XHC20: flags=0<> mtu 0
 XHC0: flags=0<> mtu 0
 XHC1: flags=0<> mtu 0
 en2: flags=8963<UP,BROADCAST,SMART,RUNNING,PROMISC,SIMPLEX,MULTICAST> mtu 1500
 options=60<TSO4,TSO6>
 ether ba:00:44:f8:93:04
 media: autoselect <full-duplex>
 status: inactive
 en3: flags=8963<UP,BROADCAST,SMART,RUNNING,PROMISC,SIMPLEX,MULTICAST> mtu 1500
 options=60<TSO4,TSO6>
 ether ba:00:44:f8:93:01
 media: autoselect <full-duplex>
 status: inactive
 en1: flags=8963<UP,BROADCAST,SMART,RUNNING,PROMISC,SIMPLEX,MULTICAST> mtu 1500
 options=60<TSO4,TSO6>
 ether ba:00:44:f8:93:00
 media: autoselect <full-duplex>
 status: inactive
 en4: flags=8963<UP,BROADCAST,SMART,RUNNING,PROMISC,SIMPLEX,MULTICAST> mtu 1500
 options=60<TSO4,TSO6>
 ether ba:00:44:f8:93:05
 media: autoselect <full-duplex>
 status: inactive
 en0: flags=8b63<UP,BROADCAST,SMART,RUNNING,PROMISC,ALLMULTI,SIMPLEX,MULTICAST> mtu 1500
 ether 8c:85:90:3c:54:e6
 inet 169.254.193.238 netmask 0xffff0000 broadcast 169.254.255.255
 nd6 options=201<PERFORMNUD,DAD>
 media: autoselect
 status: active
 p2p0: flags=8802<BROADCAST,SIMPLEX,MULTICAST> mtu 2304
 ether 0e:85:90:3c:54:e6
 media: autoselect
 status: inactive
 awdl0: flags=8902<BROADCAST,PROMISC,SIMPLEX,MULTICAST> mtu 1484
 ether be:64:ea:5a:f5:bd
 nd6 options=201<PERFORMNUD,DAD>
 media: autoselect
 status: inactive
 bridge0: flags=8863<UP,BROADCAST,SMART,RUNNING,SIMPLEX,MULTICAST> mtu 1500
 options=63<RXCSUM,TXCSUM,TSO4,TSO6>
 ether ba:00:44:f8:93:00
 Configuration:
 id 0:0:0:0:0:0 priority 0 hellotime 0 fwddelay 0
 maxage 0 holdcnt 0 proto stp maxaddr 100 timeout 1200
 root id 0:0:0:0:0:0 priority 0 ifcost 0 port 0
 ipfilter disabled flags 0x2
 member: en1 flags=3<LEARNING,DISCOVER>
 ifmaxaddr 0 port 11 priority 0 path cost 0
 member: en2 flags=3<LEARNING,DISCOVER>
 ifmaxaddr 0 port 9 priority 0 path cost 0
 member: en3 flags=3<LEARNING,DISCOVER>
 ifmaxaddr 0 port 10 priority 0 path cost 0
 member: en4 flags=3<LEARNING,DISCOVER>
 ifmaxaddr 0 port 12 priority 0 path cost 0
 nd6 options=201<PERFORMNUD,DAD>
 media: <unknown type>
 status: inactive
 utun0: flags=8051<UP,POINTOPOINT,RUNNING,MULTICAST> mtu 2000
 inet6 fe80::4ea3:809b:78d7:cc31%utun0 prefixlen 64 scopeid 0x11
 nd6 options=201<PERFORMNUD,DAD>
 en7: flags=8863<UP,BROADCAST,SMART,RUNNING,SIMPLEX,MULTICAST> mtu 1500
 options=4<VLAN_MTU>
 ether 00:0e:c6:d6:34:89
 inet6 fe80::1402:a1a3:c0c9:4097%en7 prefixlen 64 secured scopeid 0x7
 inet 172.25.33.8 netmask 0xffffff00 broadcast 172.25.33.255   识别到的地址是这个，
 nd6 options=201<PERFORMNUD,DAD>
 media: autoselect (100baseTX <full-duplex,flow-control>)
 status: active
 en6: flags=8863<UP,BROADCAST,SMART,RUNNING,SIMPLEX,MULTICAST> mtu 1500
 ether ac:de:48:00:11:22
 inet6 fe80::aede:48ff:fe00:1122%en6 prefixlen 64 scopeid 0x8
 nd6 options=201<PERFORMNUD,DAD>
 media: autoselect
 status: active
 en8: flags=8863<UP,BROADCAST,SMART,RUNNING,SIMPLEX,MULTICAST> mtu 1500
 ether 86:8e:0c:9c:e2:54
 inet6 fe80::844:bef7:22d1:ef97%en8 prefixlen 64 secured scopeid 0x13
 inet 169.254.239.3 netmask 0xffff0000 broadcast 169.254.255.255
 nd6 options=201<PERFORMNUD,DAD>
 media: autoselect (100baseTX <full-duplex>)
 status: active
 bridge100: flags=8a63<UP,BROADCAST,SMART,RUNNING,ALLMULTI,SIMPLEX,MULTICAST> mtu 1500
 options=3<RXCSUM,TXCSUM>
 ether ba:00:44:8f:3d:64
 inet6 2001:2:0:aab1::1 prefixlen 64
 inet6 fe80::b800:44ff:fe8f:3d64%bridge100 prefixlen 64 scopeid 0x12
 inet6 2001:2::aab1:1041:5755:a444:cb2f prefixlen 64 autoconf secured
 inet6 2001:2::aab1:ddae:2e57:5289:f716 prefixlen 64 autoconf temporary
 Configuration:
 id 0:0:0:0:0:0 priority 0 hellotime 0 fwddelay 0
 maxage 0 holdcnt 0 proto stp maxaddr 100 timeout 1200
 root id 0:0:0:0:0:0 priority 0 ifcost 0 port 0
 ipfilter disabled flags 0x2
 member: en0 flags=3<LEARNING,DISCOVER>
 ifmaxaddr 0 port 13 priority 0 path cost 0
 nd6 options=201<PERFORMNUD,DAD>
 media: autoselect
 status: active

 */