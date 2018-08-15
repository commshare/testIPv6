
#ifndef _M_THREAD_H_
#define _M_THREAD_H_

#include <semaphore.h>
#include <pthread.h>

#define THREAD_QUIT_WAIT_SEC 10

extern sem_t g_threads_sem;

extern int thread_init(void);
extern void thread_exit(void);

extern int thread_sem_timedwait(void);
extern int thread_sem_post(void);
extern int m_thread_kill(pthread_t tid);

#endif /* _M_THREAD_H_ */
