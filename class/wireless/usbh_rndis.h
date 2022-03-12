#ifndef _USB_CLASHH_RNDIS_H__
#define _USB_CLASHH_RNDIS_H__

#include "usbh_core.h"

extern const struct usbh_class_driver msc_class_driver;

struct usbh_rndis {
    usbh_epinfo_t bulkin;  /* Bulk IN endpoint */
    usbh_epinfo_t bulkout; /* Bulk OUT endpoint */
    usbh_epinfo_t int_notify; /* Notify endpoint */

    uint32_t	request_id;

    struct usbh_hubport *hport;
    uint8_t intf;
};

/*
 * Codes for "msg_type" field of rndis messages;
 * only the data channel uses packet messages (maybe batched);
 * everything else goes on the control channel.
 */
#define RNDIS_MSG_COMPLETION	0x80000000
#define RNDIS_MSG_PACKET	0x00000001	/* 1-N packets */
#define RNDIS_MSG_INIT		0x00000002
#define RNDIS_MSG_INIT_C	(RNDIS_MSG_INIT|RNDIS_MSG_COMPLETION)
#define RNDIS_MSG_HALT		0x00000003
#define RNDIS_MSG_QUERY		0x00000004
#define RNDIS_MSG_QUERY_C	(RNDIS_MSG_QUERY|RNDIS_MSG_COMPLETION)
#define RNDIS_MSG_SET		0x00000005
#define RNDIS_MSG_SET_C		(RNDIS_MSG_SET|RNDIS_MSG_COMPLETION)
#define RNDIS_MSG_RESET		0x00000006
#define RNDIS_MSG_RESET_C	(RNDIS_MSG_RESET|RNDIS_MSG_COMPLETION)
#define RNDIS_MSG_INDICATE	0x00000007
#define RNDIS_MSG_KEEPALIVE	0x00000008
#define RNDIS_MSG_KEEPALIVE_C	(RNDIS_MSG_KEEPALIVE|RNDIS_MSG_COMPLETION)

typedef uint32_t __le32;

/*
 * CONTROL uses CDC "encapsulated commands" with funky notifications.
 *  - control-out:  SEND_ENCAPSULATED
 *  - interrupt-in:  RESPONSE_AVAILABLE
 *  - control-in:  GET_ENCAPSULATED
 *
 * We'll try to ignore the RESPONSE_AVAILABLE notifications.
 *
 * REVISIT some RNDIS implementations seem to have curious issues still
 * to be resolved.
 */
struct rndis_msg_hdr {
	__le32	msg_type;			/* RNDIS_MSG_* */
	__le32	msg_len;
	/* followed by data that varies between messages */
	__le32	request_id;
	__le32	status;
	/* ... and more */
} __attribute__ ((packed));

struct rndis_data_hdr {
	__le32	msg_type;		/* RNDIS_MSG_PACKET */
	__le32	msg_len;		/* rndis_data_hdr + data_len + pad */
	__le32	data_offset;		/* 36 -- right after header */
	__le32	data_len;		/* ... real packet size */

	__le32	oob_data_offset;	/* zero */
	__le32	oob_data_len;		/* zero */
	__le32	num_oob;		/* zero */
	__le32	packet_data_offset;	/* zero */

	__le32	packet_data_len;	/* zero */
	__le32	vc_handle;		/* zero */
	__le32	reserved;		/* zero */
} __attribute__ ((packed));

struct rndis_init {		/* OUT */
	/* header and: */
	__le32	msg_type;			/* RNDIS_MSG_INIT */
	__le32	msg_len;			/* 24 */
	__le32	request_id;
	__le32	major_version;			/* of rndis (1.0) */
	__le32	minor_version;
	__le32	max_transfer_size;
} __attribute__ ((packed));

struct rndis_init_c {		/* IN */
	/* header and: */
	__le32	msg_type;			/* RNDIS_MSG_INIT_C */
	__le32	msg_len;
	__le32	request_id;
	__le32	status;
	__le32	major_version;			/* of rndis (1.0) */
	__le32	minor_version;
	__le32	device_flags;
	__le32	medium;				/* zero == 802.3 */
	__le32	max_packets_per_message;
	__le32	max_transfer_size;
	__le32	packet_alignment;		/* max 7; (1<<n) bytes */
	__le32	af_list_offset;			/* zero */
	__le32	af_list_size;			/* zero */
} __attribute__ ((packed));

struct rndis_query {		/* OUT */
	/* header and: */
	__le32	msg_type;			/* RNDIS_MSG_QUERY */
	__le32	msg_len;
	__le32	request_id;
	__le32	oid;
	__le32	len;
	__le32	offset;
/*?*/	__le32	handle;				/* zero */
} __attribute__ ((packed));

struct rndis_query_c {		/* IN */
	/* header and: */
	__le32	msg_type;			/* RNDIS_MSG_QUERY_C */
	__le32	msg_len;
	__le32	request_id;
	__le32	status;
	__le32	len;
	__le32	offset;
} __attribute__ ((packed));

struct rndis_set {		/* OUT */
	/* header and: */
	__le32	msg_type;			/* RNDIS_MSG_SET */
	__le32	msg_len;
	__le32	request_id;
	__le32	oid;
	__le32	len;
	__le32	offset;
/*?*/	__le32	handle;				/* zero */
} __attribute__ ((packed));

struct rndis_set_c {		/* IN */
	/* header and: */
	__le32	msg_type;			/* RNDIS_MSG_SET_C */
	__le32	msg_len;
	__le32	request_id;
	__le32	status;
} __attribute__ ((packed));

struct rndis_reset {		/* IN */
	/* header and: */
	__le32	msg_type;			/* RNDIS_MSG_RESET */
	__le32	msg_len;
	__le32	reserved;
} __attribute__ ((packed));

struct rndis_reset_c {		/* OUT */
	/* header and: */
	__le32	msg_type;			/* RNDIS_MSG_RESET_C */
	__le32	msg_len;
	__le32	status;
	__le32	addressing_lost;
} __attribute__ ((packed));

struct rndis_indicate {		/* IN (unrequested) */
	/* header and: */
	__le32	msg_type;			/* RNDIS_MSG_INDICATE */
	__le32	msg_len;
	__le32	status;
	__le32	length;
	__le32	offset;
/**/	__le32	diag_status;
	__le32	error_offset;
/**/	__le32	message;
} __attribute__ ((packed));

struct rndis_keepalive {	/* OUT (optionally IN) */
	/* header and: */
	__le32	msg_type;			/* RNDIS_MSG_KEEPALIVE */
	__le32	msg_len;
	__le32	request_id;
} __attribute__ ((packed));

struct rndis_keepalive_c {	/* IN (optionally OUT) */
	/* header and: */
	__le32	msg_type;			/* RNDIS_MSG_KEEPALIVE_C */
	__le32	msg_len;
	__le32	request_id;
	__le32	status;
} __attribute__ ((packed));

#endif /* _USB_CLASHH_RNDIS_H__ */
