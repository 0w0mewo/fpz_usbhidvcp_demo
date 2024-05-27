#ifndef __COMPOSITE_H_H
#define __COMPOSITE_H_H

#include <furi_hal_usb.h>
#include <furi_hal_usb_i.h>
#include <furi_hal_usb_cdc.h>
#include <furi.h>
#include <furi_hal.h>
#include "usb_cdc.h"
#include "usb_hid.h"
#include "descr_defs.h"
#include "hid_sensor_spec.h"

#define USBD_VID 0x0483
#define USBD_PID 0x5741

#define CDC_EP_OUT 0x04
#define CDC_EP_IN 0x85
#define CDC_EP_NTF 0x86
#define CDC_EP_NTF_PACKETSIZE 0x08
#define CDC_EP_PACKET_SIZE 64

#define HID_EP_PACKET_SIZE 16
#define HID_EP_IN 0x82
#define HID_EP_OUT 0x01
#define HID_INTERVAL 32

#define SENSOR_STATE_UNKNOWN 0x00
#define SENSOR_STATE_READY 0x01
#define SENSOR_STATE_NOT_AVAILABLE 0x02
#define SENSOR_STATE_NO_DATA 0x03
#define SENSOR_STATE_INITIALIZING 0x04
#define SENSOR_STATE_ACCESS_DENIED 0x05
#define SENSOR_STATE_ERROR 0x06

#define SENSOR_EVENT_UNKNOWN 0x00
#define SENSOR_EVENT_STATE_CHANGED 0x01
#define SENSOR_EVENT_DATA_UPDATED 0x03

#define SENSOR_POLL_PERIOD 200
#define SENSOR_TEMPC_MAX 150
#define SENSOR_TEMPC_MIN (-50)

#define LOG_TAG "hellousb"

#define HIGH8_WORD(x) ((x >> 8) & 0xff)

typedef enum { IntfTypeHIDsensor, IntfTypeCDC } IntfType;

typedef void (*CompositeRxCallback)(IntfType which, void* context);

struct TempSensorFeature {
    uint16_t min_interval;
    uint16_t interval;
    int16_t max_temp;
    int16_t min_temp;
} FURI_PACKED;

struct HidSensorTempReport {
    uint8_t state;
    uint8_t event;
    int16_t temperature;
} FURI_PACKED;

typedef struct {
    void (*tx_ep_callback)(void* context);
    void (*rx_ep_callback)(void* context);
} HidCallbacks;

struct CompositeUsbDevice {
    usbd_device* usb_dev;
    FuriHalUsbInterface* prev_intf;
    bool usb_connected;

    // cdc stuffs
    struct usb_cdc_line_coding cdc_config;
    uint8_t cdc_ctrl_line_state;
    CdcCallbacks* cdc_callbacks;
    void* cdc_cb_ctx;

    // hid sensor stuffs
    struct TempSensorFeature sensor_feature_report;
    HidCallbacks* hid_callbacks;
    void* hid_cb_ctx;

    // tx semaphores
    FuriSemaphore* hid_sensor_semaphore;
    FuriSemaphore* cdc_tx_semaphore;

    // CompositeRxCallback rx_cb;
};

FuriStatus composite_connect();
FuriStatus composite_disconnect();
bool composite_is_connected(void);
void composite_hid_send_temp_report(int16_t temp_raw);
void composite_hid_send_temp();
void composite_hid_set_callbacks(HidCallbacks* cb, void* context);
void composite_cdc_send(const uint8_t* data, uint16_t sz);
void composite_cdc_set_callbacks(CdcCallbacks* cb, void* context);
int32_t composite_cdc_receive(uint8_t* buf, uint16_t len);

extern FuriHalUsbInterface hid_with_cdc_intf;
;
#endif