
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "common.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int g_conn_pipe_fd[2];
int g_track_pipe_fd[2];

int pipe_init(void)
{
    if (-1 == pipe2(g_conn_pipe_fd, O_CLOEXEC))
    {
        fprintf(stderr, "pipe2 failed : %m\n");
        return ERROR_FAILED;
    }

    if (-1 == pipe2(g_track_pipe_fd, O_CLOEXEC))
    {
        fprintf(stderr, "pipe2 failed : %m\n");
        return ERROR_FAILED;
    }

    return ERROR_SUCCESS;
}

void pipe_exit(void)
{
    close(g_track_pipe_fd[0]);
    close(g_track_pipe_fd[1]);

    close(g_conn_pipe_fd[0]);
    close(g_conn_pipe_fd[1]);

    return;
}