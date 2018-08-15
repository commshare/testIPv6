
#ifndef _M_EPOLL_H_
#define _M_EPOLL_H_

#include "list.h"

typedef int (*epoll_callback_t)(int iEvent, int iFd);

typedef struct tagEpollConnMsg
{
    int fd;
    epoll_callback_t pfCallBack;
}EPOLL_CONN_MSG_S;

typedef struct tagEpollConnMsgNode
{
    DTQ_NODE_S stNode;
    EPOLL_CONN_MSG_S stEpollConnMsg;
}EPOLL_CONN_MSG_NODE_S;

extern void epoll_schedule(void);

extern int epoll_init(void);
extern void epoll_exit(void);


#endif /* _M_EPOLL_H_ */