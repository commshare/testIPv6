
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "common.h"
#include "msignal.h"

#include <stdio.h>


_Thread_local THREAD_EVENT_E g_thread_events = T_E_NONE;

int signal_block_thread_all(void)
{
    int ret;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGRT_TERM);

    ret = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "pthread_sigmask: %m\n");
        return -1;
    }

    return 0;
}

int signal_unblock_thread_main(void)
{
    int ret;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);

    ret = pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_sigmask: %m\n");
        return -1;
    }

    return 0;
}

int signal_unblock_thread_other(void)
{
    int ret;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGRT_TERM);

    ret = pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_sigmask: %m");
        return -1;
    }

	return 0;
}

static void sigrt_term_action(int sig __attribute__ ((unused)), siginfo_t *info, void *unused __attribute__ ((unused)))
{
    if (g_process_pid == info->si_pid)
    {
        g_thread_events |= T_E_QUIT;
        fprintf(stdout, "SIGRT_TERM signal received from PID %d UID %d\n", info->si_pid, info->si_uid);
    }
    else
    {
        fprintf(stderr, "Invalid SIGRT_TERM signal received from PID %d UID %d\n", info->si_pid, info->si_uid);
    }

    return;
}

static void sigterm_handler(int sig __attribute__ ((unused)))
{
    g_thread_events |= T_E_QUIT;

    return;
}

int signal_init(void)
{
    struct sigaction act;

    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGHUP);
    act.sa_handler = sigterm_handler;           /*SIGTERM SIGINT for main thread*/
    if (sigaction(SIGTERM, &act, NULL) == -1 ||
        sigaction(SIGINT, &act, NULL) == -1)
    {
        fprintf(stderr, "sigaction: %m\n");
        return -1;
    }

    act.sa_flags = SA_SIGINFO | SA_RESTART;
	sigemptyset(&act.sa_mask);
	act.sa_sigaction= sigrt_term_action;        /*SIGRT_TERM for other threads*/
	if (sigaction(SIGRT_TERM, &act, NULL) == -1) {
		fprintf(stderr, "sigaction: %m");
		return -1;
	}

    return 0;
}

void signal_exit(void)
{
    return;
}