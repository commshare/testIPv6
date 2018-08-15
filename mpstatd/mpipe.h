
#ifndef _M_PIPE_H_
#define _M_PIPE_H_

extern int g_conn_pipe_fd[2];
extern int g_track_pipe_fd[2];

extern int pipe_init(void);
extern void pipe_exit(void);


#endif /* _M_PIPE_H_ */

