#include <rtthread.h>
#include <string.h>

#include "usbh_core.h"
#include "usb_cdc.h"
#include <class/wireless/usbh_rndis.h>

#include <netif/ethernetif.h>
#include <netdev.h>

#define DM9051_RX_DUMP
#define DM9051_TX_DUMP
// #define DM9051_DUMP_RAW

#define MAX_ADDR_LEN         6

struct rt_rndis_eth
{
    /* inherit from ethernet device */
    struct eth_device parent;

    struct usbh_rndis *class;
    struct usbh_hubport *hport;
    uint8_t intf;

    rt_uint8_t   dev_addr[MAX_ADDR_LEN];
};
typedef struct rt_rndis_eth * rt_rndis_eth_t;
static struct rt_rndis_eth usbh_rndis_eth_device;

#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void dump_hex(const void *ptr, rt_size_t buflen)
{
    unsigned char *buf = (unsigned char*)ptr;
    int i, j;

    for (i=0; i<buflen; i+=16)
    {
        rt_kprintf("%08X:", i);

        for (j=0; j<16; j++)
            if (i+j < buflen)
            {
                if ((j % 8) == 0) {
                    rt_kprintf("  ");
                }

                rt_kprintf("%02X ", buf[i+j]);
            }
            else
                rt_kprintf("   ");
        rt_kprintf(" ");

        for (j=0; j<16; j++)
            if (i+j < buflen)
                rt_kprintf("%c", __is_print(buf[i+j]) ? buf[i+j] : '.');
        rt_kprintf("\n");
    }
}

#if defined(DM9051_RX_DUMP) ||  defined(DM9051_TX_DUMP)
static void packet_dump(const char * msg, const struct pbuf* p)
{
    rt_uint8_t header[6 + 6 + 2];
    rt_uint16_t type;

    pbuf_copy_partial(p, header, sizeof(header), 0);
    type = (header[12] << 8) | header[13];

    rt_kprintf("%02X:%02X:%02X:%02X:%02X:%02X <== %02X:%02X:%02X:%02X:%02X:%02X ",
               header[0], header[1], header[2], header[3], header[4], header[5],
               header[6], header[7], header[8], header[9], header[10], header[11]);

    switch (type)
    {
    case 0x0800:
        rt_kprintf("IPv4. ");
        break;

    case 0x0806:
        rt_kprintf("ARP.  ");
        break;

    case 0x86DD:
        rt_kprintf("IPv6. ");
        break;

    default:
        rt_kprintf("%04X. ", type);
        break;
    }

    rt_kprintf("%s %d byte. \n", msg, p->tot_len);
#ifdef DM9051_DUMP_RAW    
    const struct pbuf* q;
    rt_uint32_t i,j;
    rt_uint8_t *ptr;

    rt_kprintf("%s %d byte\n", msg, p->tot_len);

    i=0;
    for(q=p; q != RT_NULL; q= q->next)
    {
        ptr = q->payload;

        for(j=0; j<q->len; j++)
        {
            if( (i%8) == 0 )
            {
                rt_kprintf("  ");
            }
            if( (i%16) == 0 )
            {
                rt_kprintf("\r\n");
            }
            rt_kprintf("%02X ", *ptr);

            i++;
            ptr++;
        }
    }

    rt_kprintf("\n\n");
#endif /* DM9051_DUMP_RAW */
}
#else
#define packet_dump(...)
#endif /* dump */

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
static uint8_t cdc_buffer2[4096];
static void usbh_cdc_acm_callback(void *arg, int nbytes)
{
    //struct usbh_cdc_acm *rndis_class = (struct usbh_cdc_acm *)arg;

    USB_LOG_INFO("%s L%d, nbytes:%d\r\n", __FUNCTION__, __LINE__, nbytes);
    if (nbytes > 0) 
    {
        // dump_hex(cdc_buffer, nbytes);
    }
    else
    {
        return;
    }

    struct rndis_data_hdr *data_hdr = (struct rndis_data_hdr *)cdc_buffer2;
    // struct rndis_msg_hdr *msg_hdr = (struct rndis_msg_hdr *)cdc_buffer;

    USB_LOG_INFO("%s L%d, type: %08X, len:%d, data_len:%d\r\n", __FUNCTION__, __LINE__, data_hdr->msg_type, data_hdr->msg_len, data_hdr->data_len);
    // USB_LOG_INFO("%s L%d, type: %08X, len:%d, request_id:%d\r\n", __FUNCTION__, __LINE__, msg_hdr->msg_type, msg_hdr->msg_len, msg_hdr->request_id);

    if(data_hdr->msg_type == RNDIS_MSG_PACKET)
    {
        struct pbuf *p = RT_NULL;

        /* allocate buffer           */
        p = pbuf_alloc(PBUF_LINK, data_hdr->msg_len, PBUF_RAM);
        if (p != NULL)
        {
            const uint8_t *tmp_buf = (const uint8_t *)cdc_buffer2;

            // memcpy((void *)p->payload, tmp_buf + sizeof(struct rndis_data_hdr), data_hdr->msg_len);
            pbuf_take(p, tmp_buf + sizeof(struct rndis_data_hdr), data_hdr->msg_len);

#ifdef DM9051_RX_DUMP
            // if (p)
            // packet_dump(__FUNCTION__, p);
            dump_hex(p->payload, nbytes);
#endif /* DM9051_RX_DUMP */

            struct eth_device *eth_dev = &usbh_rndis_eth_device.parent;
            if ((eth_dev->netif->input(p, eth_dev->netif)) != ERR_OK)
            {
                USB_LOG_INFO("F:%s L:%d IP input error", __FUNCTION__, __LINE__);
                pbuf_free(p);
                p = RT_NULL;
            }
            USB_LOG_INFO("%s L%d input OK\r\n", __FUNCTION__, __LINE__);
        }
        else
        {
            USB_LOG_ERR("%s L%d pbuf_alloc NULL\r\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        USB_LOG_WRN("%s L%d msg_type != RNDIS_MSG_PACKET\r\n", __FUNCTION__, __LINE__);
        dump_hex(data_hdr, nbytes);
    }
}

static int rndis_init(struct usbh_rndis *class)
{
    int ret = 0;
    uint8_t intf = 0;
    struct usbh_hubport *hport = class->hport;
    struct usb_setup_packet *setup = hport->setup;

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
        struct rndis_init_c *resp;
        // struct rndis_init_c msg;
        resp = (struct rndis_init_c *)rt_malloc(sizeof(struct rndis_init_c) + 512);

        setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
        setup->bRequest = CDC_REQUEST_GET_ENCAPSULATED_RESPONSE;
        setup->wValue = 0;
        setup->wIndex = intf;
        setup->wLength = 4096;

        ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)resp);
        USB_LOG_INFO("CDC_REQUEST_GET_ENCAPSULATED_RESPONSE RNDIS_MSG_INIT ret: %d\r\n", ret);

        if (ret == 0) {
            dump_hex(resp, sizeof(struct rndis_init_c) + 64);
            USB_LOG_INFO("resp msg type: %08X len: %d id: %08X status: %08X\r\n", resp->msg_type, resp->msg_len, resp->request_id, resp->status);
        }
        // dump_hex(&msg, sizeof(struct rndis_init_c));
        rt_free(resp);
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
    struct rndis_keepalive_c *resp;

    msg.msg_type = RNDIS_MSG_KEEPALIVE;
    msg.msg_len = sizeof(msg);
    msg.request_id = class->request_id++;

    setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = 0;
    setup->wValue = 0;
    setup->wIndex = intf;
    setup->wLength = sizeof(msg);

    ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)&msg);
    USB_LOG_INFO("usbh_control_transfer RNDIS_MSG_KEEPALIVE ret: %d, id:%08X\r\n", ret, msg.request_id);

    setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = CDC_REQUEST_GET_ENCAPSULATED_RESPONSE;
    setup->wValue = 0;
    setup->wIndex = intf;
    setup->wLength = 4096;

    resp = (struct rndis_keepalive_c *)rt_malloc(4096);

    ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)resp);
    USB_LOG_INFO("CDC_REQUEST_GET_ENCAPSULATED_RESPONSE RNDIS_MSG_KEEPALIVE ret: %d\r\n", ret);

    if(ret == 0)
    {
        dump_hex(resp, resp->msg_len + 32);
        USB_LOG_INFO("resp msg type: %08X len: %d id: %08X status: %08X\r\n\r\n", resp->msg_type, resp->msg_len, resp->request_id, resp->status);

        if(resp->msg_type == RNDIS_MSG_INDICATE)
        {
            USB_LOG_WRN("%s L%d RNDIS_MSG_INDICATE\r\n", __FUNCTION__, __LINE__);
            ret = 123456789; // magic code.
        }

        if(resp->msg_type != RNDIS_MSG_KEEPALIVE_C)
        {
            USB_LOG_ERR("resp msg type: %08X len: %d id: %08X status: %08X\r\n\r\n", resp->msg_type, resp->msg_len, resp->request_id, resp->status);

            setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
            setup->bRequest = CDC_REQUEST_GET_ENCAPSULATED_RESPONSE;
            setup->wValue = 0;
            setup->wIndex = intf;
            setup->wLength = 4096;

            int ret2 = usbh_control_transfer(hport->ep0, setup, (uint8_t *)resp);
            USB_LOG_INFO("CDC_REQUEST_GET_ENCAPSULATED_RESPONSE RNDIS_MSG_KEEPALIVE ret2: %d\r\n", ret2);
            USB_LOG_ERR("resp msg2 type: %08X len: %d id: %08X status: %08X\r\n\r\n", resp->msg_type, resp->msg_len, resp->request_id, resp->status);
        }

    }
    rt_free(resp);

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

static int rndis_msg_set(struct usbh_rndis *class, uint32_t oid,
                         void *set_buf, uint32_t set_len)
{
    int ret = 0;
    uint8_t intf = 0;
    struct usbh_hubport *hport = class->hport;
    struct usb_setup_packet *setup = hport->setup;
    struct rndis_set *msg = {0};
    struct rndis_set_c *resp;

    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);

    msg = (struct rndis_set *)rt_malloc(sizeof(struct rndis_set) + set_len);
    memcpy((uint8_t*)msg + sizeof(struct rndis_set), set_buf, set_len);

    msg->msg_type = RNDIS_MSG_SET;
    msg->msg_len = sizeof(struct rndis_set) + set_len;
    msg->request_id = class->request_id++;
    msg->oid = oid;
    msg->len = set_len;
    msg->offset = 20;

    setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = 0;
    setup->wValue = 0;
    setup->wIndex = intf;
    setup->wLength = sizeof(struct rndis_set) + set_len;

    ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)msg);
    USB_LOG_INFO("usbh_control_transfer RNDIS_MSG_SET ret: %d\r\n", ret);

    setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = CDC_REQUEST_GET_ENCAPSULATED_RESPONSE;
    setup->wValue = 0;
    setup->wIndex = intf;
    setup->wLength = sizeof(struct rndis_set_c) + 64;

    resp = (struct rndis_set_c *)rt_malloc(sizeof(struct rndis_set_c) + 64);

    ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)resp);
    USB_LOG_INFO("CDC_REQUEST_GET_ENCAPSULATED_RESPONSE RNDIS_MSG_SET ret: %d\r\n", ret);

    if(ret == 0)
    {
        dump_hex(resp, sizeof(struct rndis_set_c) + 64);
        USB_LOG_INFO("resp msg type: %08X len: %d id: %08X status: %08X\r\n", resp->msg_type, resp->msg_len, resp->request_id, resp->status);
    }

    return ret;
}

static rt_err_t rt_rndis_eth_init(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t rt_rndis_eth_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t rt_rndis_eth_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t rt_rndis_eth_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_size_t rt_rndis_eth_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    rt_set_errno(-RT_ENOSYS);
    return 0;
}
static rt_err_t rt_rndis_eth_control(rt_device_t dev, int cmd, void *args)
{
    rt_rndis_eth_t rndis_eth_dev = (rt_rndis_eth_t)dev;

    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
    switch(cmd)
    {
    case NIOCTL_GADDR:
        /* get mac address */
        if(args)
        { 
            USB_LOG_INFO("%s L%d NIOCTL_GADDR\r\n", __FUNCTION__, __LINE__);
            rt_memcpy(args, rndis_eth_dev->dev_addr, MAX_ADDR_LEN);
        }    
        else
        { 
            return -RT_ERROR;
        }    
        break;
    default :
        break;
    }

    return RT_EOK;
}

/* reception packet. */
struct pbuf *rt_rndis_eth_rx(rt_device_t dev)
{
    struct pbuf* p = RT_NULL;

    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);

    return p;
}

/* transmit packet. */
rt_err_t rt_rndis_eth_tx(rt_device_t dev, struct pbuf* p)
{
    struct pbuf* q;
    int ret = 0;
    rt_err_t result = RT_EOK;
    uint8_t *tmp_buf = RT_NULL;
    rt_rndis_eth_t rndis_eth = (rt_rndis_eth_t)dev;
    struct usbh_rndis *class = rndis_eth->class;

    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
    // return result;

    // static int tx_count = 0;
    // if (tx_count++ != 3) {
    //     // eth_device_linkchange(&usbh_rndis_eth_device.parent, RT_TRUE);
    //     return result;
    // }

#ifdef DM9051_TX_DUMP
    packet_dump(__FUNCTION__, p);
#endif /* DM9051_TX_DUMP */

    // ret = rndis_keepalive(class);
    // USB_LOG_INFO("rndis_keepalive ret=%d\r\n", ret);

    tmp_buf = (uint8_t *)rt_malloc(sizeof(struct rndis_data_hdr) + p->tot_len);
    if (!tmp_buf) {
        USB_LOG_INFO("[%s L%d], no memory for pbuf, len=%d.", __FUNCTION__, __LINE__, p->tot_len);
        goto _exit;
    }

    struct rndis_data_hdr *data_hdr = (struct rndis_data_hdr *)tmp_buf;

    pbuf_copy_partial(p, tmp_buf + sizeof(struct rndis_data_hdr), p->tot_len, 0);

    data_hdr->msg_type = RNDIS_MSG_PACKET;
    data_hdr->msg_len = sizeof(struct rndis_data_hdr) + p->tot_len;
    data_hdr->data_offset = sizeof(struct rndis_data_hdr) - 8;
    data_hdr->data_len = p->tot_len;
    data_hdr->oob_data_offset = 0;
    data_hdr->oob_data_len = 0;
    data_hdr->num_oob = 0;
    data_hdr->packet_data_offset = 0;
    data_hdr->packet_data_len = 0;
    data_hdr->vc_handle = 0;
    data_hdr->reserved = 0;

    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
    ret = usbh_ep_bulk_transfer(class->bulkout, tmp_buf, data_hdr->msg_len);
    if (ret < 0) {
        printf("bulk out error\r\n");
        USB_LOG_INFO("send over ret:%d\r\n", ret);
        goto _exit;
    }
    USB_LOG_INFO("send over ret:%d\r\n", ret);

#if 0
    rt_thread_delay(10);
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
    memset(cdc_buffer2, 0, sizeof(cdc_buffer2));
    usbh_ep_bulk_async_transfer(class->bulkin, cdc_buffer2, 2048, usbh_cdc_acm_callback, class);
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
#endif

_exit:
    if(tmp_buf)
    {
        rt_free(tmp_buf);
    }

    return result;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops rndis_device_ops =
{
    rt_rndis_eth_init,
    rt_rndis_eth_open,
    rt_rndis_eth_close,
    rt_rndis_eth_read,
    rt_rndis_eth_write,
    rt_rndis_eth_control
}
#endif /* RT_USING_DEVICE_OPS */

static void rt_thread_rndis_data_entry(void *parameter)
{
    int ret, len;
    struct usbh_hubport *hport;
    uint8_t intf;

    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
    rt_thread_delay(1000*1);
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);

    struct usbh_rndis *class = (struct usbh_rndis *)usbh_find_class_instance("/dev/e0");
    if (class == NULL) {
        printf("do not find /dev/ttyACM0\r\n");
        return;
    }
    USB_LOG_INFO("usbh_rndis=%p\r\n", class);

    memset(cdc_buffer, 0, sizeof(cdc_buffer));
    hport = class->hport;
    intf = class->ctrl_intf;
    USB_LOG_INFO("hport=%p, intf=%d.\r\n", hport, intf);

    ret = rndis_init(class);
    USB_LOG_INFO("rndis_init ret=%d\r\n", ret);

#if 0
    int len = sizeof(cdc_buffer);
    ret = rndis_query_oid(class, RNDIS_OID_GEN_SUPPORTED_LIST, cdc_buffer, &len);
    USB_LOG_INFO("rndis_query_oid RNDIS_OID_GEN_SUPPORTED_LIST, ret=%d, len=%d.\r\n", ret, len);
    if(ret == 0)
    {
        dump_hex(cdc_buffer, len);
    }
#endif

    int query_len, result_len;
    len = 4096;//TODO


    query_len = 6;
    memset(cdc_buffer, 0, 512);
    ret = rndis_query_oid(class, RNDIS_OID_GEN_SUPPORTED_LIST, 0, cdc_buffer, len);
    USB_LOG_INFO("rndis_query_oid RNDIS_OID_GEN_SUPPORTED_LIST, ret=%d, len=%d.\r\n", ret, len);
    if (ret == 0) {
        struct rndis_query_c *resp = (struct rndis_query_c *)cdc_buffer;
        dump_hex((const uint8_t *)cdc_buffer + sizeof(struct rndis_query_c), resp->len);
    }

    query_len = 6;
    memset(cdc_buffer, 0, 100);
    ret = rndis_query_oid(class, RNDIS_OID_802_3_PERMANENT_ADDRESS, query_len, cdc_buffer, len);
    USB_LOG_INFO("rndis_query_oid RNDIS_OID_802_3_PERMANENT_ADDRESS, ret=%d, len=%d.\r\n", ret, len);
    if(ret == 0)
    {
        const uint8_t *tmp_buf = (const uint8_t *)cdc_buffer;
        struct rndis_query_c *resp = (struct rndis_query_c *)cdc_buffer;

        dump_hex((const uint8_t *)cdc_buffer + sizeof(struct rndis_query_c), resp->len);
    }
    memset(cdc_buffer, 0, 100);
    ret = rndis_query_oid(class, RNDIS_OID_802_3_CURRENT_ADDRESS, query_len, cdc_buffer, len);
    USB_LOG_INFO("rndis_query_oid RNDIS_OID_802_3_CURRENT_ADDRESS, ret=%d, len=%d.\r\n", ret, len);
    if(ret == 0)
    {
        const uint8_t *tmp_buf = (const uint8_t *)cdc_buffer;
        struct rndis_query_c *resp = (struct rndis_query_c *)cdc_buffer;

        memcpy(&usbh_rndis_eth_device.dev_addr[0], tmp_buf + sizeof(struct rndis_query_c), query_len); // 这里不知道为什么不在0位置，
        dump_hex((const uint8_t *)cdc_buffer + sizeof(struct rndis_query_c), resp->len);
    }

    uint32_t pquery_rlt = 0x0f;
    ret = rndis_msg_set(class, RNDIS_OID_GEN_CURRENT_PACKET_FILTER, &pquery_rlt, 4);
    USB_LOG_INFO("rndis_msg_set RNDIS_OID_GEN_CURRENT_PACKET_FILTER, ret=%d, len=%d.\r\n", ret, len);

    {
        uint8_t MULTICAST_LIST[] = { 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01 };
        ret = rndis_msg_set(class, RNDIS_OID_802_3_MULTICAST_LIST, &MULTICAST_LIST[0], 6);
        USB_LOG_INFO("rndis_msg_set RNDIS_OID_802_3_MULTICAST_LIST, ret=%d, len=%d.\r\n", ret, len);
    }

    for(int ii=0; ii<1; ii++)
    {
        query_len = 4;
        memset(cdc_buffer, 0, 100);
        ret = rndis_query_oid(class, RNDIS_OID_GEN_LINK_SPEED, query_len, cdc_buffer, len);
        USB_LOG_INFO("rndis_query_oid RNDIS_OID_GEN_LINK_SPEED, ret=%d, len=%d.\r\n", ret, len);
        if (ret == 0) {
            struct rndis_query_c *resp = (struct rndis_query_c *)cdc_buffer;
            dump_hex((const uint8_t *)cdc_buffer + sizeof(struct rndis_query_c), resp->len);
        }

        query_len = 4;
        memset(cdc_buffer, 0, 100);
        ret = rndis_query_oid(class, RNDIS_OID_GEN_MEDIA_CONNECT_STATUS, query_len, cdc_buffer, len);
        USB_LOG_INFO("rndis_query_oid RNDIS_OID_GEN_MEDIA_CONNECT_STATUS, ret=%d, len=%d.\r\n", ret, len);
        if (ret == 0) {
            struct rndis_query_c *resp = (struct rndis_query_c *)cdc_buffer;
            dump_hex((const uint8_t *)cdc_buffer + sizeof(struct rndis_query_c), resp->len);
        }

        // rt_thread_delay(1000);
    }

#ifdef RT_USING_DEVICE_OPS
    usbh_rndis_eth_device.parent.parent.ops           = &rndis_device_ops;
#else
    usbh_rndis_eth_device.parent.parent.init          = rt_rndis_eth_init;
    usbh_rndis_eth_device.parent.parent.open          = rt_rndis_eth_open;
    usbh_rndis_eth_device.parent.parent.close         = rt_rndis_eth_close;
    usbh_rndis_eth_device.parent.parent.read          = rt_rndis_eth_read;
    usbh_rndis_eth_device.parent.parent.write         = rt_rndis_eth_write;
    usbh_rndis_eth_device.parent.parent.control       = rt_rndis_eth_control;
#endif
    usbh_rndis_eth_device.parent.parent.user_data     = RT_NULL;

    usbh_rndis_eth_device.parent.eth_rx               = rt_rndis_eth_rx;
    usbh_rndis_eth_device.parent.eth_tx               = rt_rndis_eth_tx;

    usbh_rndis_eth_device.class = class;
    usbh_rndis_eth_device.hport = hport;
    usbh_rndis_eth_device.intf = intf;

    // rt_thread_delay(1000*10);
    // eth_device_init(&usbh_rndis_eth_device.parent, "u0");

#if 1
    for (int ii = 0; ii < 2; ii++) {
        rt_thread_delay(1000 * 2);
        USB_LOG_INFO("%s L%d rndis_keepalive #%d\r\n", __FUNCTION__, __LINE__, ii);
        ret = rndis_keepalive(class);
        USB_LOG_INFO("%s L%d rndis_keepalive ret=%d\r\n", __FUNCTION__, __LINE__, ret);

        if(ret == 123456789)
        {
            USB_LOG_INFO("%s L%d rndis_keepalive ret=%d RNDIS_MSG_INDICATE\r\n", __FUNCTION__, __LINE__, ret);
            break;
        }
    }
    eth_device_init(&usbh_rndis_eth_device.parent, "u0");
    eth_device_linkchange(&usbh_rndis_eth_device.parent, RT_FALSE);

    // wait link up
    for(int ii=0; ii<10; ii++)
    {
        query_len = 4;
        memset(cdc_buffer, 0, 100);
        ret = rndis_query_oid(class, RNDIS_OID_GEN_LINK_SPEED, query_len, cdc_buffer, len);
        USB_LOG_INFO("rndis_query_oid RNDIS_OID_GEN_LINK_SPEED, ret=%d, len=%d.\r\n", ret, len);
        if (ret == 0) {
            struct rndis_query_c *resp = (struct rndis_query_c *)cdc_buffer;
            dump_hex((const uint8_t *)cdc_buffer + sizeof(struct rndis_query_c), resp->len);
        }

        query_len = 4;
        memset(cdc_buffer, 0, 100);
        ret = rndis_query_oid(class, RNDIS_OID_GEN_MEDIA_CONNECT_STATUS, query_len, cdc_buffer, len);
        USB_LOG_INFO("rndis_query_oid RNDIS_OID_GEN_MEDIA_CONNECT_STATUS, ret=%d, len=%d.\r\n", ret, len);
        if (ret == 0) {
            uint32_t *link;
            struct rndis_query_c *resp = (struct rndis_query_c *)cdc_buffer;
            dump_hex((const uint8_t *)cdc_buffer + sizeof(struct rndis_query_c), resp->len);

            link = (uint32_t *)((const uint8_t *)cdc_buffer + sizeof(struct rndis_query_c));
            if(*link)
            {
                USB_LOG_INFO("LINK DOWN\r\n\r\n\r\n\r\n");
                eth_device_linkchange(&usbh_rndis_eth_device.parent, RT_FALSE);
            }
            else
            {
                USB_LOG_INFO("LINK UP\r\n\r\n\r\n\r\n");
                eth_device_linkchange(&usbh_rndis_eth_device.parent, RT_TRUE);
                break;
            }
        }

        // rt_thread_delay(1000);
    }

    // return;
#endif

#if 0
    ret = rndis_keepalive(class);
    USB_LOG_INFO("%s L%d rndis_keepalive ret=%d\r\n", __FUNCTION__, __LINE__, ret);

    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
    memset(cdc_buffer2, 0, sizeof(cdc_buffer2));
    usbh_ep_bulk_async_transfer(class->bulkin, cdc_buffer2, 2048, usbh_cdc_acm_callback, class);
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
    return;
#endif

#if 0
    while (1) {
        // ret = rndis_keepalive(class);
        // USB_LOG_INFO("%s L%d rndis_keepalive ret=%d\r\n", __FUNCTION__, __LINE__, ret);

        memset(cdc_buffer2, 0, sizeof(cdc_buffer2));
        USB_LOG_INFO("try async_transfer %s L%d\r\n", __FUNCTION__, __LINE__);
        usbh_ep_bulk_async_transfer(class->bulkin, cdc_buffer2, 2048, usbh_cdc_acm_callback, class);
        USB_LOG_INFO("async_transfer %s L%d\r\n", __FUNCTION__, __LINE__);

        rt_thread_delay(200);
    }
    return;
#endif

    while (1) {
        ret = rndis_keepalive(class);
        USB_LOG_INFO("%s L%d rndis_keepalive ret=%d\r\n", __FUNCTION__, __LINE__, ret);

        rt_thread_delay(10);

        memset(cdc_buffer2, 0, sizeof(cdc_buffer2));
        ret = usbh_ep_bulk_transfer(class->bulkin, cdc_buffer2, 2048);
        if (ret < 0) {
            USB_LOG_WRN("%s L%d bulk in error ret=%d\r\n", __FUNCTION__, __LINE__, ret);
            continue;
        }
        USB_LOG_INFO("%s L%d bulkin ret=%d\r\n", __FUNCTION__, __LINE__, ret);

        struct rndis_data_hdr *data_hdr = (struct rndis_data_hdr *)cdc_buffer2;
        if (data_hdr->msg_type == RNDIS_MSG_PACKET) {
            struct pbuf *p = RT_NULL;
            /* allocate buffer           */
            p = pbuf_alloc(PBUF_LINK, data_hdr->msg_len, PBUF_RAM);
            if (p != NULL) {
                const uint8_t *tmp_buf = (const uint8_t *)cdc_buffer2;

                // memcpy((void *)p->payload, tmp_buf + sizeof(struct rndis_data_hdr), data_hdr->msg_len);
                pbuf_take(p, tmp_buf + sizeof(struct rndis_data_hdr), data_hdr->msg_len);

#ifdef DM9051_RX_DUMP
                // if (p)
                packet_dump(__FUNCTION__, p);
                // dump_hex(p->payload, data_hdr->msg_len);
#endif /* DM9051_RX_DUMP */

                struct eth_device *eth_dev = &usbh_rndis_eth_device.parent;
                if ((eth_dev->netif->input(p, eth_dev->netif)) != ERR_OK) {
                    USB_LOG_INFO("F:%s L:%d IP input error", __FUNCTION__, __LINE__);
                    pbuf_free(p);
                    p = RT_NULL;
                }
                USB_LOG_INFO("%s L%d input OK\r\n", __FUNCTION__, __LINE__);
            } else {
                USB_LOG_ERR("%s L%d pbuf_alloc NULL\r\n", __FUNCTION__, __LINE__);
            }
        } else {
            USB_LOG_WRN("%s L%d msg_type != RNDIS_MSG_PACKET\r\n", __FUNCTION__, __LINE__);
            dump_hex(data_hdr, 32);
        }
    } // while (1)

    return;



#if 0 
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
    ret = usbh_ep_bulk_transfer(class->bulkin, cdc_buffer, 512);
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
    ret = usbh_ep_bulk_transfer(class->bulkout, cdc_buffer, data_hdr->msg_len);
    if (ret < 0) {
        printf("bulk out error\r\n");
        return;
    }
    USB_LOG_INFO("send over ret:%d\r\n", ret);
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);

#if 0
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
    usbh_ep_bulk_async_transfer(class->bulkin, cdc_buffer, 2048, usbh_cdc_acm_callback, class);
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);
#endif

}

int rndis_test(void)
{
    int ret = 0;

    rt_thread_t tid = rt_thread_create("rndis",
                                       rt_thread_rndis_data_entry,
                                       0,
                                       1024 * 6,
                                       6,
                                       20);
    if(tid)
    {
        USB_LOG_INFO("%s L%d rt_thread_startup rndis\r\n", __FUNCTION__, __LINE__);
        rt_thread_startup(tid);
    }
    USB_LOG_INFO("%s L%d\r\n", __FUNCTION__, __LINE__);

    return ret;
}
// MSH_CMD_EXPORT(rndis_test, rndis_test.);

extern int usbh_initialize(void);
MSH_CMD_EXPORT_ALIAS(usbh_initialize, usb, CherryUSB initial.);

