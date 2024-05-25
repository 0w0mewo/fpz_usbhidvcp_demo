#ifndef __COMPOSITE_H_H
#define __COMPOSITE_H_H

#include <furi_hal_usb.h>
#include <furi_hal_usb_i.h>
#include <furi.h>
#include "usb_cdc.h"
#include "usb_hid.h"
#include "descr_defs.h"
#include "hid_sensor_spec.h"

#define USBD_VID 0x0483
#define USBD_PID 0x5741

#define CDC_EP_RXD 0x04
#define CDC_EP_TXD 0x85
#define CDC_EP_NTF 0x86
#define CDC_EP_NTF_PACKETSIZE 0x08
#define CDC_EP_PACKET_SIZE 64

#define HID_EP_PACKET_SIZE 16
#define HID_EP_IN 0x82
#define HID_EP_OUT 0x01
#define HID_INTERVAL 5

#define HID_FEATURE_REPORT_TYPE 0x0300

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

struct HidSensorTempReport {
    uint8_t state;
    uint8_t event;
    int16_t temperature;
} FURI_PACKED;

extern FuriHalUsbInterface hid_with_cdc_intf;

bool usbdev_is_connected(void);
void hid_send_temp_report(int16_t temp_raw);
void cdc_send(const uint8_t* data, uint16_t sz);

#endif