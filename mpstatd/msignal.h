
#ifndef _M_SIGNAL_H_
#define _M_SIGNAL_H_

#include <signal.h>

#define SIGRT_CTLME (SIGRTMIN + 0)  //control end, rsp to main
#define SIGRT_TERM  (SIGRTMIN + 1)

typedef enum thread_event_mask
{
    T_E_NONE   = 0x0000,
    T_E_QUIT   = 0x0001,
    T_E_RELOAD = 0x0002,
    T_E_CLEAN  = 0x0004
}THREAD_EVENT_E;


extern _Thread_local THREAD_EVENT_E g_thread_events;

extern int signal_block_thread_all(void);
extern int signal_unblock_thread_main(void);
extern int signal_unblock_thread_other(void);
extern int signal_init(void);
extern void signal_exit(void);


#endif /* _M_SIGNAL_H_ */
