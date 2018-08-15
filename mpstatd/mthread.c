
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "common.h"
#include "mthread.h"
#include "msignal.h"

#include <stdio.h>
#include <time.h>
#include <errno.h>

#define THREAD_CREATE_TIMEOUT 10

sem_t g_threads_sem;

int thread_init(void)
{
    if (-1 == sem_init(&g_threads_sem, 0, 0))
    {
        fprintf(stderr, "sem init failed : %m\n");
        return ERROR_FAILED;
    }

    return ERROR_SUCCESS;
}

void thread_exit(void)
{
    sem_destroy(&g_threads_sem);

    return;
}

int thread_sem_timedwait(void)
{
    struct timespec ts = {0};

    ts.tv_sec = time(NULL) + THREAD_CREATE_TIMEOUT + 1;
    if (0 != sem_timedwait(&g_threads_sem, &ts))
    {
        fprintf(stderr, "sem timedwait failed: %m");
        return ERROR_FAILED;
    }

    return ERROR_SUCCESS;
}

int thread_sem_post(void)
{
    if (0 != sem_post(&g_threads_sem))
    {
        fprintf(stderr, "sem post failed: %m");
        return ERROR_FAILED;
    }

    return ERROR_SUCCESS;
}

int m_thread_kill(pthread_t tid)
{
    int ret;
    ret = pthread_tryjoin_np(tid, NULL);
    switch (ret) {
    case 0:
        return 0;
        break;
    case EBUSY:
        break;
    default:
        fprintf(stderr, "pthread_tryjoin_np: %m\n");
        return 0;
        break;
    }

    ret = pthread_kill(tid, SIGRT_TERM);
    if (ret != 0)
    {
        fprintf(stderr, "pthread_kill: %m\n");
        return -1;
    }

    struct timespec ts = {0};
    ts.tv_sec = time(NULL) + THREAD_QUIT_WAIT_SEC + 1;
    ret = pthread_timedjoin_np(tid, NULL, &ts);
    if (ret != 0)
    {
        fprintf(stderr, "pthread timedjoin np failed: %m\n");
        return -1;
    }

    return 0;
}
