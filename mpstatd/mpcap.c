
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "common.h"
#include "mthread.h"
#include "msignal.h"
#include "mpipe.h"
#include "mpacket.h"
#include "mpcap.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <pcap/pcap.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/ether.h>



#define PCAP_SNAPLEN 65535
#define PCAP_PROMISC 1
#define PCAP_TIMEOUT 1000       /* 10 ms */
#define DEFAULT_PCAP_BUFSIZE (1024 * 1024 * 64)      /* 64 Mb */

struct pcap_common_arg {
    int conn_pipe_w_fd;    //CONST
    int64_t last_ts;
    uint8_t pcap_type;
    uint32_t pcap_index;
};

struct pcap_t_arg
{
    char ifname[IFNAMSIZ]; //CONST
    unsigned int ifindex;  //CONST
    int offline;
    int running;
    int ignore;
    int id;
    pthread_t tid;

    struct pcap_common_arg common;
};

struct pcap_t_arg g_pcap_info = {};

static int packet_tcphdr_valid(const uint8_t *l4data, size_t l4len)
{
    if (l4len < sizeof(struct tcphdr)) {
        fprintf(stderr, "TCP: no valid tcp header found\n");
        return ERROR_FAILED;
    }

    const struct tcphdr *th = (const struct tcphdr *)l4data;
    size_t th_len = th->doff * 4;
    if (th_len > l4len) {
        fprintf(stderr, "TCP: no valid tcp data found\n");
        return ERROR_FAILED;
    }

    return ERROR_SUCCESS;
}

static int packet_ip_l4proto_ignore(const uint8_t *l3data, size_t l3len, uint8_t *l4proto, uint16_t *l4offset)
{
    const struct ip *h = (const struct ip *)l3data;
    unsigned int next = h->ip_p;
    size_t off = h->ip_hl << 2;

    for (; off < l3len;)
    {
        switch (next)
        {
            case IPPROTO_AH:
            {
                if (off + sizeof(struct ip6_ext) >= l3len)
                {
                    return 1;
                }
                const struct ip6_ext *ext = (struct ip6_ext *)(l3data + off);
                next = ext->ip6e_nxt;
                off += ((size_t)ext->ip6e_len + 2) << 2;

                continue;
                break;
            }
            case IPPROTO_TCP:
            {
                *l4proto = IPPROTO_TCP;
                *l4offset = off;
                if (ERROR_SUCCESS != packet_tcphdr_valid(l3data + off, l3len - off))
                {
                    return ERROR_FAILED;
                }
                else
                {
                    return ERROR_SUCCESS;
                }
                break;
            }
            default:
            {
                return ERROR_FAILED;
                break;
            }
        }
    }

    return ERROR_FAILED;
}

static int pcap_save_packet(struct pcap_common_arg *pca, const uint8_t *l3data, size_t l3len, uint8_t l3family, int direction)
{
    uint16_t l4offset = 0;
    uint8_t l4proto = 0;
    int fd;

    switch (l3family)
    {
        case AF_INET:
        {
            if (ERROR_SUCCESS != packet_ip_l4proto_ignore(l3data, l3len, &l4proto, &l4offset))
            {
                return ERROR_FAILED;
            }
            break;
        }
        default:
        {
            return ERROR_FAILED;
            break;
        }
    }

    if (IPPROTO_TCP == l4proto)
    {
        fd = pca->conn_pipe_w_fd;
    }
    else
    {
        return ERROR_FAILED;
    }

    uint8_t *sdata = malloc(l3len + sizeof(struct pcap_data_hdr));
    if (NULL == sdata)
    {
        fprintf(stderr, "malloc failed\n");
        return ERROR_FAILED;
    }

    memcpy(sdata + sizeof(struct pcap_data_hdr), l3data, l3len);
    struct pcap_data_hdr *hdr = (struct pcap_data_hdr *)sdata;
    hdr->plen = l3len;
    hdr->pcap_type = pca->pcap_type;
    hdr->pcap_index = pca->pcap_index;
    hdr->l3family = l3family;
    hdr->l4offset = l4offset;
    hdr->l4proto = l4proto;
    hdr->direction = direction;

    ssize_t nwrite = write(fd, &sdata, sizeof(uint8_t *));
    switch (nwrite)
    {
        case -1:
            fprintf(stderr, "write: %m\n");
            free(sdata);
            break;
        case sizeof(uint8_t *):
            break;
        default:
            free(sdata);
            break;
    }

    return ERROR_SUCCESS;
}

static int netdev_is_up(const char *ifname)
{
    struct ifreq ir;

    strncpy(ir.ifr_name, ifname, IFNAMSIZ);
    ir.ifr_name[IFNAMSIZ - 1] = '\0';

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        fprintf(stderr, "socket: %m\n");
        return 0;
    }

    if (ioctl(fd, SIOCGIFFLAGS, &ir) == -1) {
        fprintf(stderr, "ioctl: %m\n");
        close(fd);
        return 0;
    }
    close(fd);

    if (ir.ifr_flags & IFF_UP)
    {
        return 1;
    }

    return 0;
}

static char *get_pcap_filter_str(void)
{
	char *str = NULL;

#define L2_ADDRESS_FILTER "not (ether broadcast and multicast)"

	/*MAC Filter*/
    char *t;
	asprintf(&t, "(ether host %s)", "d4:be:d9:df:dd:37");

	char *mac_filter = t;
	if (!mac_filter)
		return NULL;

#define L3_ADDRESS_FILTER "not (ip multicast or ip6 multicast)"

	if (asprintf(&str, "(%s) and (%s) and (%s)",
	             L2_ADDRESS_FILTER,
	             mac_filter,
	             L3_ADDRESS_FILTER) == -1) {
		fprintf(stderr, "asprintf: %m");
		free(mac_filter);
		return NULL;
	}
	free(mac_filter);

	return str;
}

static pcap_t *m_pcap_open(const char *dev)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *ph;

    ph = pcap_create(dev, errbuf);
    if (!ph)
    {
        fprintf(stderr, "pcap_create: %s", errbuf);
        return NULL;
    }

    pcap_set_snaplen(ph, PCAP_SNAPLEN);
    pcap_set_promisc(ph, PCAP_PROMISC);
    pcap_set_timeout(ph, PCAP_TIMEOUT);
    pcap_set_immediate_mode(ph, 1);
    pcap_set_buffer_size(ph, DEFAULT_PCAP_BUFSIZE);

    switch (pcap_activate(ph)) {
    case 0:
        break;
    case PCAP_WARNING_PROMISC_NOTSUP:
    case PCAP_WARNING_TSTAMP_TYPE_NOTSUP:
    case PCAP_ERROR_ACTIVATED:
    case PCAP_ERROR_NO_SUCH_DEVICE:
    case PCAP_ERROR_PERM_DENIED:
    case PCAP_ERROR_PROMISC_PERM_DENIED:
    case PCAP_ERROR_RFMON_NOTSUP:
    case PCAP_ERROR_IFACE_NOT_UP:
    case PCAP_WARNING:
    case PCAP_ERROR:
    default:
        fprintf(stderr, "pcap_activate: %s", pcap_geterr(ph));
        pcap_close(ph);
        return NULL;
        break;
    }

    if (pcap_set_datalink(ph, DLT_EN10MB) == -1) {
        fprintf(stderr, "pcap_set_datalink: %s", pcap_geterr(ph));
        pcap_close(ph);
        return NULL;
    }

    /* man filter (pcap_filter_str) : http://www.tcpdump.org/manpages/pcap-filter.7.html */
    struct bpf_program bp;
    char *pcap_filter_str = get_pcap_filter_str();
    if (NULL == pcap_filter_str)
    {
        //fprintf(stderr, "get_pcap_filter_str failed\n");
        //return NULL;
    }

    fprintf(stdout, "filter-str: %s\n", pcap_filter_str);
    if (pcap_compile(ph, &bp, pcap_filter_str, 1, PCAP_NETMASK_UNKNOWN) == -1) {
        fprintf(stderr, "pcap_compile: %s", pcap_geterr(ph));
        pcap_close(ph);
        return NULL;
    }
    free(pcap_filter_str);

    if (pcap_setfilter(ph, &bp) == -1) {
        fprintf(stderr, "pcap_setfilter: %s", pcap_geterr(ph));
        pcap_close(ph);
        return NULL;
    }

    if (1 != netdev_is_up(dev))
    {
        fprintf(stderr, "netdev(%s) is not up\n", dev);
        pcap_close(ph);
        return NULL;
    }

    return ph;
}

#define SRC_P_IN DP_IN
#define SRC_P_OU DP_OUT
#define SRC_P_UN DP_UNKOWN
#define DST_P_IN DP_IN
#define DST_P_OU DP_OUT
#define DST_P_UN DP_UNKOWN

static unsigned int ehter_direction_arr[3][3] = {
	[SRC_P_UN][DST_P_UN] = DP_UNKOWN,
	[SRC_P_UN][DST_P_IN] = DP_IN,
	[SRC_P_UN][DST_P_OU] = DP_OUT,
	[SRC_P_IN][DST_P_UN] = DP_OUT,
	[SRC_P_IN][DST_P_IN] = DP_UNKOWN,
	[SRC_P_IN][DST_P_OU] = DP_OUT,
	[SRC_P_OU][DST_P_UN] = DP_IN,
	[SRC_P_OU][DST_P_IN] = DP_IN,
	[SRC_P_OU][DST_P_OU] = DP_UNKOWN
};

static int ether_ht_get_position(const struct ether_addr *e)
{
    struct ether_addr eh_addr;

    ether_aton_r("d4:be:d9:df:dd:37", &eh_addr);

    if (0 == memcmp(e, &eh_addr, sizeof(struct ether_addr)))
    {
        return SRC_P_IN;
    }

    return DST_P_UN;
}

static int ether_get_direction(struct ether_header *eh)
{
    unsigned int dst_p = ether_ht_get_position((struct ether_addr *)&eh->ether_dhost);
    unsigned int src_p = ether_ht_get_position((struct ether_addr *)&eh->ether_shost);

    return ehter_direction_arr[src_p][dst_p];
}

static void pcap_callback(uint8_t *arg, const struct pcap_pkthdr *packet_header, const uint8_t *packet_content)
{
    struct pcap_t_arg *targ = (struct pcap_t_arg *)arg;

    //fprintf(stdout, "listening on %s, capture size %u bytes\n", targ->ifname, packet_header->caplen);

    if (packet_header->caplen != packet_header->len)
    {
        fprintf(stderr, "snaplen need to be increased. Packet len: %u, caplen: %u", packet_header->len, packet_header->caplen);
        //pcap_breakloop(pstPcapInfo->pstPacpFd);
        return;
    }

    const uint8_t *l3data = packet_content + sizeof(struct ether_header);
    size_t l3len = packet_header->caplen - sizeof(struct ether_header);
    size_t padlen = 0;
    uint8_t l3family;
    struct ether_header *eth = (struct ether_header *)packet_content;
    uint16_t l3type = ntohs(eth->ether_type);
    for (;;)
    {
        switch (l3type)
        {
            case ETH_P_IP:
            {
                if (0 != packet_ip_l3_ignore(l3data, l3len, &padlen))
                {
                    return;
                }
                l3family = AF_INET;
                l3len -= padlen;
                break;
            }
            case ETH_P_8021Q: //VLAN
            case ETH_P_8021AD:
                if (0 != packet_vlan_ignore(&l3data, &l3len, &l3type))
                {
                    return;
                }
                continue;
                break;
            default:
                return;
                break;
        }

        break;
    }

    uint8_t direction = ether_get_direction(eth);
    if (DP_UNKOWN == direction)
    {
        return;
    }

    pcap_save_packet(&targ->common, l3data, l3len, l3family, direction);

    return;
}

static void *pcap_thread_exec(void *arg)
{
    pcap_t *ph;
    struct pcap_t_arg *pcap_info = arg;

    fprintf(stdout, "pthread id : %lu\n", pcap_info->tid);

    if (-1 == signal_unblock_thread_other())
    {
        pthread_exit(NULL);
    }

    ph = m_pcap_open(pcap_info->ifname);
    if (NULL == ph)
    {
        pthread_exit(NULL);
    }

    if (ERROR_SUCCESS != thread_sem_post())
    {
        fprintf(stderr, "pcap-thread-init : thread sem post failed\n");
        pcap_close(ph);
        pthread_exit(NULL);
    }

    for(;;)
    {
        if (g_thread_events != T_E_NONE)
        {
            if (g_thread_events & T_E_QUIT)
            {
                break;
            }
        }

        switch (pcap_dispatch(ph, -1, pcap_callback, arg))
        {
            case -1:
            {
                fprintf(stderr, "pcap_dispatch: %s\n", pcap_geterr(ph));
                break;
            }
            case -2:
            {
                fprintf(stderr, "pcap break loop\n");
                break;
            }
            default:
            {
                continue;
                break;
            }
        }

        break;
    }

    pcap_close(ph);
    fprintf(stdout, "pcap_thread exit\n");

    pthread_exit(NULL);
}

int pcap_thread_create(void)
{
    STRLCPY(g_pcap_info.ifname, "p0", sizeof(g_pcap_info.ifname));
    g_pcap_info.common.conn_pipe_w_fd = g_conn_pipe_fd[1];
    if (0 != pthread_create(&(g_pcap_info.tid), NULL, pcap_thread_exec, &g_pcap_info))
    {
        fprintf(stdout, "phread create failed\n");
        return ERROR_FAILED;
    }

    if (ERROR_SUCCESS != thread_sem_timedwait())
    {
        return ERROR_FAILED;
    }

    g_pcap_info.running = 1;
    if (0 != pthread_setname_np(g_pcap_info.tid, "pcap_proc"))
    {
        fprintf(stdout, "pthread_setname_np failed\n");
    }

    char thread_name[16];
    if (0 != pthread_getname_np(g_pcap_info.tid, thread_name, 16))
    {
        fprintf(stdout, "pthread_getname_np : %ld, failed\n", g_pcap_info.tid);
    }
    else
    {
        fprintf(stdout, "pthread_getname_np name : %s\n", thread_name);
    }

    return ERROR_SUCCESS;
}

int pcap_thread_close(void)
{
    fprintf(stderr, "Killing pcap thread...\n");
    if (0 == g_pcap_info.running)
    {
        return ERROR_SUCCESS;
    }

    if (0 != m_thread_kill(g_pcap_info.tid))
    {
        fprintf(stderr, "pcap_thread exit failed\n");
        return ERROR_FAILED;
    }

    g_pcap_info.tid = 0;
    g_pcap_info.running = 0;

    return ERROR_SUCCESS;
}




