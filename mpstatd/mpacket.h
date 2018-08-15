#ifndef _M_PCAKET_H_
#define _M_PCAKET_H_


extern int packet_ip_l3_ignore(const uint8_t *l3data, size_t l3len, size_t *padlen);
extern int packet_vlan_ignore(const uint8_t **l3data, size_t *l3len, uint16_t *l3type);

#endif /* _M_PCAKET_H_ */
