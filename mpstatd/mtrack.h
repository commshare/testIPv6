
#ifndef _M_TRACK_H_
#define _M_TRACK_H_

#include "list.h"
#include "mconn_relay.h"
#include <uuid.h>

struct track_msg
{
    void *data;
    uuid_t uuid;
};

struct conn_track {
    struct conn_identity cid;
    DTQ_HEAD_S i_list;
	DTQ_HEAD_S o_list;
    uint32_t i_seq_n;
	uint32_t o_seq_n;
    int atype;
};


extern int track_thread_create(void);
extern int track_thread_close(void);

extern int conn_track_ht_init(void);
extern void conn_track_ht_exit(void);

#endif /* _M_TRACK_H_ */
