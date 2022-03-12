#include "usbh_core.h"
#include "usbh_rndis.h"

#define DEV_FORMAT "/dev/e%c"

static int usbh_rndis_connect(struct usbh_hubport *hport, uint8_t intf)
{
    USB_LOG_ERR("TBD: %s %d\r\n", __FUNCTION__, __LINE__);
    return -ENOMEM;
}

static int usbh_rndis_disconnect(struct usbh_hubport *hport, uint8_t intf)
{
    USB_LOG_ERR("TBD: %s %d\r\n", __FUNCTION__, __LINE__);
    return -ENOMEM;
}

const struct usbh_class_driver rndis_class_driver = {
    .driver_name = "rndis",
    .connect = usbh_rndis_connect,
    .disconnect = usbh_rndis_disconnect
};

