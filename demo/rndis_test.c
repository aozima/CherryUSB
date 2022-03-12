#include <rtthread.h>
#include <string.h>

#include "usbh_core.h"
#include <class/wireless/usbh_rndis.h>

static uint8_t cdc_buffer[4096];

int rndis_test(void)
{
    int ret;

    struct usbh_rndis *cdc_acm_class = (struct usbh_rndis *)usbh_find_class_instance("/dev/e1");
    if (cdc_acm_class == NULL) {
        printf("do not find /dev/ttyACM0\r\n");
        return -1;
    }

    memset(cdc_buffer, 0, 512);
    USB_LOG_INFO("%s %d\r\n", __FUNCTION__, __LINE__);
    ret = usbh_ep_bulk_transfer(cdc_acm_class->bulkin, cdc_buffer, 512);
    if (ret < 0) {
        printf("bulk in error\r\n");
        return ret;
    }

    printf("recv over:%d\r\n", ret);
    for (size_t i = 0; i < ret; i++) {
        printf("0x%02x ", cdc_buffer[i]);
    }
    printf("\r\n");
    const uint8_t data1[10] = { 0x02, 0x00, 0x00, 0x00, 0x02, 0x02, 0x08, 0x14 };

    memcpy(cdc_buffer, data1, 8);
    USB_LOG_INFO("%s %d\r\n", __FUNCTION__, __LINE__);
    ret = usbh_ep_bulk_transfer(cdc_acm_class->bulkout, cdc_buffer, 8);
    if (ret < 0) {
        printf("bulk out error\r\n");
        return ret;
    }
    printf("send over:%d\r\n", ret);
    USB_LOG_INFO("%s %d\r\n", __FUNCTION__, __LINE__);

    return ret;
}
MSH_CMD_EXPORT(rndis_test, rndis_test.);
