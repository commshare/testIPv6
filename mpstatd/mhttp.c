
#include "common.h"
#include "mpcap.h"
#include "mtrack.h"
#include "mprotocol.h"

#include <string.h>
#include <http_parser.h>

static int http_dpkt_1st_cb_on_headers_complete(http_parser *p __attribute__((unused)))
{
	return HPE_CB_headers_complete;
}

static int http_dpkt_1st_version(const uint8_t *data, size_t len)
{
	http_parser parser;
#if !__clang__ && __GNUC__ <= 4
	http_parser_settings settings = {0};
#else
	http_parser_settings settings = {};
#endif
	settings.on_headers_complete = http_dpkt_1st_cb_on_headers_complete;

	http_parser_init(&parser, HTTP_REQUEST);

	http_parser_execute(&parser, &settings, (char *)data, len);

	switch (parser.http_errno) {
	case HPE_OK:
	case HPE_CB_headers_complete:
		if (parser.http_major > 1)
			return 2;
		else
			return 1;
		break;
	default:
		break;
	}

	return 0;
}

/*
 * Format: text: <method> <uri> HTTP/<maj.min>\r\n
 * Sendby: client
 * Maybe :
 */
void http_dpkt_1st_check(struct conn_track *track, const uint8_t *pdata)
{
	struct pcap_data_hdr *ph = (struct pcap_data_hdr *)pdata;
	const uint8_t *l5data = pdata + sizeof(struct pcap_data_hdr) + ph->l5offset;
	size_t l5len = ph->plen - ph->l5offset;

	///\refer http://www.iana.org/assignments/http-methods/http-methods.xhtml
	size_t off = 0;
	switch (l5data[0]) {
	case 'A': //0x41
		if (strncmp("ACL", (char *)l5data, 3) == 0) {
			off = 3;
			break;
		}
		break;
	case 'B': //0x42
		if (strncmp("BIND", (char *)l5data, 4) == 0) {
			off = 4;
			break;
		}
		if (strncmp("BASELINE-CONTROL", (char *)l5data, 16) == 0) {
			off = 16;
			break;
		}
		break;
	case 'C': //0x43
		if (strncmp("CONNECT", (char *)l5data, 7) == 0) {
			off = 7;
			break;
		}
		if (strncmp("COPY", (char *)l5data, 4) == 0) {
			off = 4;
			break;
		}
		if (strncmp("CHECKIN", (char *)l5data, 7) == 0) {
			off = 7;
			break;
		}
		if (strncmp("CHECKOUT", (char *)l5data, 8) == 0) {
			off = 8;
			break;
		}
		break;
	case 'D': //0x44
		if (strncmp("DELETE", (char *)l5data, 6) == 0) {
			off = 6;
			break;
		}
		break;
	case 'G': //0x47
		if (strncmp("GET", (char *)l5data, 3) == 0) {
			off = 3;
			break;
		}
		break;
	case 'H': //0x48
		if (strncmp("HEAD", (char *)l5data, 4) == 0) {
			off = 4;
			break;
		}
		break;
	case 'L': //0x4C
		if (strncmp("LOCK", (char *)l5data, 4) == 0) {
			off = 4;
			break;
		}
		if (strncmp("LINK", (char *)l5data, 4) == 0) {
			off = 4;
			break;
		}
		if (strncmp("LABEL", (char *)l5data, 5) == 0) {
			off = 5;
			break;
		}
		break;
	case 'M': //0x4D
		if (strncmp("MOVE", (char *)l5data, 4) == 0) {
			off = 4;
			break;
		}
		if (strncmp("MKCOL", (char *)l5data, 5) == 0) {
			off = 5;
			break;
		}
		if (strncmp("MERGE", (char *)l5data, 5) == 0) {
			off = 5;
			break;
		}
		if (strncmp("MKACTIVITY", (char *)l5data, 10) == 0) {
			off = 10;
			break;
		}
		if (strncmp("MKCALENDAR", (char *)l5data, 10) == 0) {
			off = 10;
			break;
		}
		if (strncmp("MKREDIRECTREF", (char *)l5data, 13) == 0) {
			off = 13;
			break;
		}
		if (strncmp("MKWORKSPACE", (char *)l5data, 11) == 0) {
			off = 11;
			break;
		}
		break;
	case 'O': //0x4F
		if (strncmp("OPTIONS", (char *)l5data, 7) == 0) {
			off = 7;
			break;
		}
		if (strncmp("ORDERPATCH", (char *)l5data, 10) == 0) {
			off = 10;
			break;
		}
		break;
	case 'P': //0x50
		if (strncmp("POST", (char *)l5data, 4) == 0) {
			off = 4;
			break;
		}
		if (strncmp("PUT", (char *)l5data, 3) == 0) {
			off = 3;
			break;
		}
		if (strncmp("PROPFIND", (char *)l5data, 8) == 0) {
			off = 8;
			break;
		}
		if (strncmp("PROPPATCH", (char *)l5data, 9) == 0) {
			off = 9;
			break;
		}
		if (strncmp("PRI", (char *)l5data, 3) == 0) {
			off = 3;
			break;
		}
		if (strncmp("PATCH", (char *)l5data, 5) == 0) {
			off = 5;
			break;
		}
		break;
	case 'R': //0x52
		if (strncmp("REPORT", (char *)l5data, 6) == 0) {
			off = 6;
			break;
		}
		if (strncmp("REBIND", (char *)l5data, 6) == 0) {
			off = 6;
			break;
		}
		break;
	case 'S': //0x53
		if (strncmp("SEARCH", (char *)l5data, 6) == 0) {
			off = 6;
			break;
		}
		break;
	case 'T': //0x54
		if (strncmp("TRACE", (char *)l5data, 5) == 0) {
			off = 5;
			break;
		}
		break;
	case 'U': //0x55
		if (strncmp("UNLOCK", (char *)l5data, 6) == 0) {
			off = 6;
			break;
		}
		if (strncmp("UNBIND", (char *)l5data, 6) == 0) {
			off = 6;
			break;
		}
		if (strncmp("UNCHECKOUT", (char *)l5data, 10) == 0) {
			off = 10;
			break;
		}
		if (strncmp("UNLINK", (char *)l5data, 6) == 0) {
			off = 6;
			break;
		}
		if (strncmp("UPDATE", (char *)l5data, 6) == 0) {
			off = 6;
			break;
		}
		if (l5len > 18 && strncmp("UNDATEREDIRECTREF", (char *)l5data, 18) == 0) {
			off = 18;
			break;
		}
		break;
	case 'V': //0x56
		if (strncmp("VERSION-CONTROL", (char *)l5data, 15) == 0) {
			off = 15;
			break;
		}
		break;
	default:
		break;
	}

	if (off == 0 || l5data[off] != ' ')
		goto not_http;
	off++;
	track->atype_ex[PSD_PROTO_SMTP] = 1;
	track->atype_ex[PSD_PROTO_FTPC] = 1;
	track->atype_ex[PSD_PROTO_NNTP] = 1;
	track->atype_ex[PSD_PROTO_NBT ] = 1;
	track->atype_ex[PSD_PROTO_MSDS] = 1;
	track->atype_ex[PSD_PROTO_SSL ] = 1;
	track->atype_ex[PSD_PROTO_POP3] = 1;
	track->atype_ex[PSD_PROTO_IMAP] = 1;
	track->atype_ex[PSD_PROTO_IRC ] = 1;
	track->atype_ex[PSD_PROTO_SSH ] = 1;
	track->atype_ex[PSD_PROTO_LDAP] = 1;
	track->atype_ex[PSD_PROTO_SOCKS4] = 1;
	track->atype_ex[PSD_PROTO_SOCKS5] = 1;
	track->atype_ex[PSD_PROTO_VNC ] = 1;
	track->atype_ex[PSD_PROTO_BT  ] = 1;
	track->atype_ex[PSD_PROTO_AFP ] = 1;
	track->atype_ex[PSD_PROTO_SNMP] = 1;
	track->atype_ex[PSD_PROTO_XMPP] = 1;
	track->atype_ex[PSD_PROTO_SPICE ] = 1;

	switch (http_dpkt_1st_version(l5data, l5len)) {
	case 1:
		track->atype = PSD_PROTO_HTTP;
		return;
		break;
	case 2:
		track->atype_ex[PSD_PROTO_HTTP] = 1;
		track->atype = PSD_PROTO_HTTP2;
		return;
		break;
	case 0:
	default:
		break;
	}

	return;
}


