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

/* Object Identifiers used by NdisRequest Query/Set Information */
/* General (Required) Objects */
#define RNDIS_OID_GEN_SUPPORTED_LIST		0x00010101
#define RNDIS_OID_GEN_HARDWARE_STATUS		0x00010102
#define RNDIS_OID_GEN_MEDIA_SUPPORTED		0x00010103
#define RNDIS_OID_GEN_MEDIA_IN_USE		0x00010104
#define RNDIS_OID_GEN_MAXIMUM_LOOKAHEAD		0x00010105
#define RNDIS_OID_GEN_MAXIMUM_FRAME_SIZE	0x00010106
#define RNDIS_OID_GEN_LINK_SPEED		0x00010107
#define RNDIS_OID_GEN_TRANSMIT_BUFFER_SPACE	0x00010108
#define RNDIS_OID_GEN_RECEIVE_BUFFER_SPACE	0x00010109
#define RNDIS_OID_GEN_TRANSMIT_BLOCK_SIZE	0x0001010A
#define RNDIS_OID_GEN_RECEIVE_BLOCK_SIZE	0x0001010B
#define RNDIS_OID_GEN_VENDOR_ID			0x0001010C
#define RNDIS_OID_GEN_VENDOR_DESCRIPTION	0x0001010D
#define RNDIS_OID_GEN_CURRENT_PACKET_FILTER	0x0001010E
#define RNDIS_OID_GEN_CURRENT_LOOKAHEAD		0x0001010F
#define RNDIS_OID_GEN_DRIVER_VERSION		0x00010110
#define RNDIS_OID_GEN_MAXIMUM_TOTAL_SIZE	0x00010111
#define RNDIS_OID_GEN_PROTOCOL_OPTIONS		0x00010112
#define RNDIS_OID_GEN_MAC_OPTIONS		0x00010113
#define RNDIS_OID_GEN_MEDIA_CONNECT_STATUS	0x00010114
#define RNDIS_OID_GEN_MAXIMUM_SEND_PACKETS	0x00010115
#define RNDIS_OID_GEN_VENDOR_DRIVER_VERSION	0x00010116
#define RNDIS_OID_GEN_SUPPORTED_GUIDS		0x00010117
#define RNDIS_OID_GEN_NETWORK_LAYER_ADDRESSES	0x00010118
#define RNDIS_OID_GEN_TRANSPORT_HEADER_OFFSET	0x00010119
#define RNDIS_OID_GEN_PHYSICAL_MEDIUM		0x00010202
#define RNDIS_OID_GEN_MACHINE_NAME		0x0001021A
#define RNDIS_OID_GEN_RNDIS_CONFIG_PARAMETER	0x0001021B
#define RNDIS_OID_GEN_VLAN_ID			0x0001021C

/* 802.3 Objects (Ethernet) */
#define RNDIS_OID_802_3_PERMANENT_ADDRESS	0x01010101
#define RNDIS_OID_802_3_CURRENT_ADDRESS		0x01010102
#define RNDIS_OID_802_3_MULTICAST_LIST		0x01010103
#define RNDIS_OID_802_3_MAXIMUM_LIST_SIZE	0x01010104
#define RNDIS_OID_802_3_MAC_OPTIONS		0x01010105

#define RNDIS_802_3_MAC_OPTION_PRIORITY		0x00000001

#define RNDIS_OID_802_3_RCV_ERROR_ALIGNMENT	0x01020101
#define RNDIS_OID_802_3_XMIT_ONE_COLLISION	0x01020102
#define RNDIS_OID_802_3_XMIT_MORE_COLLISIONS	0x01020103

#define RNDIS_OID_802_3_XMIT_DEFERRED		0x01020201
#define RNDIS_OID_802_3_XMIT_MAX_COLLISIONS	0x01020202
#define RNDIS_OID_802_3_RCV_OVERRUN		0x01020203
#define RNDIS_OID_802_3_XMIT_UNDERRUN		0x01020204
#define RNDIS_OID_802_3_XMIT_HEARTBEAT_FAILURE	0x01020205
#define RNDIS_OID_802_3_XMIT_TIMES_CRS_LOST	0x01020206
#define RNDIS_OID_802_3_XMIT_LATE_COLLISIONS	0x01020207

#define RNDIS_OID_802_11_BSSID				0x0d010101
#define RNDIS_OID_802_11_SSID				0x0d010102
#define RNDIS_OID_802_11_INFRASTRUCTURE_MODE		0x0d010108
#define RNDIS_OID_802_11_ADD_WEP			0x0d010113
#define RNDIS_OID_802_11_REMOVE_WEP			0x0d010114
#define RNDIS_OID_802_11_DISASSOCIATE			0x0d010115
#define RNDIS_OID_802_11_AUTHENTICATION_MODE		0x0d010118
#define RNDIS_OID_802_11_PRIVACY_FILTER			0x0d010119
#define RNDIS_OID_802_11_BSSID_LIST_SCAN		0x0d01011a
#define RNDIS_OID_802_11_ENCRYPTION_STATUS		0x0d01011b
#define RNDIS_OID_802_11_ADD_KEY			0x0d01011d
#define RNDIS_OID_802_11_REMOVE_KEY			0x0d01011e
#define RNDIS_OID_802_11_ASSOCIATION_INFORMATION	0x0d01011f
#define RNDIS_OID_802_11_CAPABILITY			0x0d010122
#define RNDIS_OID_802_11_PMKID				0x0d010123
#define RNDIS_OID_802_11_NETWORK_TYPES_SUPPORTED	0x0d010203
#define RNDIS_OID_802_11_NETWORK_TYPE_IN_USE		0x0d010204
#define RNDIS_OID_802_11_TX_POWER_LEVEL			0x0d010205
#define RNDIS_OID_802_11_RSSI				0x0d010206
#define RNDIS_OID_802_11_RSSI_TRIGGER			0x0d010207
#define RNDIS_OID_802_11_FRAGMENTATION_THRESHOLD	0x0d010209
#define RNDIS_OID_802_11_RTS_THRESHOLD			0x0d01020a
#define RNDIS_OID_802_11_SUPPORTED_RATES		0x0d01020e
#define RNDIS_OID_802_11_CONFIGURATION			0x0d010211
#define RNDIS_OID_802_11_POWER_MODE			0x0d010216
#define RNDIS_OID_802_11_BSSID_LIST			0x0d010217

#endif /* _USB_CLASHH_RNDIS_H__ */