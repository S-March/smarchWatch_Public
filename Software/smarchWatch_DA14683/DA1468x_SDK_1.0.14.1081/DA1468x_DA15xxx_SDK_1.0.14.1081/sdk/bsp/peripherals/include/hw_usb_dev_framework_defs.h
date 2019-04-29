/**
 ****************************************************************************************
 *
 * @file hw_usb_dev_framework_defs.h
 *
 * @brief Header file with USB Device Framework definitions.
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef HW_USB_DEVICE_FRAMEWORK_DEFS_H
#define HW_USB_DEVICE_FRAMEWORK_DEFS_H

/*-------------------------------------- Include files -------------------------------------------*/

#include <sdk_defs.h>

/*------------------------- Internal macro definitions not to be used directly -------------------*/

#define __HW_USB_DEVICE_FRAMEWORK_CONCAT2(t1, t2)       HW_USB_DEVICE_FRAMEWORK ## _ ## t1 ## _ ## t2
#define __HW_USB_DEVICE_FRAMEWORK_CONCAT3(t1, t2, t3)   HW_USB_DEVICE_FRAMEWORK ## _ ## t1 ## _ ## t2 ## _ ## t3

#define __HW_USB_DEVICE_FRAMEWORK_FUNC(_1, _2, _3,      \
              __HW_USB_DEVICE_FRAMEWORK_MACRO, ...)     __HW_USB_DEVICE_FRAMEWORK_MACRO

#define __HW_USB_DEVICE_FRAMEWORK_CONCAT(...)           __HW_USB_DEVICE_FRAMEWORK_FUNC(__VA_ARGS__,\
                                                        __HW_USB_DEVICE_FRAMEWORK_CONCAT3,       \
                                                        __HW_USB_DEVICE_FRAMEWORK_CONCAT2)(__VA_ARGS__)

/*----------------------------------- Local macro definitions  -----------------------------------*/


/* Create a USB enumeration entry using specific tokens
 * Supports 2 or 3 tokens.
 * e.g
 * HW_USB_DEVICE_FRAMEWORK_ENUM(0x00, DIR, OUT) expands to
 * HW_USB_DEVICE_FRAMEWORK_DIR_OUT = 0x00,
 *
 */
#define HW_USB_DEVICE_FRAMEWORK_ENUM(value, ...)  __HW_USB_DEVICE_FRAMEWORK_CONCAT(__VA_ARGS__) = value,


#pragma pack(push,1)

/*----------------------------------- Enumerations and definitions -------------------------------*/


/**
 * \brief Direction
 */
enum {
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x00, DIR, OUT)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x80, DIR, IN)
};

/**
 * \brief Types
 */
enum {
        HW_USB_DEVICE_FRAMEWORK_ENUM((0x03 << 5), TYPE, MASK)
        HW_USB_DEVICE_FRAMEWORK_ENUM((0x00 << 5), TYPE, STANDARD)
        HW_USB_DEVICE_FRAMEWORK_ENUM((0x01 << 5), TYPE, CLASS)
        HW_USB_DEVICE_FRAMEWORK_ENUM((0x02 << 5), TYPE, VENDOR)
        HW_USB_DEVICE_FRAMEWORK_ENUM((0x03 << 5), TYPE, RESERVED)
};

/**
 * \brief Recipients
 */
enum {
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x1f, RECIP, MASK)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x00, RECIP, DEVICE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x01, RECIP, INTERFACE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x02, RECIP, ENDPOINT)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x03, RECIP, OTHER)
};

/**
 * \brief Standard requests
 */
enum {
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x00, REQ, GET_STATUS)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x01, REQ, CLEAR_FEATURE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x03, REQ, SET_FEATURE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x05, REQ, SET_ADDRESS)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x06, REQ, GET_DESCRIPTOR)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x07, REQ, SET_DESCRIPTOR)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x08, REQ, GET_CONFIGURATION)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x09, REQ, SET_CONFIGURATION)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x0a, REQ, GET_INTERFACE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x0b, REQ, SET_INTERFACE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x0c, REQ, SYNCH_FRAME)
};

/**
 * \brief Feature flags
 */
enum {
        HW_USB_DEVICE_FRAMEWORK_ENUM(0, DEVICE, SELF_POWERED)
        HW_USB_DEVICE_FRAMEWORK_ENUM(1, DEVICE, REMOTE_WAKEUP)
        HW_USB_DEVICE_FRAMEWORK_ENUM(2, DEVICE, TEST_MODE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(3, DEVICE, B_HNP_ENABLE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(4, DEVICE, A_HNP_SUPPORT)
        HW_USB_DEVICE_FRAMEWORK_ENUM(5, DEVICE, A_ALT_HNP_SUPPORT)
        HW_USB_DEVICE_FRAMEWORK_ENUM(6, DEVICE, DEBUG_MODE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0, ENDPOINT, HALT)
};

/**
 * \brief Descriptor types
 */
enum {
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x01, DT, DEVICE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x02, DT, CONFIG)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x03, DT, STRING)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x04, DT, INTERFACE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x05, DT, ENDPOINT)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x06, DT, DEVICE_QUALIFIER)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x07, DT, OTHER_SPEED_CONFIG)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x08, DT, INTERFACE_POWER)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x09, DT, OTG)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x0a, DT, DEBUG)

        /* Class specific types */
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x0b, DT, INTERFACE_ASSOCIATION)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x21, DT, CS, DEVICE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x22, DT, CS, CONFIG)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x23, DT, CS, STRING)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x24, DT, CS, INTERFACE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x25, DT, CS, ENDPOINT)
};

/**
 * \brief Class codes
 */
enum {
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x00, CLASS, INTERFACE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x01, CLASS, AUDIO)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x02, CLASS, COMM)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x03, CLASS, HID)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x05, CLASS, PHYSICAL)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x06, CLASS, STILL_IMAGE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x07, CLASS, PRINTER)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x08, CLASS, MASS_STORAGE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x09, CLASS, HUB)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x0a, CLASS, CDC_DATA)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x0b, CLASS, CSCID)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x0d, CLASS, CONTENT_SEC)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x0e, CLASS, VIDEO)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0xfe, CLASS, APP_SPE)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0xff, CLASS, VENDOR_SPEC)
};

/**
 * \brief Config descriptor attributes
 *
 */
enum {
        HW_USB_DEVICE_FRAMEWORK_ENUM((1 << 7), CONFIG, ATT, ONE)
        HW_USB_DEVICE_FRAMEWORK_ENUM((1 << 6), CONFIG, ATT, SELFPOWER)
        HW_USB_DEVICE_FRAMEWORK_ENUM((1 << 5), CONFIG, ATT, REMOTE_WAKEUP)
};

/**
 * \brief Endpoints
 */
enum {
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x0f, ENDPOINT, NUMBER_MASK)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x80, ENDPOINT, DIR_MASK)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x03, ENDPOINT, XFERTYPE_MASK)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x00, ENDPOINT, XFER_CONTROL)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x01, ENDPOINT, XFER_ISOC)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x02, ENDPOINT, XFER_BULK)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x03, ENDPOINT, XFER_INT)


        HW_USB_DEVICE_FRAMEWORK_ENUM(0x00, ENDPOINT, CONTROL)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x01, ENDPOINT, ISO)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x02, ENDPOINT, BULK)
        HW_USB_DEVICE_FRAMEWORK_ENUM(0x03, ENDPOINT, INTERRUPT)

        HW_USB_DEVICE_FRAMEWORK_ENUM((0x00 << 2), ENDPOINT, NO_SYNC)
        HW_USB_DEVICE_FRAMEWORK_ENUM((0x01 << 2), ENDPOINT, ASYNC)
        HW_USB_DEVICE_FRAMEWORK_ENUM((0x02 << 2), ENDPOINT, ADAPTIVE)
        HW_USB_DEVICE_FRAMEWORK_ENUM((0x03 << 2), ENDPOINT, SYNC)

        HW_USB_DEVICE_FRAMEWORK_ENUM((0x00 << 4), ENDPOINT, DATAEND)
        HW_USB_DEVICE_FRAMEWORK_ENUM((0x01 << 4), ENDPOINT, FEEDBACK)
        HW_USB_DEVICE_FRAMEWORK_ENUM((0x02 << 4), ENDPOINT, IMP_FEEDB)
        HW_USB_DEVICE_FRAMEWORK_ENUM((0x03 << 4), ENDPOINT, RESERVED)
};

/**
 * \brief USB device state type.
 */
typedef enum {
        /* NOTATTACHED isn't in the USB spec, and this state acts
         * the same as ATTACHED ... but it's clearer this way.
         */
        USB_STATE_NOTATTACHED = 0,

        /* the chapter 9 device states */
        USB_STATE_ATTACHED,
        USB_STATE_POWERED,
        USB_STATE_DEFAULT,              /* limited function */
        USB_STATE_ADDRESS,
        USB_STATE_CONFIGURED,           /* most functions */

        USB_STATE_SUSPENDED

        /* NOTE:  there are actually four different SUSPENDED
         * states, returning to POWERED, DEFAULT, ADDRESS, or
         * CONFIGURED respectively when SOF tokens flow again.
         */
} HW_USB_DEV_FRAMEWORK_STATE;

/*
 * \brief Control request
 *
 */
typedef struct {
        uint8_t request_type;
        uint8_t request;
        uint16_t value;
        uint16_t index;
        uint16_t length;
} hw_usb_device_framework_ctrl_req_t;

/**
 *  \brief Descriptor header
 */
typedef struct {
        uint8_t  length;
        uint8_t  descriptor_type;
} hw_usb_device_framework_desc_header_t;

/**
 * \brief Device descriptor
 */
typedef struct {
        hw_usb_device_framework_desc_header_t header;
        uint16_t cd_usb;
        uint8_t  device_class;
        uint8_t  device_sub_class;
        uint8_t  device_protocol;
        uint8_t  max_packet_size0;
        uint16_t id_vendor;
        uint16_t id_product;
        uint16_t cd_device;
        uint8_t  manufacturer;
        uint8_t  product;
        uint8_t  serial_number;
        uint8_t  num_configurations;
} hw_usb_device_framework_dev_descriptor_t;

/**
 * \brief Configuration descriptor
 */
typedef struct {
        hw_usb_device_framework_desc_header_t header;
        uint16_t total_length;
        uint8_t  num_interfaces;
        uint8_t  configuration_value;
        uint8_t  configuration;
        uint8_t  attributes;
        uint8_t  max_power;
} hw_usb_device_framework_conf_descriptor_t;

/**
 * \brief Interface descriptor
 */
typedef struct {
        hw_usb_device_framework_desc_header_t header;
        uint8_t  interface_number;
        uint8_t  alternate_setting;
        uint8_t  num_endpoints;
        uint8_t  interface_class;
        uint8_t  interface_subclass;
        uint8_t  interface_protocol;
        uint8_t  interface;
} hw_usb_device_framework_if_descriptor_t;

/**
 * \brief Endpoint descriptor
 */
typedef struct {
        hw_usb_device_framework_desc_header_t header;
        uint8_t  endpoint_address;
        uint8_t  attributes;
        uint16_t max_packet_size;
        uint8_t  interval;
} hw_usb_device_framework_ep_descriptor_t;


#pragma pack(pop)

#endif /* HW_USB_DEVICE_FRAMEWORK_DEFS_H  */

