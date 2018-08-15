
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "common.h"
#include "mthread.h"
#include "mpipe.h"
#include "msignal.h"
#include "mpcap.h"
#include "mconn.h"
#include "mconn_relay.h"
#include "mtrack.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <poll.h>
#include <errno.h>
#include <glib.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

typedef struct conn_t_arg
{
    int epoll_fd;
    int running;
    pthread_t tid;
}CONN_T_INFO_S;

CONN_T_INFO_S g_conn_info;

static GHashTable *conn_index_ht = NULL;


static int iphdr_get_addr(const uint8_t *pL3data, struct conn_identity *pstCidentity, int direction)
{
    const struct ip *iph = (const struct ip *)pL3data;

    switch (direction)
    {
        case DP_OUT:
        {
            pstCidentity->_i_addr.addr4.s_addr = iph->ip_src.s_addr;
            pstCidentity->_o_addr.addr4.s_addr = iph->ip_dst.s_addr;
            break;
        }
        case DP_IN:
        {
            pstCidentity->_i_addr.addr4.s_addr = iph->ip_dst.s_addr;
            pstCidentity->_o_addr.addr4.s_addr = iph->ip_src.s_addr;
            break;
        }
        default:
        {
            fprintf(stderr, "Invalid packet direction\n");
            return ERROR_FAILED;
            break;
        }
    }


    return ERROR_SUCCESS;;
}

static int iphdr_get_port(const uint8_t *pL4data, struct conn_identity *pstCidentity, int direction)
{
    const struct tcphdr *th = (const struct tcphdr *)pL4data;

    switch (direction)
    {
        case DP_OUT:
        {
            pstCidentity->i_port = ntohs(th->th_sport);
            pstCidentity->o_port = ntohs(th->th_dport);
            break;
        }
        case DP_IN:
        {
            pstCidentity->i_port = ntohs(th->th_dport);
            pstCidentity->o_port = ntohs(th->th_sport);
            break;
        }
        default:
        {
            fprintf(stderr, "Invalid packet direction\n");
            return ERROR_FAILED;
            break;
        }
    }

    return ERROR_SUCCESS;
}

static int conn_get_identity(const struct pcap_data_hdr *pcap_hdr, struct conn_identity *pstCidentity)
{
    uint8_t *pL3data;
    uint8_t *pL4data;

    pL3data = (uint8_t *)pcap_hdr + sizeof(struct pcap_data_hdr);
    pL4data = pL3data + pcap_hdr->l4offset;

    switch (pcap_hdr->l3family)
    {
        case AF_INET:
            pstCidentity->sa_f = AF_INET;
            if (ERROR_SUCCESS != iphdr_get_addr(pL3data, pstCidentity, pcap_hdr->direction))
            {
                return ERROR_FAILED;
            }
            break;
        case AF_INET6:
            fprintf(stderr, "AF_INET6 : get addr failed : t3family = %d\n", pcap_hdr->l3family);
            return ERROR_FAILED;
            break;
        default:
            fprintf(stderr, "get addr failed : t3family = %d\n", pcap_hdr->l3family);
            return ERROR_FAILED;
            break;
    }

    switch (pcap_hdr->l4proto)
    {
        case IPPROTO_TCP:
            pstCidentity->ipproto = IPPROTO_TCP;
            if (ERROR_SUCCESS != iphdr_get_port(pL4data, pstCidentity, pcap_hdr->direction))
            {
                return ERROR_FAILED;
            }
            break;
        default:
            fprintf(stderr, "get port failed : l4proto = %d\n", pcap_hdr->l4proto);
            return ERROR_FAILED;
            break;
    }

    return ERROR_SUCCESS;
}

struct conn_index *conn_index_ht_get_record(const struct conn_identity *pcidentity)
{
    return g_hash_table_lookup(conn_index_ht, pcidentity);
}

struct conn_index *conn_index_ht_add_record(const struct conn_identity *pCidentity, int64_t ts)
{
    struct conn_identity *k = malloc(sizeof(struct conn_identity));
    if (NULL == k)
    {
        fprintf(stderr, "malloc failed : %m\n");
        return NULL;
    }

    struct conn_index *v = calloc(1, sizeof(struct conn_index));
    if (NULL == k)
    {
        fprintf(stderr, "malloc failed : %m\n");
        return NULL;
    }

    memcpy(k, pCidentity, sizeof(struct conn_identity));
    uuid_generate_time(v->uuid);
    v->init_ts = ts;
    v->last_ts = ts;

    g_hash_table_replace(conn_index_ht, k, v);

    return v;
}

static int tcp_is_new(const uint8_t *l4data)
{
    const struct tcphdr *th = (const struct tcphdr *)l4data;

    if ((1 == th->ack) && (1 == th->syn))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static int conn_is_new(const uint8_t *l4data, int l4proto)
{
    switch (l4proto)
    {
        case IPPROTO_TCP:
            return tcp_is_new(l4data);
            break;
        default:
            break;
    }

    fprintf(stderr, "conn_is_new failed : l4proto = %d\n", l4proto);

    return 0;

}

static guint conn_index_ht_hash(gconstpointer key)
{
	struct conn_identity *k = (struct conn_identity *)key;
	uint32_t h;

#if __BYTE_ORDER == __BIG_ENDIAN
	h = ((uint32_t)k->i_port << 16) + k->o_port;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	h = ((uint32_t)k->o_port << 16) + k->i_port;
#endif

	switch (k->sa_f) {
	case AF_INET:
		h ^= k->_i_addr.addr4.s_addr;
		h ^= k->_o_addr.addr4.s_addr;
		break;
	default:
		break;
	}

	return h;
}

static gboolean conn_index_ht_equal(gconstpointer a, gconstpointer b)
{
	struct conn_identity *ka = (struct conn_identity *)a;
	struct conn_identity *kb = (struct conn_identity *)b;

	if (ka->ipproto != kb->ipproto)
		return 0;
	if (ka->sa_f != kb->sa_f)
		return 0;

	if (ka->i_port != kb->i_port || ka->o_port != kb->o_port)
		return 0;

	switch (ka->sa_f) {
	case AF_INET:
		if (ka->_i_addr.addr4.s_addr == kb->_i_addr.addr4.s_addr &&
		    ka->_o_addr.addr4.s_addr == kb->_o_addr.addr4.s_addr)
			break;
		else
			return 0;
		break;
	default:
		break;
	}

	return 1;
}

int conn_index_ht_init(void)
{
    conn_index_ht = g_hash_table_new_full(conn_index_ht_hash, conn_index_ht_equal, free, free);
    if (NULL == conn_index_ht)
    {
        fprintf(stderr, "ghash-table-new failed\n");
        return ERROR_FAILED;
    }

    return ERROR_SUCCESS;
}

void conn_index_ht_exit(void)
{
    if (NULL != conn_index_ht)
    {
        g_hash_table_destroy(conn_index_ht);
    }

    return;
}

static int conn_send_track_msg(struct track_msg *msg)
{
    int wfd = g_track_pipe_fd[1];

    for (;;)
    {
        ssize_t nw = write(wfd, msg, sizeof(struct track_msg));
        switch (nw)
        {
            case sizeof(struct track_msg):
                //fprintf(stderr, "conn write pipe : %zd\n", nw);
                break;
            case -1:
                if (errno == EINTR)
                {
                    continue;
                }
                return ERROR_FAILED;
                break;
            default:
                return ERROR_FAILED;
                break;
        }

        break;
    }

    return ERROR_SUCCESS;
}

static int conn_pdata_parse(uint8_t *pdata, int *is_free_pdata)
{
    struct pcap_data_hdr *pcap_hdr = (struct pcap_data_hdr *)pdata;
    *is_free_pdata = 1;

    struct conn_identity cidentity;
    if (ERROR_SUCCESS != conn_get_identity(pcap_hdr, &cidentity))
    {
        fprintf(stderr, "conn_get_identity failed\n");
        return ERROR_FAILED;
    }

    uint8_t *l4data = pdata + sizeof(struct pcap_data_hdr) + pcap_hdr->l4offset;
    struct conn_index *cindex = conn_index_ht_get_record(&cidentity);
    if (NULL == cindex)
    {
        if (0 == conn_is_new(l4data, cidentity.ipproto))
        {
            fprintf(stdout, "conn_is not new, return, src-port : %d, dst-port = %d\n", cidentity.i_port, cidentity.o_port);
            return ERROR_SUCCESS;
        }

        fprintf(stdout, "conn_index_ht_add_record, src-port : %d, dst-port = %d................................\n", cidentity.i_port, cidentity.o_port);
        cindex = conn_index_ht_add_record(&cidentity, pcap_hdr->ts);
        if (NULL == cindex)
        {
            fprintf(stderr, "conn_index_ht_add_record failed\n");
            return ERROR_FAILED;
        }
    }
    else
    {
         //fprintf(stdout, "conn_index_ht_get_record success...\n");
    }

    cindex->last_ts = pcap_hdr->ts;

    struct conn_relay stRelay;
    memset(&stRelay, 0, sizeof(struct conn_relay));
    int iRet = conn_relay_ht_get_record((const uuid_t *)&cindex->uuid, &stRelay);
    if (0 == iRet)
    {
        conn_relay_ht_add_record((const uuid_t *)&cindex->uuid, &cidentity);
    }
    else if (-1 == iRet)
    {
        /* delte resource relay,index,track */
    }

    struct track_msg msg;
    msg.data = pdata;

    if (NULL == msg.data)
    {
        fprintf(stderr, "msg.data send NULL\n");
    }

    uuid_copy(msg.uuid, cindex->uuid);
    if (ERROR_SUCCESS != conn_send_track_msg(&msg))
    {
        fprintf(stderr, "conn_send_track_msg failed\n");
        return ERROR_FAILED;
    }

    *is_free_pdata = 0;

    return ERROR_SUCCESS;
}

static int conn_poll_pdata_handle(int fd)
{
    uint8_t *buf[100];
    ssize_t nread;

    for(;;)
    {
        nread = read(fd, buf, sizeof(buf));
        switch (nread)
        {
            case -1:
            {
                if (errno == EINTR)
                {
                    continue;
                }

                fprintf(stderr, "read failed : %m\n");
                return ERROR_FAILED;
                break;
            }
            case 0:
            {
                fprintf(stderr, "read : no data read in\n");
                return ERROR_FAILED;
                break;
            }
            default:
            {
                //fprintf(stdout, "conn pipe read : nread = %zd\n", nread);
                break;
            }
        }

        break;
    }

    if (0 != (nread % sizeof(uint8_t *)))
    {
        fprintf(stderr, "Not enough data read from capture thread: %ld", nread);
        return ERROR_FAILED;
    }

    int num = nread / sizeof(uint8_t *);
    for (int i = 0; i < num; i++)
    {
        uint8_t *pdata = buf[i];
        int is_free_pdata = 1;

        if (ERROR_SUCCESS != conn_pdata_parse(pdata, &is_free_pdata))
        {
            fprintf(stderr, "conn pdata parse failed\n");
        }

        if (is_free_pdata)
        {
            free(pdata);
        }
    }

    return ERROR_SUCCESS;
}

static void *conn_thread_exec(void *arg __attribute__ ((unused)))
{
    if (-1 == signal_unblock_thread_other())
    {
        pthread_exit(NULL);
    }

    if (ERROR_SUCCESS != thread_sem_post())
    {
        fprintf(stderr, "pcap-thread-init : thread sem post failed\n");
        pthread_exit(NULL);
    }

    struct pollfd pfd = {
        .fd = g_conn_pipe_fd[0],
        .events = POLLIN
    };

    for(;;)
    {
        if (T_E_NONE != g_thread_events)
        {
            if (g_thread_events & T_E_QUIT)
            {
                break;
            }
        }

        switch (poll(&pfd, POLLIN, -1))
        {
            case -1:
            {
                if (errno == EINTR)
                {
                    continue;
                }

                fprintf(stderr, "conn poll failed : %m\n");
                g_thread_events |= T_E_QUIT;
                break;
            }
            case 0:
            {
                fprintf(stderr, "poll: timeout\n");
                continue;
                break;
            }
            default:
            {
                break;
            }
        }

        if (ERROR_SUCCESS != conn_poll_pdata_handle(pfd.fd))
        {
            break;
        }
    }

    fprintf(stdout, "conn_thread exit\n");
    pthread_exit(NULL);
}

int conn_thread_create(void)
{
    if (0 != pthread_create(&(g_conn_info.tid), NULL, conn_thread_exec, NULL))
    {
        fprintf(stdout, "phread create failed\n");
        return ERROR_FAILED;
    }

    if (ERROR_SUCCESS != thread_sem_timedwait())
    {
        return ERROR_FAILED;
    }

    g_conn_info.running = 1;
    pthread_setname_np(g_conn_info.tid, "conn_proc");

    return ERROR_SUCCESS;
}

int conn_thread_close(void)
{
    fprintf(stderr, "Killing conn thread...\n");
    if (0 == g_conn_info.running)
    {
        return ERROR_SUCCESS;
    }

    if (0 != m_thread_kill(g_conn_info.tid))
    {
        fprintf(stderr, "conn_thread exit failed\n");
        return ERROR_FAILED;
    }

    g_conn_info.tid = 0;
    g_conn_info.running = 0;

    return ERROR_SUCCESS;
}



