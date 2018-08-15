
#ifndef _M_CONN_RELAY_H_
#define _M_CONN_RELAY_H_

#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <uuid.h>

union ipaddr_union
{
	struct in_addr addr4;
	struct in6_addr addr6;
};

struct conn_identity
{
    union ipaddr_union _i_addr; /*NOTE internal address*/
    union ipaddr_union _o_addr; /*NOTE outside address*/
    uint16_t i_port;
    uint16_t o_port;

    int sa_f;    /*Address family. i.e. AF_INET/AF_INET6*/
    int ipproto; /*l4 protocol type. i.e. IPPROTO_TCP*/
};

struct conn_relay
{
    struct conn_identity stCid;
};

extern int conn_relay_ht_init(void);
extern void conn_relay_ht_exit(void);

extern int conn_relay_ht_get_record(const uuid_t *pUuid, struct conn_relay *pstRelay);
extern int conn_relay_ht_add_record(const uuid_t *pstUuid, const struct conn_identity *pstIdentity);
extern int conn_relay_ht_del_record(const uuid_t *uuid);


#endif /* _M_CONN_RELAY_H_ */
