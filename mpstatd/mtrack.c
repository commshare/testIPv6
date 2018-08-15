
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "common.h"
#include "mthread.h"
#include "msignal.h"
#include "mpipe.h"
#include "mpcap.h"
#include "mtrack.h"
#include "mseqlist.h"

#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <pthread.h>
#include <glib.h>
#include <stdlib.h>
#include <netinet/tcp.h>


typedef struct track_t_arg
{
    int epoll_fd;
    int running;
    pthread_t tid;
}TRACK_T_INFO_S;

TRACK_T_INFO_S g_track_info;
static GHashTable *g_conn_track_ht = NULL;

static guint conn_track_ht_hash(gconstpointer key)
{
    uint32_t *k = (uint32_t *)key;

    return k[0];
}

static gboolean conn_track_ht_equal(gconstpointer a, gconstpointer b)
{
    uuid_t *ka = (uuid_t *)a;
	uuid_t *kb = (uuid_t *)b;

	if (uuid_compare(*ka, *kb) == 0)
		return 1;
	else
		return 0;
}

int conn_track_ht_init(void)
{
    g_conn_track_ht = g_hash_table_new_full(conn_track_ht_hash, conn_track_ht_equal, free, free);
    if (NULL == g_conn_track_ht)
    {
        fprintf(stderr, "g_hash_table_new_full failed : %m\n");
        return ERROR_FAILED;
    }

    return ERROR_SUCCESS;
}

void conn_track_ht_exit(void)
{
    if (NULL != g_conn_track_ht)
    {
        g_hash_table_destroy(g_conn_track_ht);
    }

    return;
}

static struct conn_track *conn_track_ht_get_record(const uuid_t * pstUuid)
{
    return g_hash_table_lookup(g_conn_track_ht, pstUuid);
}

static struct conn_track *conn_track_ht_add_record(const uuid_t * pstUuid, const struct conn_identity *pstCid)
{
    uuid_t *k = malloc(sizeof(uuid_t));
    if (NULL == k)
    {
        fprintf(stderr, "malloc failed : %m\n");
        return NULL;
    }

    struct conn_track *v = malloc(sizeof(struct conn_track));
    if (NULL == k)
    {
        fprintf(stderr, "malloc failed : %m\n");
        free(k);
        return NULL;
    }

    memset(v, 0, sizeof(struct conn_track));
    uuid_copy(*k, *pstUuid);
    memcpy(&v->cid, pstCid, sizeof(struct conn_identity));
    DTQ_Init(&v->i_list);
    DTQ_Init(&v->o_list);

    g_hash_table_replace(g_conn_track_ht, k, v);

    return v;
}

static void conn_track_ht_del_record(const uuid_t *pstUuid)
{
    g_hash_table_remove(g_conn_track_ht, pstUuid);
    g_conn_track_ht = NULL;

    return;
}

static void track_del_record(const uuid_t *pstUuid)
{
    conn_track_ht_del_record(pstUuid);
    conn_relay_ht_del_record(pstUuid);

    return;
}

static void ordered_pkt_handle(struct conn_track *pstTrack, const uuid_t *pstUuid, uint8_t  direction, DTQ_HEAD_S *pstSequent_list)
{
    SEQ_INFO_NODE_S *pstNodeTmp = NULL;

    DTQ_FOREACH_ENTRY(pstSequent_list, pstNodeTmp, stNode)
    {
        fprintf(stdout, "%d : ordered_pkt_handle : 0x%08x\n", direction, pstNodeTmp->stSeqInfo.seq_n);
    }


    return;
}

static void track_tcp_pkt_handle(struct conn_track *pstTrack, uint8_t *pdata, const uuid_t *pstUuid)
{
    int iRet;
    int pdata_free = 0;
    uint32_t ack_seq;
    //uint32_t seq;
    DTQ_HEAD_S sequent_list;

    DTQ_Init(&sequent_list);
    struct pcap_data_hdr *ph = (struct pcap_data_hdr *)pdata;
    struct tcphdr *th = (struct tcphdr *)(pdata + sizeof(struct pcap_data_hdr) + ph->l4offset);

    ph->l5offset = ph->l4offset + th->doff * 4;

    //seq = ntohl(th->seq);
    ack_seq = ntohl(th->ack_seq);

    fprintf(stdout, "ph->direction = %u, packet-ack-seq = 0x%08x, th->doff = %u\n", ph->direction, ack_seq, th->doff * 4);

    switch (ph->direction)
    {
        case DP_IN:
        {
            /* save o_list */
            if (ph->plen != ph->l5offset)
            {
                iRet = seqlist_node_add(pdata, &pstTrack->i_list);
                if (ERROR_SUCCESS != iRet)
                {
                    fprintf(stderr, "seqlist_node_add failed\n");
                    pdata_free = 1;
                }
            }
            else
            {
                fprintf(stdout, "data-len is 0\n");
                pdata_free = 1;
            }

            /* get o_list */
            iRet = seqlist_node_get(&pstTrack->o_list, ack_seq, &sequent_list);
            if (ERROR_SUCCESS != iRet)
            {
                fprintf(stdout, "seqlist_node_get null\n");
                free(pdata);
                return;
            }

            if (1 != DTQ_IsEmpty(&sequent_list))
            {
                ordered_pkt_handle(pstTrack, pstUuid, ph->direction, &sequent_list);
            }
            else
            {
                fprintf(stderr, "1 not process ordered_pkt_handle\n");
            }

            break;
        }
        case DP_OUT:
        {
            /* save o_list */
            if (ph->plen != ph->l5offset)
            {
                iRet = seqlist_node_add(pdata, &pstTrack->o_list);
                if (ERROR_SUCCESS != iRet)
                {
                    fprintf(stderr, "seqlist_node_add failed\n");
                    pdata_free = 1;
                }
            }
            else
            {
                fprintf(stdout, "data-len is 0\n");
                pdata_free = 1;
            }

            /* get i_list */
            iRet = seqlist_node_get(&pstTrack->i_list, ack_seq, &sequent_list);
            if (ERROR_SUCCESS != iRet)
            {
                //fprintf(stderr, "seqlist_node_get failed\n");
                free(pdata);
                return;
            }

            if (1 != DTQ_IsEmpty(&sequent_list))
            {
                fprintf(stderr, "2 process ordered_pkt_handle\n");
                ordered_pkt_handle(pstTrack, pstUuid, ph->direction, &sequent_list);
            }
            else
            {
                fprintf(stderr, "2 not process ordered_pkt_handle\n");
            }

            break;
        }
    }

    if (1 == pdata_free)
    {
        free(pdata);
        return;
    }

    return;
}

static int track_pdata_parse(struct track_msg *ptm)
{
    int iRet;
    uint8_t *pdata = ptm->data;
    const uuid_t *pstUuid = (const uuid_t *)&ptm->uuid;
    struct conn_relay stRelay;
    struct conn_track *pstTrack;

    pstTrack = conn_track_ht_get_record(pstUuid);
    if (NULL == pstTrack)
    {
        memset(&stRelay, 0, sizeof(struct conn_relay));
        iRet = conn_relay_ht_get_record(pstUuid, &stRelay);
        if (1 == iRet)
        {
            pstTrack = conn_track_ht_add_record(pstUuid, &stRelay.stCid);
            if (NULL == pstTrack)
            {
                fprintf(stderr, "conn_track_ht_add_record failed\n");
                free(ptm->data);
                return ERROR_FAILED;
            }
            else
            {
                fprintf(stderr, "Track : add record success\n");
            }
        }
        else
        {
            fprintf(stderr, "Track : No such conn found in conn_relay_ht\n");
            free(ptm->data);
            return ERROR_FAILED;
        }
    }

    struct pcap_data_hdr *ph = (struct pcap_data_hdr *)pdata;
    switch (ph->l4proto)
    {
        case IPPROTO_TCP:
            track_tcp_pkt_handle(pstTrack, pdata, pstUuid);
            break;
        default:
            free(ptm->data);
            return ERROR_FAILED;
            break;
    }

    if (0)
    {
        track_del_record(pstUuid);
    }

    return ERROR_SUCCESS;
}

static int track_poll_pdata_handle(int fd)
{
    struct track_msg tm[100];
    ssize_t nread;

    for(;;)
    {
        nread = read(fd, tm, sizeof(tm));
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
                break;
            }
        }

        break;
    }

    fprintf(stderr, "--------------------------\n");
    fprintf(stderr, "track pipe read : %zd \n", nread);
    if (0 != (nread % sizeof(struct track_msg)))
    {
        fprintf(stderr, "Not enough data read from capture thread: %ld", nread);
        return ERROR_FAILED;
    }

    int num = nread / sizeof(struct track_msg);
    for (int i = 0; i < num; i++)
    {
        struct track_msg *ptm = tm + i;

        if (ERROR_SUCCESS != track_pdata_parse(ptm))
        {
            fprintf(stderr, "track pdata parse failed\n");
        }
    }

    return ERROR_SUCCESS;
}

static void *track_thread_exec(void *arg __attribute__ ((unused)))
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
        .fd = g_track_pipe_fd[0],
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

        if (ERROR_SUCCESS != track_poll_pdata_handle(pfd.fd))
        {
            break;
        }
    }

    fprintf(stdout, "track_thread exit\n");
    pthread_exit(NULL);
}

int track_thread_create(void)
{
    if (0 != pthread_create(&(g_track_info.tid), NULL, track_thread_exec, NULL))
    {
        fprintf(stdout, "phread create failed\n");
        return ERROR_FAILED;
    }

    if (ERROR_SUCCESS != thread_sem_timedwait())
    {
        return ERROR_FAILED;
    }

    g_track_info.running = 1;
    pthread_setname_np(g_track_info.tid, "track_proc");

    return ERROR_SUCCESS;
}

int track_thread_close(void)
{
    fprintf(stderr, "Killing track thread...\n");
    if (0 == g_track_info.running)
    {
        return ERROR_SUCCESS;
    }

    if (0 != m_thread_kill(g_track_info.tid))
    {
        fprintf(stderr, "track_thread exit failed\n");
        return ERROR_FAILED;
    }

    g_track_info.tid = 0;
    g_track_info.running = 0;

    return ERROR_SUCCESS;
}