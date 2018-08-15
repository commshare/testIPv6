
#ifndef _LIST_H_
#define _LIST_H_

typedef struct tagDTQ_NODE
{
    struct tagDTQ_NODE *pstPrev;
    struct tagDTQ_NODE *pstNext;
}DTQ_NODE_S;

typedef struct tagHeadNode
{
    DTQ_NODE_S stHead;
}DTQ_HEAD_S;

typedef void (*free_info_t)(void *);

static inline int DTQ_IsEmpty(DTQ_HEAD_S *pstList)
{
    if (pstList->stHead.pstNext == &pstList->stHead)
    {
        return 1;
    }

    return 0;
}

static inline int DTQ_IsEndOfQ(DTQ_HEAD_S *pstList, DTQ_NODE_S *pstNode)
{
    if (DTQ_IsEmpty(pstList))
    {
        return 1;
    }

    if (NULL == pstNode)
    {
        return 1;
    }

    return (pstNode == &(pstList->stHead));
}

static inline DTQ_NODE_S* DTQ_Next(DTQ_NODE_S *pstNode)
{
    return (pstNode->pstNext);
}

static inline DTQ_NODE_S* DTQ_Prev(DTQ_NODE_S *pstNode)
{
    return (pstNode->pstPrev);
}

#define DTQ_ENTRY(ptr, type, member) \
        (type *)((char *)ptr - offsetof(type, member))

#define DTQ_FOREACH(pstList, pstNode) \
        for ((pstNode) = (pstList)->stHead.pstNext; \
            ((pstNode) != &((pstList)->stHead)); \
            (pstNode = pstNode->pstNext))

#define DTQ_FOREACH_SAFE(pstList, pstNode, pstNextNode) \
        for ((pstNode) = (pstList)->stHead.pstNext; \
            (((pstNode) != &(pstList)->stHead)) && ({(pstNextNode) = pstNode->pstNext; 1;}); \
            (pstNode) = (pstNextNode))

#define DTQ_ENTRY_FIRST(pstList, type, member) \
        ({DTQ_NODE_S *pstNode__Tmp__Mx = DTQ_First(pstList); \
        (NULL == pstNode__Tmp__Mx) ? NULL : DTQ_ENTRY(pstNode__Tmp__Mx, type, member);})

#define DTQ_ENTRY_LAST(pstList, type, member) \
        ({DTQ_NODE_S *pstNode__Tmp__Mx = DTQ_Last(pstList); \
         (NULL == pstNode__Tmp__Mx) ? NULL : DTQ_ENTRY(pstNode__Tmp__Mx, type, member);})

#define DTQ_ENTRY_PREV(pstList, pstEntry, member) \
        (DTQ_IsEndOfQ(pstList, (NULL == (pstEntry) ? NULL : DTQ_Prev(&((pstEntry)->member)))) ? \
         NULL : \
         DTQ_ENTRY(DTQ_Prev(&((pstEntry)->member)), __typeof__(*(pstEntry)), member))

/* walk */
#define DTQ_FOREACH_ENTRY(pstList, pstEntry, member) \
        for ((pstEntry) = DTQ_ENTRY((pstList)->stHead.pstNext, __typeof__(*(pstEntry)), member); \
             ((&(pstEntry)->member != &(pstList)->stHead) || ({pstEntry = NULL; 0;})); \
             (pstEntry) = DTQ_ENTRY((pstEntry)->member.pstNext, __typeof__(*(pstEntry)), member))

#define DTQ_FOREACH_ENTRY_SAFE(pstList, pstEntry, pstNextEntry, member) \
        for((pstEntry) = DTQ_ENTRY(&(pstList)->stHead.pstNext, __typeof__(*(pstEntry)), member); \
            ((&(pstEntry)->member != &(pstList)->stHead) && \
            ({(pstNextEntry) = DTQ_ENTRY((pstEntry)->member.pstNext, __typeof__(*(pstEntry)), member); 1;})) || \
            ({pstEntry = NULL; 0;}); \
            (pstEntry) = (pstNextEntry))

#define DTQ_FOREACH_ENTRY_REVERSE(pstList, pstEntry, member) \
        for ((pstEntry) = DTQ_ENTRY_LAST(pstList, __typeof__(*(pstEntry)), member); \
             NULL != (pstEntry); \
             (pstEntry) = DTQ_ENTRY_PREV(pstList, pstEntry, member))

#define DTQ_FOREACH_ENTRY_REVERSE_SAFE(pstList, pstEntry, pstPrevEntry, member) \
        for ((pstEntry) = DTQ_ENTRY_LAST(pstList, __typeof__(*(pstEntry)), member); \
             (NULL != (pstEntry)) && \
             ({(pstPrevEntry) = DTQ_ENTRY_PREV(pstList, pstEntry, member); 1;}); \
             (pstEntry) = (pstPrevEntry))

/* extern */
extern void DTQ_Init(DTQ_HEAD_S *pstList);
extern void DTQ_NodeInit(DTQ_NODE_S *pstNode);
extern DTQ_NODE_S* DTQ_First(DTQ_HEAD_S *pstList);
extern DTQ_NODE_S* DTQ_Last(DTQ_HEAD_S *pstList);
extern void DTQ_AddAfter(DTQ_NODE_S *pstPrev, DTQ_NODE_S *pstInst);
extern void DTQ_AddBefore(DTQ_NODE_S *pstNext, DTQ_NODE_S *pstInst);
extern void DTQ_Del(DTQ_NODE_S *pstNode);
extern void DTQ_AddHead(DTQ_HEAD_S *pstList, DTQ_NODE_S *pstNode);
extern void DTQ_AddTail(DTQ_HEAD_S *pstList, DTQ_NODE_S *pstNode);
extern void DTQ_Append(DTQ_HEAD_S *pstDstList, DTQ_HEAD_S *pstSrcList);
extern void DTQ_FreeAll(DTQ_HEAD_S *pstDstList, free_info_t pfFree);


#endif/* _LIST_H_ */


