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

static uint8_t cdc_buffer[4096];
static void usbh_cdc_acm_callback(void *arg, int nbytes)
{
    //struct usbh_cdc_acm *rndis_class = (struct usbh_cdc_acm *)arg;

    USB_LOG_INFO("%s %d\r\n", __FUNCTION__, __LINE__);
    if (nbytes > 0) {
        for (size_t i = 0; i < nbytes; i++) {
            printf("0x%02x ", cdc_buffer[i]);
        }
    }

    printf("nbytes:%d\r\n", nbytes);
}

static int rndis_init(struct usbh_rndis *class, struct usbh_hubport *hport, uint8_t intf)
{
    int ret = 0;
    struct usb_setup_packet *setup;

    USB_LOG_INFO("%s %d\r\n", __FUNCTION__, __LINE__);

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

int rndis_test(void)
{
    int ret;
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
    USB_LOG_INFO("hport=%p, intf=%d, intf_desc.bNumEndpoints:%d\r\n", hport, intf);

    ret = rndis_init(rndis_class, hport, 0);

#if 1
    usbh_ep_bulk_async_transfer(rndis_class->bulkin, cdc_buffer, 2048, usbh_cdc_acm_callback, rndis_class);
#endif

#if 0    
    USB_LOG_INFO("%s %d\r\n", __FUNCTION__, __LINE__);
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

    const uint8_t data1[10] = { 0x02, 0x00, 0x00, 0x00, 0x02, 0x02, 0x08, 0x14 };

    memcpy(cdc_buffer, data1, 8);
    USB_LOG_INFO("%s %d\r\n", __FUNCTION__, __LINE__);
    ret = usbh_ep_bulk_transfer(rndis_class->bulkout, cdc_buffer, 8);
    if (ret < 0) {
        printf("bulk out error\r\n");
        return ret;
    }
    USB_LOG_INFO("send over ret:%d\r\n", ret);
    USB_LOG_INFO("%s %d\r\n", __FUNCTION__, __LINE__);

    return ret;
}
MSH_CMD_EXPORT(rndis_test, rndis_test.);
