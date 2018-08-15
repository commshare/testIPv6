
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "common.h"

#include <stdio.h>
#include <netinet/ip.h>



int packet_ip_l3_ignore(const uint8_t *l3data, size_t l3len, size_t *padlen)
{
    if (l3len < sizeof(struct ip))
    {
        fprintf(stderr, "IPV4: no valiad header found\n");
        return ERROR_FAILED;
    }

    const struct ip *h = (const struct ip*)l3data;
    if (h->ip_v != 4)
    {
        fprintf(stderr, "IPV4: version is mismatch\n");
        return ERROR_FAILED;
    }

    size_t tot = ntohs(h->ip_len);
    if (l3len != tot)
    {
        if (tot > l3len)
        {
            fprintf(stderr, "IPv4: invalid packet ignored, tot: %lu, l3len: %lu\n", tot, l3len);
            return ERROR_FAILED;
        }
        else
        {
            *padlen = l3len - tot;
        }
    }

    return ERROR_SUCCESS;
}

/** Vlan header struct, got from suricata v3.0 */
typedef struct VLANHdr_ {
    uint16_t vlan_cfi;
    uint16_t protocol;  /**< protocol field */
} __attribute__ ((__packed__)) VLANHdr;

int packet_vlan_ignore(const uint8_t **l3data, size_t *l3len, uint16_t *l3type)
{
    if (*l3len < sizeof(VLANHdr))
    {
        fprintf(stderr, "VLAN: no valiad header found\n");
        return ERROR_FAILED;
    }

    VLANHdr *vh = (VLANHdr *)(*l3data);
    *l3data = *l3data + sizeof(VLANHdr);
    *l3len = *l3len - sizeof(VLANHdr);
    *l3type = ntohs(vh->protocol);

    return ERROR_SUCCESS;
}

