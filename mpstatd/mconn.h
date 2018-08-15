
#ifndef _M_CONN_H_
#define _M_CONN_H_

#include <uuid.h>

struct conn_index {
	uuid_t uuid;  ///\note time based uuid
	int ignore;
	unsigned int close;
	int64_t init_ts;
	int64_t last_ts;
	uint32_t in_fin_seq;
	uint32_t ou_fin_seq;

	uint64_t in_bytes;
	uint64_t ou_bytes;

	int unref_trackid;
};


extern int conn_thread_create(void);
extern int conn_thread_close(void);
extern int conn_index_ht_init(void);
extern void conn_index_ht_exit(void);

#endif /* _M_CONN_H_ */
