#include "usbh_core.h"
#include "usbh_rndis.h"

static const char *DEV_FORMAT = "/dev/e%d";

#if 0
static int rndis_init(struct usbh_hubport *hport, uint8_t intf)
{
    int ret = 0;
    struct usb_setup_packet *setup;
    struct usbh_rndis *class = (struct usbh_rndis *)hport->config.intf[intf].priv;
	union {
		// void			*buf;
		// struct rndis_msg_hdr	*header;
		struct rndis_msg_init	init;
		struct rndis_msg_query	query;
	} u;
    setup = hport->setup;

    /*set config*/
    setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_STANDARD | USB_REQUEST_RECIPIENT_DEVICE;
    setup->bRequest = 9; // 9=USB_REQ_SET_CONFIGURATION
    setup->wValue = 0;
    setup->wIndex = 0;
    setup->wLength = 0;
    setup->wValue = 1;

    ret = usbh_control_transfer(hport->ep0, setup, NULL);
    USB_LOG_ERR("set config ret: %d\r\n", ret);

    /* set interface */

    /* init */
    struct rndis_msg_init *msg = &u.init;
    msg->msg_type = RNDIS_MSG_INIT;
    msg->msg_len = sizeof(struct rndis_msg_init);
    msg->request_id = 0;
    msg->major_version = 1;
    msg->minor_version = 0;
    msg->max_transfer_size = 0x4000;

    setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = 0;
    setup->wValue = 0;
    setup->wIndex = intf;
    setup->wLength = sizeof(struct rndis_msg_init);

    ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)msg);
    USB_LOG_ERR("init ret: %d\r\n", ret);

    /* query */
    struct rndis_msg_query *query = &u.query;
    // query->msg_type = RNDIS_MSG_QUERY;
    // query->msg_len = sizeof(struct rndis_msg_query);

    struct rndis_msg_keepalive msg1;
    msg1.msg_type = 8;
    msg1.msg_len = sizeof(struct rndis_msg_keepalive);
    msg1.request_id = 1;

    setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = 0;
    setup->wValue = 0;
    setup->wIndex = intf;
    setup->wLength = sizeof(struct rndis_msg_keepalive);

    // ret = usbh_control_transfer(hport->ep0, setup, (uint8_t *)&msg1);
    USB_LOG_ERR("keepalive ret: %d\r\n", ret);

    return ret;
}
#endif

static int usbh_rndis_connect(struct usbh_hubport *hport, uint8_t intf)
{
    int ret = 0;
    struct usbh_endpoint_cfg ep_cfg = { 0 };
    struct usb_endpoint_descriptor *ep_desc;

    USB_LOG_INFO("%s %d\r\n", __FUNCTION__, __LINE__);

    struct usbh_rndis *class = usb_malloc(sizeof(struct usbh_rndis));
    if (class == NULL) 
    {
        USB_LOG_ERR("Fail to alloc class\r\n");
        return -ENOMEM;
    }
    memset(class, 0, sizeof(struct usbh_rndis));

    class->hport = hport;
    class->ctrl_intf = intf;
    class->data_intf = intf + 1;

    snprintf(hport->config.intf[intf].devname, CONFIG_USBHOST_DEV_NAMELEN, DEV_FORMAT, intf);
    USB_LOG_INFO("Register RNDIS Class:%s\r\n", hport->config.intf[intf].devname);
    hport->config.intf[intf].priv = class;

#if 1
    USB_LOG_INFO("hport=%p, intf=%d, intf_desc.bNumEndpoints:%d\r\n", hport, intf, hport->config.intf[intf].intf_desc.bNumEndpoints);
    for (uint8_t i = 0; i < hport->config.intf[intf].intf_desc.bNumEndpoints; i++) 
    {
        ep_desc = &hport->config.intf[intf].ep[i].ep_desc;

        USB_LOG_INFO("ep[%d] bLength=%d, type=%d\r\n", i, ep_desc->bLength, ep_desc->bDescriptorType);
        USB_LOG_INFO("ep_addr=%02X, attr=%02X\r\n", ep_desc->bEndpointAddress, ep_desc->bmAttributes & USB_ENDPOINT_TYPE_MASK);
        USB_LOG_INFO("wMaxPacketSize=%d, bInterval=%d\r\n\r\n", ep_desc->wMaxPacketSize, ep_desc->bInterval);
    }
#endif

#if 1
    USB_LOG_INFO("hport=%p, intf=%d, intf_desc.bNumEndpoints:%d\r\n", hport, intf + 1, hport->config.intf[intf + 1].intf_desc.bNumEndpoints);
    for (uint8_t i = 0; i < hport->config.intf[intf + 1].intf_desc.bNumEndpoints; i++) 
    {
        ep_desc = &hport->config.intf[intf + 1].ep[i].ep_desc;

        USB_LOG_INFO("ep[%d] bLength=%d, type=%d\r\n", i, ep_desc->bLength, ep_desc->bDescriptorType);
        USB_LOG_INFO("ep_addr=%02X, attr=%02X\r\n", ep_desc->bEndpointAddress, ep_desc->bmAttributes & USB_ENDPOINT_TYPE_MASK);
        USB_LOG_INFO("wMaxPacketSize=%d, bInterval=%d\r\n\r\n", ep_desc->wMaxPacketSize, ep_desc->bInterval);
    }
#endif

    for (uint8_t i = 0; i < hport->config.intf[intf + 1].intf_desc.bNumEndpoints; i++) 
    {
        ep_desc = &hport->config.intf[intf + 1].ep[i].ep_desc;

        ep_cfg.ep_addr = ep_desc->bEndpointAddress;
        ep_cfg.ep_type = ep_desc->bmAttributes & USB_ENDPOINT_TYPE_MASK;
        ep_cfg.ep_mps = ep_desc->wMaxPacketSize;
        ep_cfg.ep_interval = ep_desc->bInterval;
        ep_cfg.hport = hport;
        if (ep_desc->bEndpointAddress & 0x80) {
            usbh_ep_alloc(&class->bulkin, &ep_cfg);
        } else {
            usbh_ep_alloc(&class->bulkout, &ep_cfg);
        }
    }

    // ret = rndis_init(hport, intf);
    // if (ret != 0) 
    // {
    //     USB_LOG_ERR("rndis_init ret: %d\r\n", ret);
    //     return -ENOMEM;
    // }

    extern int rndis_test(void);
    rndis_test();

    return ret;
}

static int usbh_rndis_data_connect(struct usbh_hubport *hport, uint8_t intf)
{
    int ret = 0;
    struct usbh_endpoint_cfg ep_cfg = { 0 };
    struct usb_endpoint_descriptor *ep_desc;

    USB_LOG_INFO("%s %d\r\n", __FUNCTION__, __LINE__);

    struct usbh_rndis *class = usb_malloc(sizeof(struct usbh_rndis));
    if (class == NULL) 
    {
        USB_LOG_ERR("Fail to alloc class\r\n");
        return -ENOMEM;
    }
    memset(class, 0, sizeof(struct usbh_rndis));

    class->hport = hport;
    class->data_intf = intf;

    snprintf(hport->config.intf[intf].devname, CONFIG_USBHOST_DEV_NAMELEN, DEV_FORMAT, intf);
    USB_LOG_INFO("Register RNDIS CDC DATA Class:%s\r\n", hport->config.intf[intf].devname);
    hport->config.intf[intf].priv = class;

#if 1
    USB_LOG_INFO("hport=%p, intf=%d, intf_desc.bNumEndpoints:%d\r\n", hport, intf, hport->config.intf[intf].intf_desc.bNumEndpoints);
    for (uint8_t i = 0; i < hport->config.intf[intf].intf_desc.bNumEndpoints; i++) 
    {
        ep_desc = &hport->config.intf[intf].ep[i].ep_desc;

        USB_LOG_INFO("ep[%d] bLength=%d, type=%d\r\n", i, ep_desc->bLength, ep_desc->bDescriptorType);
        USB_LOG_INFO("ep_addr=%02X, attr=%02X\r\n", ep_desc->bEndpointAddress, ep_desc->bmAttributes & USB_ENDPOINT_TYPE_MASK);
        USB_LOG_INFO("wMaxPacketSize=%d, bInterval=%d\r\n\r\n", ep_desc->wMaxPacketSize, ep_desc->bInterval);
    }
#endif

    for (uint8_t i = 0; i < hport->config.intf[intf + 1].intf_desc.bNumEndpoints; i++) 
    {
        ep_desc = &hport->config.intf[intf + 1].ep[i].ep_desc;

        ep_cfg.ep_addr = ep_desc->bEndpointAddress;
        ep_cfg.ep_type = ep_desc->bmAttributes & USB_ENDPOINT_TYPE_MASK;
        ep_cfg.ep_mps = ep_desc->wMaxPacketSize;
        ep_cfg.ep_interval = ep_desc->bInterval;
        ep_cfg.hport = hport;
        if (ep_desc->bEndpointAddress & 0x80) {
            usbh_ep_alloc(&class->bulkin, &ep_cfg);
        } else {
            usbh_ep_alloc(&class->bulkout, &ep_cfg);
        }
    }

    return ret;
}

static int usbh_rndis_disconnect(struct usbh_hubport *hport, uint8_t intf)
{
    int ret = 0;

    USB_LOG_ERR("TBD: %s %d\r\n", __FUNCTION__, __LINE__);
    return ret;
}

// Class:0xe0,Subclass:0x01,Protocl:0x03: 只有1个用于中断通知的endpoint
const struct usbh_class_driver rndis_class_driver = {
    .driver_name = "rndis",
    .connect = usbh_rndis_connect,
    .disconnect = usbh_rndis_disconnect
};

// Class:0x0a,Subclass:0x00,Protocl:0x00: bulkin & bulkout
const struct usbh_class_driver rndis_data_class_driver = {
    .driver_name = "rndis_data",
    .connect = usbh_rndis_data_connect,
    .disconnect = usbh_rndis_disconnect
};
