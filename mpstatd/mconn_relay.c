
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "common.h"
#include "mconn_relay.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <glib.h>

static GHashTable *g_conn_relay_ht = NULL;
static pthread_rwlock_t conn_relay_ht_rwlock = {};

static guint conn_relay_ht_hash(gconstpointer key)
{
    uint32_t *k = (uint32_t *)key;

    return k[0];
}

static gboolean conn_relay_ht_equal(gconstpointer a, gconstpointer b)
{
    uuid_t *ka = (uuid_t *)a;
    uuid_t *kb = (uuid_t *)b;

    if (0 == uuid_compare(*ka, *kb))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int conn_relay_ht_init(void)
{
    g_conn_relay_ht = g_hash_table_new_full(conn_relay_ht_hash, conn_relay_ht_equal, free, free);
    if (NULL == g_conn_relay_ht)
    {
        fprintf(stderr, "g_hash_table_new_full failed : %m\n");
        return ERROR_FAILED;
    }

    int iRet = pthread_rwlock_init(&conn_relay_ht_rwlock, NULL);
    if (0 != iRet)
    {
        fprintf(stderr, "pthread_rwlock_init failed : %m\n");
        g_hash_table_destroy(g_conn_relay_ht);
        g_conn_relay_ht = NULL;
        return ERROR_FAILED;
    }

    return ERROR_SUCCESS;
}

void conn_relay_ht_exit(void)
{
    if (NULL != g_conn_relay_ht)
    {
        g_hash_table_destroy(g_conn_relay_ht);
        pthread_rwlock_destroy(&conn_relay_ht_rwlock);
    }

    return;
}

int conn_relay_ht_get_record(const uuid_t *pstUuid, struct conn_relay *pstRelay)
{
    int iRet;
    int exit_code = -1;

    iRet = pthread_rwlock_rdlock(&conn_relay_ht_rwlock);
    if (0 != iRet)
    {
        fprintf(stderr, "pthread_rwlock_rdlock failed : %m\n");
        return exit_code;
    }

    struct conn_relay *r = g_hash_table_lookup(g_conn_relay_ht, pstUuid);
    if (NULL != r)
    {
        memcpy(&pstRelay->stCid, &r->stCid, sizeof(struct conn_identity));
        exit_code = 1;
    }
    else
    {
        exit_code = 0;
    }

    iRet = pthread_rwlock_unlock(&conn_relay_ht_rwlock);
    if (0 != iRet)
    {
        fprintf(stderr, "pthread_rwlock_unlock failed : %m\n");
    }

    return exit_code;
}

int conn_relay_ht_add_record(const uuid_t *pstUuid, const struct conn_identity *pstIdentity)
{
    int iRet;
    int exit_code = -1;

    uuid_t *k = malloc(sizeof(uuid_t));
    if (NULL == k)
    {
        fprintf(stderr, "malloc failed : %m\n");
        return exit_code;
    }

    struct conn_relay *v = calloc(1, sizeof(struct conn_relay));
    if (NULL == v)
    {
        fprintf(stderr, "calloc failed : %m\n");
        free(k);
        return exit_code;
    }

    uuid_copy(*k, *pstUuid);
    memcpy(&v->stCid, pstIdentity, sizeof(struct conn_identity));

    iRet = pthread_rwlock_wrlock(&conn_relay_ht_rwlock);
    if (0 != iRet)
    {
        fprintf(stderr, "pthread_rwlock_rdlock failed : %m\n");
        free(k);
        free(v);
        return exit_code;
    }

    g_hash_table_replace(g_conn_relay_ht, k, v);

    iRet = pthread_rwlock_unlock(&conn_relay_ht_rwlock);
    if (0 != iRet)
    {
        fprintf(stderr, "pthread_rwlock_unlock failed : %m\n");
    }

    return 0;
}

int conn_relay_ht_del_record(const uuid_t *uuid)
{
    int iRet;

    iRet = pthread_rwlock_wrlock(&conn_relay_ht_rwlock);
    if (0 != iRet)
    {
        fprintf(stderr, "pthread_rwlock_wrlock failed : %m\n");
        return -1;
    }

    g_hash_table_remove(g_conn_relay_ht, uuid);
    g_conn_relay_ht = NULL;

    iRet = pthread_rwlock_unlock(&conn_relay_ht_rwlock);
    if (0 != iRet)
    {
        fprintf(stderr, "pthread_rwlock_unlock failed : %m\n");
    }

    return 0;
}

