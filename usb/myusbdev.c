#include <furi.h>
#include <furi_hal_version.h>
#include <cli/cli.h>
#include "myusbdev.h"
#include "my_cli_vcp.h"

/* TODO: HID report: sensor */
static const uint8_t hid_sensor_report_desc[] = {
    HID_USAGE_PAGE_SENSOR,
    HID_USAGE_SENSOR_TYPE_ENVIRONMENTAL_TEMPERATURE,
    HID_COLLECTION(Physical),

    //feature reports (xmit/receive)
    HID_USAGE_PAGE_SENSOR,

    HID_USAGE_SENSOR_PROPERTY_MINIMUM_REPORT_INTERVAL,
    HID_LOGICAL_MIN_8(HID_INTERVAL),
    HID_LOGICAL_MAX_16(0xFF, 0xFF),
    HID_REPORT_SIZE(16),
    HID_REPORT_COUNT(1),
    HID_UNIT_EXPONENT(0),
    HID_FEATURE(Data_Var_Abs),

    HID_USAGE_SENSOR_PROPERTY_REPORT_INTERVAL,
    HID_LOGICAL_MIN_8(HID_INTERVAL),
    HID_LOGICAL_MAX_16(0xFF, 0xFF),
    HID_REPORT_SIZE(16),
    HID_REPORT_COUNT(1),
    HID_UNIT_EXPONENT(0),
    HID_FEATURE(Data_Var_Abs),

    HID_USAGE_SENSOR_DATA(
        HID_USAGE_SENSOR_DATA_ENVIRONMENTAL_TEMPERATURE,
        HID_USAGE_SENSOR_DATA_MOD_MAX),
    HID_LOGICAL_MIN_16(0x01, 0x80), // LOGICAL_MINIMUM (-32767)
    HID_LOGICAL_MAX_16(0xFF, 0x7F), // LOGICAL_MAXIMUM (32767)
    HID_REPORT_SIZE(16),
    HID_REPORT_COUNT(1),
    HID_UNIT_EXPONENT(
        0x0E), // scale default unit “Celsius” to provide 2 digits past the decimal point
    HID_FEATURE(Data_Var_Abs),

    HID_USAGE_SENSOR_DATA(
        HID_USAGE_SENSOR_DATA_ENVIRONMENTAL_TEMPERATURE,
        HID_USAGE_SENSOR_DATA_MOD_MIN),
    HID_LOGICAL_MIN_16(0x01, 0x80), // LOGICAL_MINIMUM (-32767)
    HID_LOGICAL_MAX_16(0xFF, 0x7F), // LOGICAL_MAXIMUM (32767)
    HID_REPORT_SIZE(16),
    HID_REPORT_COUNT(1),
    HID_UNIT_EXPONENT(
        0x0E), // scale default unit “Celsius” to provide 2 digits past the decimal point
    HID_FEATURE(Data_Var_Abs),

    //input reports (transmit)
    HID_USAGE_PAGE_SENSOR,

    HID_USAGE_SENSOR_STATE,
    HID_LOGICAL_MIN_8(0),
    HID_LOGICAL_MAX_8(6),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(1),
    HID_COLLECTION(Logical),
    HID_USAGE_SENSOR_STATE_UNKNOWN,
    HID_USAGE_SENSOR_STATE_READY,
    HID_USAGE_SENSOR_STATE_NOT_AVAILABLE,
    HID_USAGE_SENSOR_STATE_NO_DATA,
    HID_USAGE_SENSOR_STATE_INITIALIZING,
    HID_USAGE_SENSOR_STATE_ACCESS_DENIED,
    HID_USAGE_SENSOR_STATE_ERROR,
    HID_INPUT(Data_Arr_Abs),
    HID_END_COLLECTION,

    HID_USAGE_SENSOR_EVENT,
    HID_LOGICAL_MIN_8(0),
    HID_LOGICAL_MAX_8(16),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(1),
    HID_COLLECTION(Logical),
    HID_USAGE_SENSOR_EVENT_UNKNOWN,
    HID_USAGE_SENSOR_EVENT_STATE_CHANGED,
    HID_USAGE_SENSOR_EVENT_PROPERTY_CHANGED,
    HID_USAGE_SENSOR_EVENT_DATA_UPDATED,
    HID_USAGE_SENSOR_EVENT_POLL_RESPONSE,
    HID_USAGE_SENSOR_EVENT_CHANGE_SENSITIVITY,
    HID_USAGE_SENSOR_EVENT_MAX_REACHED,
    HID_USAGE_SENSOR_EVENT_MIN_REACHED,
    HID_USAGE_SENSOR_EVENT_HIGH_THRESHOLD_CROSS_UPWARD,
    HID_USAGE_SENSOR_EVENT_HIGH_THRESHOLD_CROSS_DOWNWARD,
    HID_USAGE_SENSOR_EVENT_LOW_THRESHOLD_CROSS_UPWARD,
    HID_USAGE_SENSOR_EVENT_LOW_THRESHOLD_CROSS_DOWNWARD,
    HID_USAGE_SENSOR_EVENT_ZERO_THRESHOLD_CROSS_UPWARD,
    HID_USAGE_SENSOR_EVENT_ZERO_THRESHOLD_CROSS_DOWNWARD,
    HID_USAGE_SENSOR_EVENT_PERIOD_EXCEEDED,
    HID_USAGE_SENSOR_EVENT_FREQUENCY_EXCEEDED,
    HID_USAGE_SENSOR_EVENT_COMPLEX_TRIGGER,
    HID_INPUT(Data_Arr_Abs),
    HID_END_COLLECTION,

    HID_USAGE_SENSOR_DATA_ENVIRONMENTAL_TEMPERATURE,
    HID_LOGICAL_MIN_16(0x01, 0x80), // LOGICAL_MINIMUM (-32767)
    HID_LOGICAL_MAX_16(0xFF, 0x7F), // LOGICAL_MAXIMUM (32767)
    HID_REPORT_SIZE(16),
    HID_REPORT_COUNT(1),
    HID_UNIT_EXPONENT(
        0x0E), // scale default unit “Celsius” to provide 2 digits past the decimal point
    HID_INPUT(Data_Var_Abs),
    HID_END_COLLECTION};

static const struct usb_string_descriptor dev_manuf_desc = USB_STRING_DESC("Flipper Devices");
static const struct usb_string_descriptor dev_prod_desc = USB_STRING_DESC("Helloworld");
static const struct usb_string_descriptor dev_serial_desc = USB_STRING_DESC("IDK");

/* Device descriptor */
static const struct usb_device_descriptor hid_sensor_device_desc = {
    .bLength = sizeof(struct usb_device_descriptor),
    .bDescriptorType = USB_DTYPE_DEVICE,
    .bcdUSB = VERSION_BCD(2, 0, 0),
    .bDeviceClass = USB_CLASS_IAD,
    .bDeviceSubClass = USB_SUBCLASS_IAD,
    .bDeviceProtocol = USB_PROTO_IAD,
    .bMaxPacketSize0 = USB_EP0_SIZE,
    .idVendor = USBD_VID,
    .idProduct = USBD_PID,
    .bcdDevice = VERSION_BCD(1, 0, 0),
    .iManufacturer = UsbDevManuf,
    .iProduct = UsbDevProduct,
    .iSerialNumber = UsbDevSerial,
    .bNumConfigurations = 1,
};

/* Device configuration descriptor */
static const struct CompositeConfigDescriptor composite_cfg_desc =
    {.config =
         {
             .bLength = sizeof(struct usb_config_descriptor),
             .bDescriptorType = USB_DTYPE_CONFIGURATION,
             .wTotalLength = sizeof(struct CompositeConfigDescriptor),
             .bNumInterfaces = 3,
             .bConfigurationValue = 1,
             .iConfiguration = NO_DESCRIPTOR,
             .bmAttributes = USB_CFG_ATTR_RESERVED | USB_CFG_ATTR_SELFPOWERED,
             .bMaxPower = USB_CFG_POWER_MA(100),
         },
     .iad_0 =
         {
             .hid =
                 {
                     .bLength = sizeof(struct usb_interface_descriptor),
                     .bDescriptorType = USB_DTYPE_INTERFACE,
                     .bInterfaceNumber = 0,
                     .bAlternateSetting = 0,
                     .bNumEndpoints = 2,
                     .bInterfaceClass = USB_CLASS_HID,
                     .bInterfaceSubClass = USB_HID_SUBCLASS_NONBOOT,
                     .bInterfaceProtocol = USB_HID_PROTO_NONBOOT,
                     .iInterface = NO_DESCRIPTOR,
                 },
             .hid_desc =
                 {
                     .bLength = sizeof(struct usb_hid_descriptor),
                     .bDescriptorType = USB_DTYPE_HID,
                     .bcdHID = VERSION_BCD(1, 0, 0),
                     .bCountryCode = USB_HID_COUNTRY_NONE,
                     .bNumDescriptors = 1,
                     .bDescriptorType0 = USB_DTYPE_HID_REPORT,
                     .wDescriptorLength0 = sizeof(hid_sensor_report_desc),
                 },
             .hid_ep_in =
                 {
                     .bLength = sizeof(struct usb_endpoint_descriptor),
                     .bDescriptorType = USB_DTYPE_ENDPOINT,
                     .bEndpointAddress = HID_EP_IN,
                     .bmAttributes = USB_EPTYPE_INTERRUPT,
                     .wMaxPacketSize = HID_EP_PACKET_SIZE,
                     .bInterval = HID_INTERVAL,
                 },
             .hid_ep_out =
                 {
                     .bLength = sizeof(struct usb_endpoint_descriptor),
                     .bDescriptorType = USB_DTYPE_ENDPOINT,
                     .bEndpointAddress = HID_EP_OUT,
                     .bmAttributes = USB_EPTYPE_INTERRUPT,
                     .wMaxPacketSize = HID_EP_PACKET_SIZE,
                     .bInterval = HID_INTERVAL,
                 },
         },
     .iad_1 = {
         .comm_iad =
             {
                 .bLength = sizeof(struct usb_iad_descriptor),
                 .bDescriptorType = USB_DTYPE_INTERFASEASSOC,
                 .bFirstInterface = 1,
                 .bInterfaceCount = 2,
                 .bFunctionClass = USB_CLASS_CDC,
                 .bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
                 .bFunctionProtocol = USB_PROTO_NONE,
                 .iFunction = NO_DESCRIPTOR,
             },
         .comm =
             {
                 .bLength = sizeof(struct usb_interface_descriptor),
                 .bDescriptorType = USB_DTYPE_INTERFACE,
                 .bInterfaceNumber = 0 + 1,
                 .bAlternateSetting = 0,
                 .bNumEndpoints = 1,
                 .bInterfaceClass = USB_CLASS_CDC,
                 .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
                 .bInterfaceProtocol = USB_PROTO_NONE,
                 .iInterface = NO_DESCRIPTOR,
             },
         .cdc_hdr =
             {
                 .bFunctionLength = sizeof(struct usb_cdc_header_desc),
                 .bDescriptorType = USB_DTYPE_CS_INTERFACE,
                 .bDescriptorSubType = USB_DTYPE_CDC_HEADER,
                 .bcdCDC = VERSION_BCD(1, 1, 0),
             },
         .cdc_mgmt =
             {
                 .bFunctionLength = sizeof(struct usb_cdc_call_mgmt_desc),
                 .bDescriptorType = USB_DTYPE_CS_INTERFACE,
                 .bDescriptorSubType = USB_DTYPE_CDC_CALL_MANAGEMENT,
                 .bmCapabilities = 0,
                 .bDataInterface = 1 + 1,
             },
         .cdc_acm =
             {
                 .bFunctionLength = sizeof(struct usb_cdc_acm_desc),
                 .bDescriptorType = USB_DTYPE_CS_INTERFACE,
                 .bDescriptorSubType = USB_DTYPE_CDC_ACM,
                 .bmCapabilities = 0,
             },
         .cdc_union =
             {
                 .bFunctionLength = sizeof(struct usb_cdc_union_desc),
                 .bDescriptorType = USB_DTYPE_CS_INTERFACE,
                 .bDescriptorSubType = USB_DTYPE_CDC_UNION,
                 .bMasterInterface0 = 0 + 1,
                 .bSlaveInterface0 = 1 + 1,
             },
         .comm_ep =
             {
                 .bLength = sizeof(struct usb_endpoint_descriptor),
                 .bDescriptorType = USB_DTYPE_ENDPOINT,
                 .bEndpointAddress = CDC_EP_NTF,
                 .bmAttributes = USB_EPTYPE_INTERRUPT,
                 .wMaxPacketSize = CDC_EP_NTF_PACKETSIZE,
                 .bInterval = 0xFF,
             },
         .data =
             {
                 .bLength = sizeof(struct usb_interface_descriptor),
                 .bDescriptorType = USB_DTYPE_INTERFACE,
                 .bInterfaceNumber = 1 + 1,
                 .bAlternateSetting = 0,
                 .bNumEndpoints = 2,
                 .bInterfaceClass = USB_CLASS_CDC_DATA,
                 .bInterfaceSubClass = USB_SUBCLASS_NONE,
                 .bInterfaceProtocol = USB_PROTO_NONE,
                 .iInterface = NO_DESCRIPTOR,
             },
         .data_eptx =
             {
                 .bLength = sizeof(struct usb_endpoint_descriptor),
                 .bDescriptorType = USB_DTYPE_ENDPOINT,
                 .bEndpointAddress = CDC_EP_IN,
                 .bmAttributes = USB_EPTYPE_BULK,
                 .wMaxPacketSize = CDC_EP_PACKET_SIZE,
                 .bInterval = 0x01,
             },
         .data_eprx =
             {
                 .bLength = sizeof(struct usb_endpoint_descriptor),
                 .bDescriptorType = USB_DTYPE_ENDPOINT,
                 .bEndpointAddress = CDC_EP_OUT,
                 .bmAttributes = USB_EPTYPE_BULK,
                 .wMaxPacketSize = CDC_EP_PACKET_SIZE,
                 .bInterval = 0x01,
             },
     }};

static void composite_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx);
static void composite_deinit(usbd_device* dev);
static void composite_on_wakeup(usbd_device* dev);
static void composite_on_suspend(usbd_device* dev);
static usbd_respond composite_ep_config(usbd_device* dev, uint8_t cfg);
static usbd_respond
    composite_ctrlreq_handler(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback);

static CdcCallbacks cdc_cbs;
static HidCallbacks hid_cbs;

static Cli* cli_handle;

static struct CompositeUsbDevice usbd = {
    .usb_dev = NULL,
    .prev_intf = NULL,
    .usb_connected = false,
    .cdc_config = {},
    .cdc_ctrl_line_state = 0,
    .sensor_feature_report =
        {
            .min_interval = HID_INTERVAL,
            .interval = SENSOR_POLL_PERIOD,
            .max_temp = SENSOR_TEMPC_MAX * 100,
            .min_temp = SENSOR_TEMPC_MIN * 100,
        },
    .cdc_tx_semaphore = NULL,
    .hid_sensor_semaphore = NULL,
    .cdc_callbacks = &cdc_cbs,
    .cdc_cb_ctx = NULL,
    .hid_callbacks = &hid_cbs,
    .hid_cb_ctx = NULL};

static void log_callback(const uint8_t* data, size_t size, void* context);
static FuriLogHandler log_handler = {.callback = log_callback, .context = NULL};

static struct HidSensorTempReport tempature_report = {
    .state = SENSOR_STATE_UNKNOWN,
    .event = SENSOR_EVENT_UNKNOWN,
    .temperature = 0};

static void composite_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx) {
    UNUSED(intf);
    UNUSED(ctx);

    if(usbd.hid_sensor_semaphore == NULL) {
        usbd.hid_sensor_semaphore = furi_semaphore_alloc(1, 1);
    }

    if(usbd.cdc_tx_semaphore == NULL) {
        usbd.cdc_tx_semaphore = furi_semaphore_alloc(1, 1);
    }

    usbd.usb_dev = dev;

    usbd_reg_config(dev, composite_ep_config);
    usbd_reg_control(dev, composite_ctrlreq_handler);

    usbd_connect(dev, true);
}

static void composite_deinit(usbd_device* dev) {
    usbd_reg_config(dev, NULL);
    usbd_reg_control(dev, NULL);

    furi_semaphore_free(usbd.hid_sensor_semaphore);
    furi_semaphore_free(usbd.cdc_tx_semaphore);
}

static void composite_on_wakeup(usbd_device* dev) {
    UNUSED(dev);
    usbd.usb_connected = true;

    if(usbd.cdc_callbacks != NULL) {
        if(usbd.cdc_callbacks->state_callback != NULL)
            usbd.cdc_callbacks->state_callback(usbd.cdc_cb_ctx, 1);
    }
}

static void composite_on_suspend(usbd_device* dev) {
    UNUSED(dev);
    if(usbd.usb_connected) {
        usbd.usb_connected = false;
        furi_semaphore_release(usbd.hid_sensor_semaphore);
        furi_semaphore_release(usbd.cdc_tx_semaphore);
    }

    if(usbd.cdc_callbacks != NULL) {
        if(usbd.cdc_callbacks->state_callback != NULL)
            usbd.cdc_callbacks->state_callback(usbd.cdc_cb_ctx, 0);
    }
}

static void usb_rx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);

    if(ep == CDC_EP_OUT) {
        if(usbd.cdc_callbacks != NULL) {
            if(usbd.cdc_callbacks->rx_ep_callback != NULL) {
                usbd.cdc_callbacks->rx_ep_callback(usbd.cdc_cb_ctx);
            }
        }
    }

    if(ep == HID_EP_OUT) {
        if(usbd.hid_callbacks != NULL) {
            if(usbd.hid_callbacks->rx_ep_callback != NULL) {
                usbd.hid_callbacks->rx_ep_callback(usbd.hid_cb_ctx);
            }
        }
    }
}

static void usb_tx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);

    if(ep == HID_EP_IN) {
        furi_semaphore_release(usbd.hid_sensor_semaphore);

        if(usbd.hid_callbacks != NULL) {
            if(usbd.hid_callbacks->tx_ep_callback != NULL) {
                usbd.hid_callbacks->tx_ep_callback(usbd.hid_cb_ctx);
            }
        }
    }

    if(ep == CDC_EP_IN || ep == CDC_EP_NTF) {
        furi_semaphore_release(usbd.cdc_tx_semaphore);

        if(usbd.cdc_callbacks != NULL) {
            if(usbd.cdc_callbacks->tx_ep_callback != NULL) {
                usbd.cdc_callbacks->tx_ep_callback(usbd.cdc_cb_ctx);
            }
        }
    }
}

static void usb_txrx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    switch(event) {
    case usbd_evt_eptx:
        usb_tx_ep_callback(dev, event, ep);
        break;
    case usbd_evt_eprx:
        usb_rx_ep_callback(dev, event, ep);
        break;
    default:
        break;
    }
}

static void hid_sensor_send_response(uint8_t* data, uint8_t len) {
    if((usbd.hid_sensor_semaphore == NULL) || !usbd.usb_connected) {
        return;
    }

    FuriStatus s = furi_semaphore_acquire(usbd.hid_sensor_semaphore, SENSOR_POLL_PERIOD * 2);
    if(s == FuriStatusErrorTimeout) {
        return;
    }
    furi_check(s == FuriStatusOk);

    if(usbd.usb_connected) {
        usbd_ep_write(usbd.usb_dev, HID_EP_IN, data, len);
    }
}

/* Configure endpoints */
static usbd_respond composite_ep_config(usbd_device* dev, uint8_t cfg) {
    switch(cfg) {
    case 0:
        /* deconfiguring device */
        usbd_ep_deconfig(dev, HID_EP_IN);
        usbd_ep_deconfig(dev, HID_EP_OUT);
        usbd_reg_endpoint(dev, HID_EP_IN, 0);
        usbd_reg_endpoint(dev, HID_EP_OUT, 0);

        usbd_ep_deconfig(dev, CDC_EP_NTF);
        usbd_ep_deconfig(dev, CDC_EP_IN);
        usbd_ep_deconfig(dev, CDC_EP_OUT);
        usbd_reg_endpoint(dev, CDC_EP_IN, 0);
        usbd_reg_endpoint(dev, CDC_EP_OUT, 0);

        return usbd_ack;
    case 1:
        /* configuring device */
        usbd_ep_config(dev, HID_EP_IN, USB_EPTYPE_INTERRUPT, HID_EP_PACKET_SIZE);
        usbd_ep_config(dev, HID_EP_OUT, USB_EPTYPE_INTERRUPT, HID_EP_PACKET_SIZE);
        usbd_reg_endpoint(dev, HID_EP_IN, usb_txrx_ep_callback);
        usbd_reg_endpoint(dev, HID_EP_OUT, usb_txrx_ep_callback);
        usbd_ep_write(dev, HID_EP_IN, 0, 0);

        usbd_ep_config(dev, CDC_EP_IN, USB_EPTYPE_BULK, CDC_EP_PACKET_SIZE);
        usbd_ep_config(dev, CDC_EP_OUT, USB_EPTYPE_BULK, CDC_EP_PACKET_SIZE);
        usbd_ep_config(dev, CDC_EP_NTF, USB_EPTYPE_INTERRUPT, CDC_EP_NTF_PACKETSIZE);
        usbd_reg_endpoint(dev, CDC_EP_IN, usb_txrx_ep_callback);
        usbd_reg_endpoint(dev, CDC_EP_OUT, usb_txrx_ep_callback);
        usbd_ep_write(dev, CDC_EP_IN, 0, 0);

        return usbd_ack;
    default:
        return usbd_fail;
    }
}

/* Control requests handler */
static usbd_respond
    composite_ctrlreq_handler(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback) {
    UNUSED(callback);

    /* CDC */
    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
           (USB_REQ_INTERFACE | USB_REQ_CLASS) &&
       req->wIndex == 1) {
        switch(req->bRequest) {
        case USB_CDC_SET_CONTROL_LINE_STATE:
            usbd.cdc_ctrl_line_state = req->wValue;
            if(usbd.cdc_callbacks != NULL) {
                if(usbd.cdc_callbacks->ctrl_line_callback != NULL) {
                    usbd.cdc_callbacks->ctrl_line_callback(usbd.cdc_cb_ctx, req->wValue);
                }
            }

            return usbd_ack;

        case USB_CDC_SET_LINE_CODING:
            memcpy(&usbd.cdc_config, req->data, sizeof(usbd.cdc_config));
            if(usbd.cdc_callbacks != NULL) {
                if(usbd.cdc_callbacks->config_callback != NULL) {
                    usbd.cdc_callbacks->config_callback(usbd.cdc_cb_ctx, &usbd.cdc_config);
                }
            }
            return usbd_ack;

        case USB_CDC_GET_LINE_CODING:
            dev->status.data_ptr = &usbd.cdc_config;
            dev->status.data_count = sizeof(usbd.cdc_config);

            return usbd_ack;

        default:
            furi_crash("Unimplemented usb cdc req");
            return usbd_fail;
        }
    }

    /* HID */
    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
           (USB_REQ_INTERFACE | USB_REQ_CLASS) &&
       req->wIndex == 0) {
        switch(req->bRequest) {
        case USB_HID_SETIDLE:
            return usbd_ack;

        case USB_HID_GETREPORT:
            switch(HIGH8_WORD(req->wValue)) {
            case USB_HID_REPORT_FEATURE:
                // feature 0, report id is ignored
                dev->status.data_ptr = (uint8_t*)&(usbd.sensor_feature_report);
                dev->status.data_count = sizeof(usbd.sensor_feature_report);
                break;

            default:
                FURI_LOG_D(LOG_TAG, "Unimplemented usb hid GET_REPORT");
                break;
            }
            return usbd_ack;

        case USB_HID_SETREPORT:
            switch(HIGH8_WORD(req->wValue)) {
            case USB_HID_REPORT_FEATURE:
                // feature 0, report id is ignored
                memcpy(
                    &usbd.sensor_feature_report,
                    (struct TempSensorFeature*)req->data,
                    MIN(req->wLength, sizeof(usbd.sensor_feature_report)));
                break;

            default:
                FURI_LOG_D(LOG_TAG, "Unimplemented usb hid SET_REPORT");
                break;
            }
            return usbd_ack;

        default:
            furi_crash("Unimplemented usb hid req");
            return usbd_fail;
        }
    }

    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
           (USB_REQ_INTERFACE | USB_REQ_STANDARD) &&
       req->wIndex == 0 && req->bRequest == USB_STD_GET_DESCRIPTOR) {
        switch(HIGH8_WORD(req->wValue)) {
        case USB_DTYPE_HID:
            dev->status.data_ptr = (uint8_t*)&(composite_cfg_desc.iad_0.hid_desc);
            dev->status.data_count = sizeof(composite_cfg_desc.iad_0.hid_desc);

            return usbd_ack;

        case USB_DTYPE_HID_REPORT:
            dev->status.data_ptr = (uint8_t*)hid_sensor_report_desc;
            dev->status.data_count = sizeof(hid_sensor_report_desc);

            return usbd_ack;

        default:
            return usbd_fail;
        }
    }

    FURI_LOG_E(LOG_TAG, "Unimplemented usb hid feature");
    return usbd_fail;
}

FuriHalUsbInterface hid_with_cdc_intf = {
    .init = composite_init,
    .deinit = composite_deinit,
    .wakeup = composite_on_wakeup,
    .suspend = composite_on_suspend,

    .dev_descr = (struct usb_device_descriptor*)&hid_sensor_device_desc,

    .str_manuf_descr = (void*)&dev_manuf_desc,
    .str_prod_descr = (void*)&dev_prod_desc,
    .str_serial_descr = (void*)&dev_serial_desc,

    .cfg_descr = (void*)&composite_cfg_desc,
};

void composite_hid_send_temp_report(int16_t temp_raw) {
    tempature_report.temperature = temp_raw;
    tempature_report.event = SENSOR_EVENT_DATA_UPDATED;
    tempature_report.state = SENSOR_STATE_READY;

    hid_sensor_send_response((uint8_t*)&tempature_report, sizeof(tempature_report));
}

void composite_hid_send_temp() {
    FuriHalAdcHandle* temp_adc = furi_hal_adc_acquire();
    furi_hal_adc_configure(temp_adc);

    float temp_raw = furi_hal_adc_convert_temp(
        temp_adc, furi_hal_adc_read(temp_adc, FuriHalAdcChannelTEMPSENSOR));
    furi_hal_adc_release(temp_adc);

    FURI_LOG_I(LOG_TAG, "temperature: %.2f", (double)temp_raw);

    temp_raw = temp_raw * 100;

    composite_hid_send_temp_report((int16_t)temp_raw);
}

void composite_cdc_set_callbacks(CdcCallbacks* cb, void* context) {
    if(usbd.cdc_callbacks != NULL) {
        if(usbd.cdc_callbacks->state_callback != NULL) {
            if(usbd.usb_connected == true) usbd.cdc_callbacks->state_callback(usbd.cdc_cb_ctx, 0);
        }
    }

    usbd.cdc_callbacks = cb;
    usbd.cdc_cb_ctx = context;

    if(usbd.cdc_callbacks != NULL) {
        if(usbd.cdc_callbacks->state_callback != NULL) {
            if(usbd.usb_connected == true) usbd.cdc_callbacks->state_callback(usbd.cdc_cb_ctx, 1);
        }
        if(usbd.cdc_callbacks->ctrl_line_callback != NULL) {
            usbd.cdc_callbacks->ctrl_line_callback(usbd.cdc_cb_ctx, usbd.cdc_ctrl_line_state);
        }
    }
}

void composite_hid_set_callbacks(HidCallbacks* cb, void* context) {
    if(usbd.hid_callbacks != NULL) {
        usbd.hid_callbacks = cb;
        usbd.hid_callbacks = context;
    }
}

int32_t composite_cdc_receive(uint8_t* buf, uint16_t sz) {
    int32_t len = 0;
    len = usbd_ep_read(usbd.usb_dev, CDC_EP_OUT, buf, sz);

    return ((len < 0) ? 0 : len);
}

void composite_cdc_send(const uint8_t* buf, uint16_t len) {
    if((usbd.cdc_tx_semaphore == NULL) || !usbd.usb_connected) {
        return;
    }

    FuriStatus s = furi_semaphore_acquire(usbd.cdc_tx_semaphore, SENSOR_POLL_PERIOD * 2);
    if(s == FuriStatusErrorTimeout) {
        return;
    }
    furi_check(s == FuriStatusOk);

    if(usbd.usb_connected) {
        usbd_ep_write(usbd.usb_dev, CDC_EP_IN, buf, len);
    }
}

bool composite_is_connected(void) {
    return usbd.usb_connected;
}

FuriStatus composite_connect() {
    if(furi_hal_usb_is_locked()) {
        FURI_LOG_E(LOG_TAG, "usb is locked by other threads");
        return FuriStatusError;
    }

    usbd.prev_intf = furi_hal_usb_get_config();
    furi_hal_usb_unlock();
    furi_hal_usb_set_config(&hid_with_cdc_intf, NULL);

    // print logs to usb cdc
    furi_log_add_handler(log_handler);

    // connect to cli
    // FIXME: NOT WORKING SOMEHOW
    cli_handle = furi_record_open(RECORD_CLI);
    cli_session_open(cli_handle, &my_cli_vcp);
    cli_session_close(cli_handle);

    return FuriStatusOk;
}

FuriStatus composite_disconnect() {
    // kill cli session
    cli_handle = furi_record_open(RECORD_CLI);
    cli_session_close(cli_handle);
    cli_session_open(cli_handle, &cli_vcp); // restore cli vcp
    furi_record_close(RECORD_CLI);

    // kill logging redirection
    furi_log_remove_handler(log_handler);

    furi_hal_usb_unlock();
    furi_hal_usb_set_config(usbd.prev_intf, NULL);

    return FuriStatusOk;
}

static void log_callback(const uint8_t* data, size_t size, void* context) {
    UNUSED(context);

    composite_cdc_send(data, size);
}
