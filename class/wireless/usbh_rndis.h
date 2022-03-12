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

struct rndis_init {		/* OUT */
	/* header and: */
	uint32_t	msg_type;			/* RNDIS_MSG_INIT */
	uint32_t	msg_len;			/* 24 */
	uint32_t	request_id;
	uint32_t	major_version;			/* of rndis (1.0) */
	uint32_t	minor_version;
	uint32_t	max_transfer_size;
} __attribute__ ((packed));

struct rndis_query {		/* OUT */
	/* header and: */
	uint32_t	msg_type;			/* RNDIS_MSG_QUERY */
	uint32_t	msg_len;
	uint32_t	request_id;
	uint32_t	oid;
	uint32_t	len;
	uint32_t	offset;
/*?*/	uint32_t	handle;				/* zero */
} __attribute__ ((packed));

struct rndis_keepalive {	/* OUT (optionally IN) */
	/* header and: */
	uint32_t	msg_type;			/* RNDIS_MSG_KEEPALIVE */
	uint32_t	msg_len;
	uint32_t	request_id;
} __attribute__ ((packed));


#endif /* _USB_CLASHH_RNDIS_H__ */
