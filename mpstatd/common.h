#ifndef _M_COMMON_H_
#define _M_COMMON_H_

#define ERROR_SUCCESS 0
#define ERROR_FAILED -1

#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>


/* Direction macros */
#define DP_UNKOWN 0
#define DP_IN     1
#define DP_OUT    2
#define DP_OPP    3

#define STRLCPY(destStr, srcStr, size) {\
size_t ret = strlen(srcStr); \
    if (size)\
    {\
        size_t len = (ret >= size)?size-1:ret;\
        memcpy(destStr, srcStr, len);\
        destStr[len] = 0;\
    }\
}

extern pid_t g_process_pid;

#endif /* _M_COMMON_H_ */
