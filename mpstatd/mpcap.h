
#ifndef _M_PCAP_H_
#define _M_PCAP_H_

struct pcap_data_hdr {
    uint32_t pcap_index;
    uint32_t plen;      ///\note Packet Length, exclude this header
    uint8_t  direction;
    uint8_t  l3family;
    uint16_t l4offset;
    uint8_t  l4proto;
    uint8_t  l4close;
    uint16_t l5offset;
    uint16_t t3offset;
    uint16_t t3proto;
    uint8_t  tunnel;
    uint8_t  t3family;
    uint8_t  r1;
    uint8_t  pcap_type;
    int64_t  ts;
} __attribute_packed__;
_Static_assert((sizeof(struct pcap_data_hdr) % 8) == 0, "Alignment required on structure pcap_data_hdr");

extern int pcap_thread_create(void);
extern int pcap_thread_close(void);


#endif /* _M_PCAP_H_ */
