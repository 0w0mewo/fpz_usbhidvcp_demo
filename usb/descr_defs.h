#ifndef DESCR_DEFS_H_H
#define DESCR_DEFS_H_H

#include "usb.h"
#include "usb_hid.h"
#include "usb_cdc.h"
#include "config.h"

struct HidIadDescriptor {
#if WITH_VCP == 0
    struct usb_iad_descriptor hid_iad;
#endif
    struct usb_interface_descriptor hid;
    struct usb_hid_descriptor hid_desc;
    struct usb_endpoint_descriptor hid_ep_in;
    struct usb_endpoint_descriptor hid_ep_out;
};

#if WITH_VCP == 1
struct CdcIadDescriptor {
    struct usb_iad_descriptor comm_iad;
    struct usb_interface_descriptor comm;
    struct usb_cdc_header_desc cdc_hdr;
    struct usb_cdc_call_mgmt_desc cdc_mgmt;
    struct usb_cdc_acm_desc cdc_acm;
    struct usb_cdc_union_desc cdc_union;
    struct usb_endpoint_descriptor comm_ep;
    struct usb_interface_descriptor data;
    struct usb_endpoint_descriptor data_eprx;
    struct usb_endpoint_descriptor data_eptx;
};
#endif

struct CompositeConfigDescriptor {
    struct usb_config_descriptor config;
    struct HidIadDescriptor iad_0;
#if WITH_VCP == 1
    struct CdcIadDescriptor iad_1;
#endif
} FURI_PACKED;
#endif