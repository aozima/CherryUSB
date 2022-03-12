#ifndef _USB_CLASHH_RNDIS_H__
#define _USB_CLASHH_RNDIS_H__

#include "usbh_core.h"

extern const struct usbh_class_driver msc_class_driver;

struct usbh_rndis {
    usbh_epinfo_t bulkin;  /* Bulk IN endpoint */
    usbh_epinfo_t bulkout; /* Bulk OUT endpoint */
    usbh_epinfo_t int_notify; /* Notify endpoint */
};

#endif /* _USB_CLASHH_RNDIS_H__ */
