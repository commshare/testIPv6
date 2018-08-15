#ifndef _M_PROTOCOL_H_
#define _M_PROTOCOL_H_ 1

#define PSD_PROTO_NIL    0
#define PSD_PROTO_IGN    1
#define PSD_PROTO_HTTP   2
#define PSD_PROTO_FTPC   3
#define PSD_PROTO_FTPD   4
#define PSD_PROTO_SMTP   5
#define PSD_PROTO_NNTP   6
#define PSD_PROTO_NBT    7  /*NetBIOS Session Service, IPv4 Only*/
#define PSD_PROTO_MSDS   8  /*microsoft-ds, port 445*/
#define PSD_PROTO_SSL    9
#define PSD_PROTO_POP3   10
#define PSD_PROTO_IMAP   11
#define PSD_PROTO_ICAP   12
#define PSD_PROTO_SSH    13
#define PSD_PROTO_LDAP   14
#define PSD_PROTO_SOCKS4 15
#define PSD_PROTO_SOCKS5 16
#define PSD_PROTO_HTTP2  17
#define PSD_PROTO_VNC    18
#define PSD_PROTO_RDP    19
#define PSD_PROTO_PPTP   20
#define PSD_PROTO_DNS    21
#define PSD_PROTO_NBNS   22 /*NetBIOS Name Service, IPv4 Only*/
#define PSD_PROTO_NBDGM  23 /*NetBIOS Datagram, IPv4 Only*/
#define PSD_PROTO_NTP    24
#define PSD_PROTO_RADIUS 25
#define PSD_PROTO_L2TP   26
#define PSD_PROTO_TEREDO 27
#define PSD_PROTO_IRC    28
#define PSD_PROTO_DCC    29
#define PSD_PROTO_BT     30 /*Bittorrent*/
#define PSD_PROTO_uTP    31
#define PSD_PROTO_DHT    32
//empty    33
#define PSD_PROTO_IKE    34
#define PSD_PROTO_STUN   35
#define PSD_PROTO_SSDP   36
#define PSD_PROTO_AFP    37
#define PSD_PROTO_SNMP   38
#define PSD_PROTO_eMule  39
#define PSD_PROTO_SIP    40
#define PSD_PROTO_SPICE  41
#define PSD_PROTO_XMPP   42
#define PSD_PROTO_Portmap   43
#define PSD_PROTO_NFS       44

#define PSD_PROTO_DDNS_ORAY 45 //DDNS protocol for oray.com

#define PSD_PROTO_M_LOW     46
#define PSD_PROTO_M_SMB     (PSD_PROTO_M_LOW + 0) /*Meta protocol for Protocol Config, not for type check*/
#define PSD_PROTO_M_WebMAIL (PSD_PROTO_M_LOW + 1) /*Meta protocol for webmail*/
#define PSD_PROTO_MAX       (PSD_PROTO_M_LOW + 2) /*MAX protocol we can get from psd_protocol_tcp_next_type*/

#define PSD_PROTO_E_SUBP   256 /*below is sub protocols*/
#define PSD_PROTO_E_CIFS   257
#define PSD_PROTO_E_SMB2   258
#define PSD_PROTO_E_SMB2E  259
#define PSD_PROTO_E_SSL20  260
#define PSD_PROTO_E_SSL30  261
#define PSD_PROTO_E_TLS10  262
#define PSD_PROTO_E_TLS11  263
#define PSD_PROTO_E_TLS12  264

#define PSD_PERR_NOERROR    0
#define PSD_PERR_CONTINUE   1
#define PSD_PERR_RECHECK    2
#define PSD_PERR_IGNORE     3
#define PSD_PERR_TCP_PROXY  4


#endif /* _M_PROTOCOL_H_ */