
#include <stddef.h>

#include "list.h"

void DTQ_Init(DTQ_HEAD_S *pstList)
{
    pstList->stHead.pstNext = &pstList->stHead;
    pstList->stHead.pstPrev = &pstList->stHead;

    return;
}

void DTQ_NodeInit(DTQ_NODE_S *pstNode)
{
    pstNode->pstNext = NULL;
    pstNode->pstPrev = NULL;

    return;
}

DTQ_NODE_S* DTQ_First(DTQ_HEAD_S *pstList)
{
    DTQ_NODE_S *pstNode = pstList->stHead.pstNext;

    if (pstNode == &(pstList->stHead))
    {
        return NULL;
    }

    return pstNode;
}

DTQ_NODE_S* DTQ_Last(DTQ_HEAD_S *pstList)
{
    DTQ_NODE_S *pstNode = pstList->stHead.pstPrev;

    if (pstNode == &(pstList->stHead))
    {
        return NULL;
    }

    return pstNode;
}

void DTQ_AddAfter(DTQ_NODE_S *pstPrev, DTQ_NODE_S *pstInst)
{
    pstInst->pstPrev = pstPrev;
    pstInst->pstNext = pstPrev->pstNext;
    pstPrev->pstNext = pstInst;
    pstInst->pstNext->pstPrev = pstInst;

    return;
}

void DTQ_AddBefore(DTQ_NODE_S *pstNext, DTQ_NODE_S *pstInst)
{
    pstInst->pstPrev = pstNext->pstPrev;
    pstInst->pstNext = pstNext;
    pstInst->pstPrev->pstNext = pstInst;
    pstInst->pstNext->pstPrev = pstInst;

    return;
}

void DTQ_Del(DTQ_NODE_S *pstNode)
{
    pstNode->pstPrev->pstNext = pstNode->pstNext;
    pstNode->pstNext->pstPrev = pstNode->pstPrev;

    return;
}

void DTQ_AddHead(DTQ_HEAD_S *pstList, DTQ_NODE_S *pstNode)
{
    DTQ_AddAfter(&(pstList->stHead), pstNode);

    return;
}

void DTQ_AddTail(DTQ_HEAD_S *pstList, DTQ_NODE_S *pstNode)
{
    DTQ_AddBefore(&(pstList->stHead), pstNode);

    return;
}

void DTQ_Append(DTQ_HEAD_S *pstDstList, DTQ_HEAD_S *pstSrcList)
{
    if (1 != DTQ_IsEmpty(pstSrcList))
    {
        pstSrcList->stHead.pstNext->pstPrev = pstDstList->stHead.pstPrev;
        pstSrcList->stHead.pstPrev->pstNext = pstDstList->stHead.pstPrev->pstNext;
        pstDstList->stHead.pstPrev->pstNext = pstSrcList->stHead.pstNext;
        pstDstList->stHead.pstPrev = pstSrcList->stHead.pstPrev;
        DTQ_Init(pstSrcList);
    }

    return;
}

void DTQ_FreeAll(DTQ_HEAD_S *pstList, free_info_t pfFree)
{
    DTQ_NODE_S *pstCurNode;
    DTQ_NODE_S *pstNextNode;

    DTQ_FOREACH_SAFE(pstList, pstCurNode, pstNextNode)
    {
        pfFree(pstCurNode);
    }

    DTQ_Init(pstList);

    return;
}
