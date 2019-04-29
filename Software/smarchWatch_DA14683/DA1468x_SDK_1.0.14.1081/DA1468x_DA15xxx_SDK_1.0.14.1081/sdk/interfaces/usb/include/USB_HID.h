/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2016     SEGGER Microcontroller GmbH & Co. KG     *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       emUSB-Device * USB Device stack for embedded applications    *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product for in-house use.         *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       emUSB-Device version: V3.02c                                 *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
Licensing information

Licensor:                 SEGGER Software GmbH
Licensed to:              Dialog Semiconductor BV, Het Zuiderkruis 53, 5215 MV S-Hertogenbosch, The Netherlands
Licensed SEGGER software: emUSB-Device
License number:           USBD-00327
License model:            Buyout SRC [Buyout Source Code License], signed on 8th August, 2016
Licensed product:         Any
Licensed platform:        D2320
Licensed number of seats: -
----------------------------------------------------------------------
File    : USB_HID.h
Purpose : Public header of the human interface device class
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef USB_HID_H          /* Avoid multiple inclusion */
#define USB_HID_H

#include "USB_SEGGER.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Config defaults
*
**********************************************************************
*/
typedef int USB_HID_HANDLE;


/*********************************************************************
*
*       Const values
*
**********************************************************************
*/
#define USB_HID_USB_CLASS     3         // Human interface device class
#define USB_HID_USB_SUBCLASS  0x00      //
#define USB_HID_USB_PROTOCOL  0x00      //



// Section 4.2
#define USB_HID_NO_SUBCLASS                0
#define USB_HID_BOOT_INTERFACE_SUBCLASS    1

// Section 7.2.5
#define USB_HID_BOOT_PROTOCOL              0
#define USB_HID_REPORT_PROTOCOL            1

// HID report types
#define USB_HID_INPUT_REPORT                1
#define USB_HID_OUTPUT_REPORT               2
#define USB_HID_FEATURE_REPORT              3

// Report descriptor
// Section 6.2.2.4 - Main Items
#define USB_HID_MAIN_INPUT                   0x80
#define USB_HID_MAIN_OUTPUT                  0x90
#define USB_HID_MAIN_FEATURE                 0xB0
#define USB_HID_MAIN_COLLECTION              0xA0
#define USB_HID_MAIN_ENDCOLLECTION           0xC0

// Section 6.2.2.5 - Input, Output, and Features Items
#define USB_HID_DATA                     (0 << 0)
#define USB_HID_CONSTANT                 (1 << 0)
#define USB_HID_ARRAY                    (0 << 1)
#define USB_HID_VARIABLE                 (1 << 1)
#define USB_HID_ABSOLUTE                 (0 << 2)
#define USB_HID_RELATIVE                 (1 << 2)
#define USB_HID_NOWRAP                   (0 << 3)
#define USB_HID_WRAP                     (1 << 3)
#define USB_HID_LINEAR                   (0 << 4)
#define USB_HID_NONLINEAR                (1 << 4)
#define USB_HID_PREFERREDSTATE           (0 << 5)
#define USB_HID_NOPREFERRED              (1 << 5)
#define USB_HID_NONULLPOSITION           (0 << 6)
#define USB_HID_NULLSTATE                (1 << 6)
#define USB_HID_NONVOLATILE              (0 << 7)
#define USB_HID_VOLATILE                 (1 << 7)
#define USB_HID_BITFIELD                 (0 << 8)
#define USB_HID_BUFFEREDBYTES            (1 << 8)

// Section 6.2.2.6 - Collection, End Collection Items
#define USB_HID_COLLECTION_PHYSICAL          0x00
#define USB_HID_COLLECTION_APPLICATION       0x01
#define USB_HID_COLLECTION_LOGICAL           0x02
#define USB_HID_COLLECTION_REPORT            0x03
#define USB_HID_COLLECTION_NAMEDARRAY        0x04
/*lint -esym(621, USB_HID_COLLECTION_USB_HID_USAGESWITCH, USB_HID_COLLECTION_USB_HID_USAGEMODIFIER) MISRA C:2012 Rule 5.1, required */
#define USB_HID_COLLECTION_USB_HID_USAGESWITCH   0x05
#define USB_HID_COLLECTION_USB_HID_USAGEMODIFIER 0x06

//  Section 6.2.2.7
#define USB_HID_GLOBAL_USAGE_PAGE            0x04
#define USB_HID_GLOBAL_LOGICAL_MINIMUM       0x14
#define USB_HID_GLOBAL_LOGICAL_MAXIMUM       0x24
#define USB_HID_GLOBAL_PHYSICAL_MINIMUM      0x34
#define USB_HID_GLOBAL_PHYSICAL_MAXIMUM      0x44
#define USB_HID_GLOBAL_UNIT_EXPONENT         0x54
#define USB_HID_GLOBAL_UNIT                  0x64
#define USB_HID_GLOBAL_REPORT_SIZE           0x74
#define USB_HID_GLOBAL_REPORT_ID             0x84
#define USB_HID_GLOBAL_REPORT_COUNT          0x94
#define USB_HID_GLOBAL_PUSH                  0xA4
#define USB_HID_GLOBAL_POP                   0xB4

// Section 6.2.2.8
#define USB_HID_LOCAL_USAGE                  0x08
#define USB_HID_LOCAL_USAGE_MINIMUM          0x18
#define USB_HID_LOCAL_USAGE_MAXIMUM          0x28
#define USB_HID_LOCAL_DESIGNATOR_INDEX       0x38
#define USB_HID_LOCAL_DESIGNATOR_MINIMUM     0x48
#define USB_HID_LOCAL_DESIGNATOR_MAXIMUM     0x58
#define USB_HID_LOCAL_STRING_INDEX           0x78
#define USB_HID_LOCAL_STRING_MINIMUM         0x88
#define USB_HID_LOCAL_STRING_MAXIMUM         0x98
#define USB_HID_LOCAL_DELIMITER              0xA8

//   Usage pages
//   HuT1_12.pdf - Section 3 - Table 1

#define USB_HID_USAGE_PAGE_UNDEFINED            0x00
#define USB_HID_USAGE_PAGE_GENERIC_DESKTOP      0x01
#define USB_HID_USAGE_PAGE_SIMULATION           0x02
#define USB_HID_USAGE_PAGE_VR                   0x03
#define USB_HID_USAGE_PAGE_SPORT                0x04
#define USB_HID_USAGE_PAGE_GAME                 0x05
#define USB_HID_USAGE_PAGE_GENERIC_DEVICE       0x06
#define USB_HID_USAGE_PAGE_KEYBOARD_KEYPAD      0x07
#define USB_HID_USAGE_PAGE_LEDS                 0x08
#define USB_HID_USAGE_PAGE_BUTTON               0x09
#define USB_HID_USAGE_PAGE_ORDINAL              0x0A
#define USB_HID_USAGE_PAGE_TELEPHONY            0x0B
#define USB_HID_USAGE_PAGE_CONSUMER             0x0C
#define USB_HID_USAGE_PAGE_DIGITIZER            0x0D
#define USB_HID_USAGE_PAGE_PID                  0x0F
#define USB_HID_USAGE_PAGE_UNICODE              0x10

//! \brief  Scale
#define USB_HID_USAGE_PAGE_SCALE_PAGE           0x8D

//! \brief  Magnetic stripe reading devices
#define USB_HID_USAGE_PAGE_MSR                  0x8E

//! \brief  USB Device Class Definition for Image Class Devices
#define USB_HID_USAGE_PAGE_CAMERA_CONTROL       0x90

//! \brief  OAAF Definitions for arcade and coinop related devices
#define USB_HID_USAGE_PAGE_ARCADE               0x91
//! @}

//! \name   Generic Desktop Usages
//! \see    HuT1_12.pdf - Section 4 - Table 6
//! @{

//! \brief  Pointer
#define USB_HID_USAGE_POINTER                   0x01

//! \brief  Mouse
#define USB_HID_USAGE_MOUSE                     0x02

//! \brief  Joystick
#define USB_HID_USAGE_JOYSTICK                  0x04

//! \brief  Game pad
#define USB_HID_USAGE_GAMEPAD                   0x05

//! \brief  Keyboard
#define USB_HID_USAGE_KEYBOARD                  0x06

//! \brief  Keypad
#define USB_HID_USAGE_KEYPAD                    0x07

//! \brief  Multi-axis controller
#define USB_HID_USAGE_MULTIAXIS                 0x08

//! \brief  X axis
#define USB_HID_USAGE_X                         0x30

//! \brief  Y axis
#define USB_HID_USAGE_Y                         0x31

//! \brief  Wheel
#define USB_HID_USAGE_WHEEL                     0x38

//! @}


/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  U8          EPIn;
  U8          EPOut;              // optional, may be 0
  const U8  * pReport;
  U16         NumBytesReport;
  U16         BuffSize;           // buffer must be provided, if EPOut == 0
  U8        * pBuff;              // to receive data from the host.
} USB_HID_INIT_DATA;

typedef enum _USB_HID_REPORT_TYPE {
  USB_HID_REPORT_TYPE_INPUT   = USB_HID_INPUT_REPORT,
  USB_HID_REPORT_TYPE_OUTPUT  = USB_HID_OUTPUT_REPORT,
  USB_HID_REPORT_TYPE_FEATURE = USB_HID_FEATURE_REPORT
} USB_HID_REPORT_TYPE;

typedef int  USB_HID_ON_GETREPORT_REQUEST_FUNC(USB_HID_REPORT_TYPE ReportType, unsigned ReportId, const U8 ** pData, U32 * pNumBytes);
typedef void USB_HID_ON_SETREPORT_REQUEST_FUNC(USB_HID_REPORT_TYPE ReportType, unsigned ReportId, U32 NumBytes);

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void           USBD_HID_Init                   (void);
USB_HID_HANDLE USBD_HID_Add                    (const USB_HID_INIT_DATA * pInitData);
unsigned       USBD_HID_GetNumBytesInBuffer    (USB_HID_HANDLE hInst);
int            USBD_HID_Read                   (USB_HID_HANDLE hInst, void* pData, unsigned NumBytes, unsigned Timeout);
int            USBD_HID_ReadOverlapped         (USB_HID_HANDLE hInst, void* pData, unsigned NumBytes);
void           USBD_HID_StartReadTransfer      (USB_HID_HANDLE hInst);
int            USBD_HID_Write                  (USB_HID_HANDLE hInst, const void* pData, unsigned NumBytes, int Timeout);
unsigned       USBD_HID_GetNumBytesRemToWrite  (USB_HID_HANDLE hInst);
unsigned       USBD_HID_GetNumBytesRemToRead   (USB_HID_HANDLE hInst);
int            USBD_HID_WaitForTX              (USB_HID_HANDLE hInst, unsigned Timeout);
int            USBD_HID_WaitForRX              (USB_HID_HANDLE hInst, unsigned Timeout);
void           USBD_HID_SetOnGetReportRequest  (USB_HID_HANDLE hInst, USB_HID_ON_GETREPORT_REQUEST_FUNC * pfOnGetReportRequest);
void           USBD_HID_SetOnSetReportRequest  (USB_HID_HANDLE hInst, USB_HID_ON_SETREPORT_REQUEST_FUNC * pfOnSetReportRequest);

/*********************************************************************
*
*       Wrapper for emUSB V2 migration
*
**********************************************************************
*/
#define USB_HID_Init                             USBD_HID_Init
#define USB_HID_Add(x)                           USBD_HID_Add(x)
#define USB_HID_GetNumBytesInBuffer()            USBD_HID_GetNumBytesInBuffer(0)
#define USB_HID_Read(p, n)                       USBD_HID_Read(0, p, n, 0)
#define USB_HID_ReadTimed(p, n, t)               USBD_HID_Read(0, p, n, t)
#define USB_HID_ReadEPOverlapped(p, n)           USBD_HID_ReadOverlapped(0, p, n)
#define USB_HID_StartReadTransfer()              USBD_HID_StartReadTransfer(0)
#define USB_HID_Write(p, n)                      USBD_HID_Write(0, p, n, 0)
#define USB_HID_WriteTimed(p, n, t)              USBD_HID_Write(0, p, n, t)
#define USB_HID_WriteEPOverlapped(p, n)          USBD_HID_Write(0, p, n, -1)
#define USB_HID_GetNumBytesToWrite()             USBD_HID_GetNumBytesRemToWrite(0)
#define USB_HID_GetNumBytesRemToRead()           USBD_HID_GetNumBytesRemToRead(0)
#define USB_HID_WaitForTX()                      USBD_HID_WaitForTX(0, 0)
#define USB_HID_WaitForRX()                      USBD_HID_WaitForRX(0, 0)
#define USB_HID_IsStartReadTransferActive()      (1)

#define USB_HID_GetNumBytesInBufferEx(h)         USBD_HID_GetNumBytesInBuffer(h)
#define USB_HID_ReadEx(h, p, n)                  USBD_HID_Read(h, p, n, 0)
#define USB_HID_ReadExTimed(h, p, n, t)          USBD_HID_Read(h, p, n, t)
#define USB_HID_ReadEPOverlappedEx(h, p, n)      USBD_HID_ReadOverlapped(h, p, n)
#define USB_HID_StartReadTransferEx(h)           USBD_HID_StartReadTransfer(h)
#define USB_HID_WriteEx(h, p, n)                 USBD_HID_Write(h, p, n, 0)
#define USB_HID_WriteExTimed(h, p, n, t)         USBD_HID_Write(h, p, n, t)
#define USB_HID_WriteEPOverlappedEx(h, p, n)     USBD_HID_Write(h, p, n, -1)
#define USB_HID_GetNumBytesToWriteEx(h)          USBD_HID_GetNumBytesRemToWrite(h)
#define USB_HID_GetNumBytesRemToReadEx(h)        USBD_HID_GetNumBytesRemToRead(h)
#define USB_HID_WaitForTXEx(h)                   USBD_HID_WaitForTX(h, 0)
#define USB_HID_WaitForRXEx(h)                   USBD_HID_WaitForRX(h, 0)
/*lint -esym(621, USB_HID_IsStartReadTransferActiveEx) MISRA C:2012 Rule 5.1, required */
#define USB_HID_IsStartReadTransferActiveEx(h)   (1)

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
