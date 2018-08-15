
#ifndef _M_SEQLIST_H_
#define _M_SEQLIST_H_

#include "common.h"
#include "list.h"

typedef struct tagseqInfo
{
    uint8_t *pdata;
    uint32_t seq;
    uint32_t seq_n;
}SEQ_INFO_S;

typedef struct tagseqInfoNode
{
    DTQ_NODE_S stNode;
    SEQ_INFO_S stSeqInfo;
}SEQ_INFO_NODE_S;

extern int seqlist_node_add(uint8_t *pdata, DTQ_HEAD_S *pstList);
extern int seqlist_node_get(DTQ_HEAD_S *pstList, uint32_t ack_seq, DTQ_HEAD_S *pstSequentList);

#endif /* _M_SEQLIST_H_ */
