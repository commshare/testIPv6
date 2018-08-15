
#include "common.h"
#include "msignal.h"
#include "mthread.h"
#include "mepoll.h"
#include "mpipe.h"
#include "mpcap.h"
#include "mconn_relay.h"
#include "mconn.h"
#include "mtrack.h"

#include <stdio.h>


int global_init(void)
{
    int iRet;

    iRet = conn_relay_ht_init();
    if (ERROR_SUCCESS != iRet)
    {
        return ERROR_FAILED;
    }

    iRet = conn_track_ht_init();
    if (ERROR_SUCCESS != iRet)
    {
        return ERROR_FAILED;
    }

    iRet = conn_index_ht_init();
    if (ERROR_SUCCESS != iRet)
    {
        return ERROR_FAILED;
    }

    iRet = pipe_init();
    if (ERROR_SUCCESS != iRet)
    {
        return ERROR_FAILED;
    }

    return ERROR_SUCCESS;
}

void global_exit(void)
{
    pipe_exit();
    conn_index_ht_exit();
    conn_track_ht_exit();
    conn_relay_ht_exit();

    return;
}

static int main_init(void)
{
    if (ERROR_SUCCESS != global_init())
    {
        return ERROR_FAILED;
    }

    if (ERROR_SUCCESS != signal_init())
    {
        return ERROR_FAILED;
    }

    if (ERROR_SUCCESS != thread_init())
    {
        return ERROR_FAILED;
    }

    if (ERROR_SUCCESS != epoll_init())
    {
        return ERROR_FAILED;
    }

    if (ERROR_SUCCESS != track_thread_create())
    {
        fprintf(stdout, "track create failed\n");
        return ERROR_FAILED;
    }

    if (ERROR_SUCCESS != conn_thread_create())
    {
        fprintf(stdout, "conn create failed\n");
        return ERROR_FAILED;
    }

    if (ERROR_SUCCESS != pcap_thread_create())
    {
        fprintf(stdout, "pcap create failed\n");
        return ERROR_FAILED;
    }

    return ERROR_SUCCESS;
}

static int main_exit()
{
    int iRet;

    iRet = pcap_thread_close();

    iRet |= conn_thread_close();

    iRet |= track_thread_close();

    epoll_exit();
    thread_exit();
    signal_exit();
    global_exit();

    return iRet;
}

int main(int argc __attribute__ ((unused)), char *argv[] __attribute__ ((unused)))
{
    int iRet;

    g_process_pid = getpid();

    if (ERROR_SUCCESS != signal_block_thread_all())
    {
        return ERROR_FAILED;
    }

    iRet = main_init();
    if (ERROR_SUCCESS == iRet)
    {
        if (ERROR_SUCCESS == signal_unblock_thread_main())
        {
            /* main event proc */
            epoll_schedule();
        }
    }

    iRet = main_exit();

    fprintf(stdout, "main process exit\n");
    return iRet;
}