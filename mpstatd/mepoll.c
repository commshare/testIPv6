
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>

#include "common.h"
#include "list.h"
#include "mepoll.h"
#include "msignal.h"

#define MAXEVENTS 20

int g_epoll_fd = 0;

static void epoll_proc_Event(int iEpollCount, struct epoll_event astEpEvent[])
{
    int iNum;
    struct epoll_event *pstEpEvent;
    EPOLL_CONN_MSG_S *pstFMDConnMsg;

    /* 处理epoll事件 */
    for (iNum = 0; iNum < iEpollCount; iNum++)
    {
        /* 获取event data */
        pstEpEvent = &(astEpEvent[iNum]);
        pstFMDConnMsg = (EPOLL_CONN_MSG_S *)(pstEpEvent->data.ptr);

        /* 使用注册的回调函数进行处理 */
        if (NULL != pstFMDConnMsg->pfCallBack)
        {
            (void)pstFMDConnMsg->pfCallBack(pstEpEvent->events, pstFMDConnMsg->fd);
        }
    }

    return;
}

void epoll_schedule(void)
{
    int iEpollCount;

    struct epoll_event stEpEvent;

    for(;;)
    {
        if (T_E_NONE != g_thread_events)
        {
            if (g_thread_events & T_E_QUIT)
            {
                break;
            }
        }

        /* epoll wait for events */
        iEpollCount = epoll_wait(g_epoll_fd, &stEpEvent, MAXEVENTS, -1);
        if (-1 == iEpollCount)
        {
            if (errno == EINTR)
            {
                continue;
            }

            fprintf(stderr, "epoll wait failed : %m\n");
            break;
        }

        epoll_proc_Event(iEpollCount, &stEpEvent);
    }

    return;
}

int epoll_init(void)
{
    int epoll_fd;

    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (-1 == epoll_fd)
    {
        fprintf(stderr, "epoll create failed\n");
        return ERROR_FAILED;
    }

    g_epoll_fd = epoll_fd;

    return ERROR_SUCCESS;
}

void epoll_exit(void)
{
    if (0 != g_epoll_fd)
    {
        close(g_epoll_fd);
        g_epoll_fd = 0;
    }

    return;
}
