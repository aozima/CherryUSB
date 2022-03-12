#include <rtthread.h>
#include <string.h>

#include "usbh_core.h"
#include "usb_cdc.h"
#include <class/wireless/usbh_rndis.h>

#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void dump_hex(const void *ptr, rt_size_t buflen)
{
    unsigned char *buf = (unsigned char*)ptr;
    int i, j;

    for (i=0; i<buflen; i+=16)
    {
        rt_kprintf("%08X: ", i);

        for (j=0; j<16; j++)
            if (i+j < buflen)
                rt_kprintf("%02X ", buf[i+j]);
            else
                rt_kprintf("   ");
        rt_kprintf(" ");

        for (j=0; j<16; j++)
            if (i+j < buflen)
                rt_kprintf("%c", __is_print(buf[i+j]) ? buf[i+j] : '.');
        rt_kprintf("\n");
    }
}

static uint8_t dhcp_discover_data[350] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x60, 0x6E, 0x0C, 0x22, 0x38, 0x08, 0x00, 0x45, 0x00, 
	0x01, 0x50, 0x00, 0x01, 0x00, 0x00, 0xFF, 0x11, 0xBA, 0x9C, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 
	0xFF, 0xFF, 0x00, 0x44, 0x00, 0x43, 0x01, 0x3C, 0x35, 0xC8, 0x01, 0x01, 0x06, 0x00, 0x7D, 0xA0, 
	0xD8, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x6E, 0x0C, 0x22, 0x38, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x82, 0x53, 0x63, 0x35, 0x01, 0x01, 0x39, 0x02, 0x05, 
	0xDC, 0x37, 0x04, 0x01, 0x03, 0x1C, 0x06, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

ALIGN(8)
static uint8_t cdc_buffer[4096];
static void usbh_cdc_acm_callback(void *arg, int nbytes)
{
    //struct usbh_cdc_acm *rndis_class = (struct usbh_cdc_acm *)arg;

    USB_LOG_INFO("%s L%d, nbytes:%d\r\n", __FUNCTION__, __LINE__, nbytes);
    if (nbytes > 0) 
    {
        dump_hex(cdc_buffer, nbytes);
    }
}

static int rndis_init(struct usbh_rndis *class, struct usbh_hubport *hport, uint8_t intf)
{
    int ret = 0;
    struct usb_setup_packet *setup;

    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);

    setup = hport->setup;

    /* set interface */
    {
        struct rndis_init msg;
        msg.msg_type = RNDIS_MSG_INIT;
        msg.msg_len = sizeof(struct rndis_init);
        msg.request_id = class->request_id++;
        msg.major_version = 1;
        msg.minor_version = 0;
        msg.max_transfer_size = 0x4000;

        setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
        setup->bRequest = 0;
        setup->wValue = 0;
        setup->wIndex = intf;
        setup->wLength = sizeof(struct rndis_init);

        ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)&msg);
        USB_LOG_INFO("usbh_control_transfer RNDIS_MSG_INIT ret: %d\r\n", ret);
    }

    {
        struct rndis_init_c msg;

        setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
        setup->bRequest = CDC_REQUEST_GET_ENCAPSULATED_RESPONSE;
        setup->wValue = 0;
        setup->wIndex = intf;
        setup->wLength = sizeof(struct rndis_init_c);

        ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)&msg);
        USB_LOG_INFO("CDC_REQUEST_GET_ENCAPSULATED_RESPONSE RNDIS_MSG_INIT ret: %d\r\n", ret);

        dump_hex(&msg, sizeof(struct rndis_init_c));
    }

    return ret;
}

static int rndis_keepalive(struct usbh_rndis *class)
{
    int ret = 0;
    uint8_t intf = 0;
    struct usbh_hubport *hport = class->hport;
    struct usb_setup_packet *setup = hport->setup;
    struct rndis_keepalive msg = {0};
    struct rndis_keepalive_c resp;

    msg.msg_type = RNDIS_MSG_KEEPALIVE;
    msg.msg_len = sizeof(msg);
    msg.request_id = class->request_id++;

    setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = 0;
    setup->wValue = 0;
    setup->wIndex = intf;
    setup->wLength = sizeof(msg);

    ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)&msg);
    USB_LOG_INFO("usbh_control_transfer RNDIS_MSG_KEEPALIVE ret: %d\r\n", ret);

    setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = CDC_REQUEST_GET_ENCAPSULATED_RESPONSE;
    setup->wValue = 0;
    setup->wIndex = intf;
    setup->wLength = sizeof(resp);

    ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)&resp);
    USB_LOG_INFO("CDC_REQUEST_GET_ENCAPSULATED_RESPONSE RNDIS_MSG_KEEPALIVE ret: %d\r\n", ret);

    if(ret == 0)
    {
        dump_hex(&resp, sizeof(resp));
        USB_LOG_INFO("resp msg type: %08X len: %d id: %08X status: %08X\r\n", resp.msg_type, resp.msg_len, resp.request_id, resp.status);
    }

    return ret;
}

static int rndis_query_oid(struct usbh_rndis *class, uint32_t oid, int query_len, void *result, int len)
{
    int ret = 0;
    uint8_t intf = 0;
    struct usbh_hubport *hport = class->hport;
    struct usb_setup_packet *setup = hport->setup;
    struct rndis_query msg = {0};

    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);

    msg.msg_type = RNDIS_MSG_QUERY;
    msg.msg_len = query_len + sizeof(msg);
    msg.request_id = class->request_id++;
    msg.oid = oid;
    msg.len = query_len;
    msg.offset = 20;

    setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = 0;
    setup->wValue = 0;
    setup->wIndex = intf;
    setup->wLength = sizeof(msg);

    ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)&msg);
    USB_LOG_INFO("usbh_control_transfer send oid[%08X] ret: %d\r\n", oid, ret);

    setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = CDC_REQUEST_GET_ENCAPSULATED_RESPONSE;
    setup->wValue = 0;
    setup->wIndex = intf;
    setup->wLength = 256; // TODO: get real resp len.

    ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)result);
    USB_LOG_INFO("CDC_REQUEST_GET_ENCAPSULATED_RESPONSE OID ret: %d\r\n", ret);

    if(ret == 0)
    {
        const struct rndis_query_c *resp = (const struct rndis_query_c *)result;

        USB_LOG_INFO("resp msg type: %08X len: %d id: %08X\r\n", resp->msg_type, resp->msg_len, resp->request_id);
    }

    return ret;
}

int rndis_test(void)
{
    int ret, len;
    struct usbh_hubport *hport;
    uint8_t intf;

    struct usbh_rndis *rndis_class = (struct usbh_rndis *)usbh_find_class_instance("/dev/e1");
    if (rndis_class == NULL) {
        printf("do not find /dev/ttyACM0\r\n");
        return -1;
    }
    USB_LOG_INFO("usbh_rndis=%p\r\n", rndis_class);

    memset(cdc_buffer, 0, 512);
    hport = rndis_class->hport;
    intf = rndis_class->intf;
    USB_LOG_INFO("hport=%p, intf=%d.\r\n", hport, intf);

    ret = rndis_init(rndis_class, hport, 0);

#if 0
    int len = sizeof(cdc_buffer);
    ret = rndis_query_oid(rndis_class, RNDIS_OID_GEN_SUPPORTED_LIST, cdc_buffer, &len);
    USB_LOG_INFO("rndis_query_oid RNDIS_OID_GEN_SUPPORTED_LIST, ret=%d, len=%d.\r\n", ret, len);
    if(ret == 0)
    {
        dump_hex(cdc_buffer, len);
    }
#endif

    int query_len, result_len;
    len = 4096;//TODO

    query_len = 6;
    ret = rndis_query_oid(rndis_class, RNDIS_OID_802_3_PERMANENT_ADDRESS, query_len, cdc_buffer, len);
    USB_LOG_INFO("rndis_query_oid RNDIS_OID_802_3_PERMANENT_ADDRESS, ret=%d, len=%d.\r\n", ret, len);
    if(ret == 0)
    {
        dump_hex(cdc_buffer, query_len);
    }
    ret = rndis_query_oid(rndis_class, RNDIS_OID_802_3_CURRENT_ADDRESS, query_len, cdc_buffer, len);
    USB_LOG_INFO("rndis_query_oid RNDIS_OID_802_3_CURRENT_ADDRESS, ret=%d, len=%d.\r\n", ret, len);
    if(ret == 0)
    {
        memcpy(&dhcp_discover_data[6], cdc_buffer, query_len);
        memcpy(&dhcp_discover_data[0x46], cdc_buffer, query_len);
        dump_hex(cdc_buffer, query_len);
    }

    ret = rndis_keepalive(rndis_class);
    USB_LOG_INFO("rndis_keepalive ret=%d\r\n", ret);

#if 0 
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
    ret = usbh_ep_bulk_transfer(rndis_class->bulkin, cdc_buffer, 512);
    if (ret < 0) {
        printf("bulk in error\r\n");
        return ret;
    }

    printf("recv over:%d\r\n", ret);
    for (size_t i = 0; i < ret; i++) {
        printf("0x%02x ", cdc_buffer[i]);
    }
    printf("\r\n");
#endif

    // DHCP Discover
    struct rndis_data_hdr *data_hdr = (struct rndis_data_hdr *)cdc_buffer;

    data_hdr->msg_type = RNDIS_MSG_PACKET;
    data_hdr->msg_len = sizeof(struct rndis_data_hdr) + sizeof(dhcp_discover_data);
    data_hdr->data_offset = sizeof(struct rndis_data_hdr) - 8;
    data_hdr->data_len = sizeof(dhcp_discover_data);
    data_hdr->oob_data_offset = 0;
    data_hdr->oob_data_len = 0;
    data_hdr->num_oob = 0;
    data_hdr->packet_data_offset = 0;
    data_hdr->packet_data_len = 0;
    data_hdr->vc_handle = 0;
    data_hdr->reserved = 0;
    
    memcpy(&cdc_buffer[sizeof(struct rndis_data_hdr)], dhcp_discover_data, sizeof(dhcp_discover_data));

    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
    ret = usbh_ep_bulk_transfer(rndis_class->bulkout, cdc_buffer, data_hdr->msg_len);
    if (ret < 0) {
        printf("bulk out error\r\n");
        return ret;
    }
    USB_LOG_INFO("send over ret:%d\r\n", ret);
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);

#if 1
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
    usbh_ep_bulk_async_transfer(rndis_class->bulkin, cdc_buffer, 2048, usbh_cdc_acm_callback, rndis_class);
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
#endif

    return ret;
}
MSH_CMD_EXPORT(rndis_test, rndis_test.);
