
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "common.h"
#include "mseqlist.h"
#include "mpcap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

int seqlist_node_add(uint8_t *pdata, DTQ_HEAD_S *pstList)
{
    SEQ_INFO_NODE_S *pstNodeTmp = NULL;

    SEQ_INFO_NODE_S *pstSeqInfoNode = malloc(sizeof(SEQ_INFO_NODE_S));
    if (NULL == pstSeqInfoNode)
    {
        fprintf(stderr, "malloc failed:%m\n");
        return ERROR_FAILED;
    }

    struct pcap_data_hdr *ph = (struct pcap_data_hdr *)pdata;
    struct tcphdr *th = (struct tcphdr *)(pdata + sizeof(struct pcap_data_hdr) + ph->l4offset);

    memset(pstSeqInfoNode, 0, sizeof(SEQ_INFO_NODE_S));
    pstSeqInfoNode->stSeqInfo.seq = ntohl(th->seq);
    pstSeqInfoNode->stSeqInfo.pdata = pdata;
    pstSeqInfoNode->stSeqInfo.seq_n = pstSeqInfoNode->stSeqInfo.seq + (ph->plen - ph->l5offset);

    if (1 == DTQ_IsEmpty(pstList))
    {
        DTQ_AddTail(pstList, &pstSeqInfoNode->stNode);
        fprintf(stderr, "add first : seq = 0x%08X, seq_n = 0x%08X, datalen = %u\n", pstSeqInfoNode->stSeqInfo.seq, pstSeqInfoNode->stSeqInfo.seq_n, (ph->plen - ph->l5offset));
        return ERROR_SUCCESS;
    }

    SEQ_INFO_NODE_S *pstNodeLast = DTQ_ENTRY(DTQ_Last(pstList), __typeof__(*pstNodeLast), stNode);
    if (pstSeqInfoNode->stSeqInfo.seq > pstNodeLast->stSeqInfo.seq)
    {
        DTQ_AddAfter(&pstNodeLast->stNode, &pstSeqInfoNode->stNode);
        fprintf(stderr, "add after : seq = 0x%08X, seq_n = 0x%08X, datalen = %u\n", pstSeqInfoNode->stSeqInfo.seq, pstSeqInfoNode->stSeqInfo.seq_n, (ph->plen - ph->l5offset));
    }
    else if (pstSeqInfoNode->stSeqInfo.seq == pstNodeLast->stSeqInfo.seq)
    {
        DTQ_AddAfter(&pstNodeLast->stNode, &pstSeqInfoNode->stNode);
        DTQ_Del(&pstNodeLast->stNode);
        fprintf(stderr, "add replace last : seq = 0x%08X, seq_n = 0x%08X, datalen = %u\n", pstSeqInfoNode->stSeqInfo.seq, pstSeqInfoNode->stSeqInfo.seq_n, (ph->plen - ph->l5offset));
    }
    else
    {
        DTQ_FOREACH_ENTRY(pstList, pstNodeTmp, stNode)
        {
            if (pstSeqInfoNode->stSeqInfo.seq < pstNodeTmp->stSeqInfo.seq)
            {
                DTQ_AddBefore(&pstNodeTmp->stNode, &pstSeqInfoNode->stNode);
                fprintf(stderr, "add insert : seq = 0x%08X, seq_n = 0x%08X, datalen = %u\n", pstSeqInfoNode->stSeqInfo.seq, pstSeqInfoNode->stSeqInfo.seq_n, (ph->plen - ph->l5offset));
                break;
            }
            else if (pstSeqInfoNode->stSeqInfo.seq == pstNodeTmp->stSeqInfo.seq)
            {
                DTQ_AddBefore(&pstNodeTmp->stNode, &pstSeqInfoNode->stNode);
                DTQ_Del(&pstNodeTmp->stNode); /* free dependents */
                fprintf(stderr, "add replace : seq = 0x%08X, seq_n = 0x%08X, datalen = %u\n", pstSeqInfoNode->stSeqInfo.seq, pstSeqInfoNode->stSeqInfo.seq_n, (ph->plen - ph->l5offset));
                break;
            }
        }
    }

    return ERROR_SUCCESS;
}

int seqlist_node_get(DTQ_HEAD_S *pstList, uint32_t ack_seq, DTQ_HEAD_S *pstSequentList)
{
    SEQ_INFO_NODE_S *pstNodeTmp = NULL;

    if (1 == DTQ_IsEmpty(pstList))
    {
        fprintf(stdout, "Get-list is empty\n");
        return ERROR_FAILED;
    }

    SEQ_INFO_NODE_S *pstNodeLast = DTQ_ENTRY(DTQ_Last(pstList), __typeof__(*pstNodeLast), stNode);
    if (pstNodeLast->stSeqInfo.seq_n < ack_seq)
    {
        fprintf(stderr, "Error : find (list-last=0x%08x < ack_seq = 0x%08x), ignore the track\n", pstNodeLast->stSeqInfo.seq_n, ack_seq);
        return ERROR_FAILED;
    }

    DTQ_FOREACH_ENTRY(pstList, pstNodeTmp, stNode)
    {
        if (pstNodeTmp->stSeqInfo.seq_n == ack_seq)
        {
            DTQ_Del(&pstNodeTmp->stNode);
            DTQ_AddTail(pstSequentList, &pstNodeTmp->stNode);
            fprintf(stdout, "Success find (ack_seq == sequent-list-last = 0x%08x)\n", ack_seq);
            break;
        }
        else if (pstNodeTmp->stSeqInfo.seq_n > ack_seq)
        {
            DTQ_Del(&pstNodeTmp->stNode);
            DTQ_AddTail(pstSequentList, &pstNodeTmp->stNode);
            fprintf(stderr, "Success find (ack_seq=0x%08x < sequent-list=0x%08x, continue)\n", ack_seq, pstNodeTmp->stSeqInfo.seq_n);
            break;
        }
    }

    return ERROR_SUCCESS;
}




