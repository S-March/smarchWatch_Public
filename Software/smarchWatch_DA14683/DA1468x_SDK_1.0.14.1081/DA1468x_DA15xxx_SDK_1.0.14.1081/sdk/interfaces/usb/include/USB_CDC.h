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
File    : USB_CDC.h
Purpose : Public header of the communication device class
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef CDC_H          /* Avoid multiple inclusion */
#define CDC_H

#include "USB_SEGGER.h"
#include "USB.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif


/*********************************************************************
*
*       Config defaults
*
**********************************************************************
*/
#define CDC_USB_CLASS     2         // 2: Communication Device Class
#define CDC_USB_SUBCLASS  0x00      //
#define CDC_USB_PROTOCOL  0x00      //

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  U32 DTERate;
  U8  CharFormat;
  U8  ParityType;
  U8  DataBits;
} USB_CDC_LINE_CODING;

typedef struct {
  U8 DCD;           // CDC spec: bRxCarrier
  U8 DSR;           // CDC spec: bTxCarrier
  U8 Break;         // CDC spec: bBreak
  U8 Ring;          // CDC spec: bRingSignal
  U8 FramingError;  // CDC spec: bFraming
  U8 ParityError;   // CDC spec: bParity
  U8 OverRunError;  // CDC spec: bOverRun
  U8 CTS;           // CDC spec: not specified
} USB_CDC_SERIAL_STATE;

typedef struct USB_CDC_CONTROL_LINE_STATE {
  U8 DTR;
  U8 RTS;
} USB_CDC_CONTROL_LINE_STATE;

typedef void USB_CDC_ON_SET_LINE_CODING(USB_CDC_LINE_CODING * pLineCoding);
typedef void USB_CDC_ON_SET_CONTROL_LINE_STATE(USB_CDC_CONTROL_LINE_STATE * pLineState);
typedef void USB_CDC_ON_SET_BREAK(unsigned BreakDuration);

typedef int USB_CDC_HANDLE;

/*********************************************************************
*
*       Communication interface
*/
typedef struct {
  U8 EPIn;
  U8 EPOut;
  U8 EPInt;
} USB_CDC_INIT_DATA;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void           USBD_CDC_Init                       (void);
USB_CDC_HANDLE USBD_CDC_Add                        (const USB_CDC_INIT_DATA * pInitData);
void           USBD_CDC_CancelRead                 (USB_CDC_HANDLE hInst);
void           USBD_CDC_CancelWrite                (USB_CDC_HANDLE hInst);
int            USBD_CDC_Read                       (USB_CDC_HANDLE hInst, void * pData, unsigned NumBytes, unsigned Timeout);
int            USBD_CDC_ReadOverlapped             (USB_CDC_HANDLE hInst, void * pData, unsigned NumBytes);
int            USBD_CDC_Receive                    (USB_CDC_HANDLE hInst, void * pData, unsigned NumBytes, int Timeout);
void           USBD_CDC_SetOnBreak                 (USB_CDC_HANDLE hInst, USB_CDC_ON_SET_BREAK * pfOnBreak);
void           USBD_CDC_SetOnLineCoding            (USB_CDC_HANDLE hInst, USB_CDC_ON_SET_LINE_CODING * pf);
void           USBD_CDC_SetOnControlLineState      (USB_CDC_HANDLE hInst, USB_CDC_ON_SET_CONTROL_LINE_STATE * pf);
void           USBD_CDC_WriteSerialState           (USB_CDC_HANDLE hInst);
void           USBD_CDC_UpdateSerialState          (USB_CDC_HANDLE hInst, const USB_CDC_SERIAL_STATE * pSerialState);
int            USBD_CDC_WaitForTX                  (USB_CDC_HANDLE hInst, unsigned Timeout);
int            USBD_CDC_WaitForRX                  (USB_CDC_HANDLE hInst, unsigned Timeout);
int            USBD_CDC_Write                      (USB_CDC_HANDLE hInst, const void * pData, unsigned NumBytes, int Timeout);
int            USBD_CDC_GetNumBytesInBuffer        (USB_CDC_HANDLE hInst);
int            USBD_CDC_GetNumBytesRemToRead       (USB_CDC_HANDLE hInst);
int            USBD_CDC_GetNumBytesRemToWrite      (USB_CDC_HANDLE hInst);
int            USBD_CDC_TxIsPending                (USB_CDC_HANDLE hInst);
void           USBD_CDC_SetOnTXEvent               (USB_CDC_HANDLE hInst, USB_EVENT_CALLBACK *pEventCb, USB_EVENT_CALLBACK_FUNC *pfEventCb, void *pContext);
void           USBD_CDC_SetOnRXEvent               (USB_CDC_HANDLE hInst, USB_EVENT_CALLBACK *pEventCb, USB_EVENT_CALLBACK_FUNC *pfEventCb, void *pContext);

/*********************************************************************
*
*       Wrapper for emUSB V2 migration
*
**********************************************************************
*/
#define  USB_CDC_Init                            USBD_CDC_Init
#define  USB_CDC_Add(x)                          USBD_CDC_Add(x)
#define  USB_CDC_CancelRead()                    USBD_CDC_CancelRead(0)
#define  USB_CDC_CancelWrite()                   USBD_CDC_CancelWrite(0)
#define  USB_CDC_Read(p, n)                      USBD_CDC_Read(0, p, n, 0)
#define  USB_CDC_ReadOverlapped(p, n)            USBD_CDC_ReadOverlapped(0, p, n)
#define  USB_CDC_ReadTimed(p, n, t)              USBD_CDC_Read(0, p, n, t)
#define  USB_CDC_Receive(p, n)                   USBD_CDC_Receive(0, p, n, 0)
#define  USB_CDC_ReceiveTimed(p, n, t)           USBD_CDC_Receive(0, p, n, t)
#define  USB_CDC_SetOnBreak(pf)                  USBD_CDC_SetOnBreak(0, pf)
#define  USB_CDC_SetOnLineCoding(pf)             USBD_CDC_SetOnLineCoding(0, pf)
#define  USB_CDC_SetOnControlLineState(pf)       USBD_CDC_SetOnControlLineState(0, pf)
#define  USB_CDC_UpdateSerialState(ps)           USBD_CDC_UpdateSerialState(0, ps)
#define  USB_CDC_WaitForTX()                     USBD_CDC_WaitForTX(0, 0)
#define  USB_CDC_WaitForRX()                     USBD_CDC_WaitForRX(0, 0)
#define  USB_CDC_WriteSerialState()              USBD_CDC_WriteSerialState(0)
#define  USB_CDC_Write(p, n)                     USBD_CDC_Write(0, p, n, 0)
#define  USB_CDC_WriteOverlapped(p, n)           USBD_CDC_Write(0, p, n, -1)
#define  USB_CDC_WriteTimed(p, n, t)             USBD_CDC_Write(0, p, n, t)
#define  USB_CDC_StartReadTransfer()             USBD_CDC_Receive(0, NULL, 0, -1)
#define  USB_CDC_IsStartReadTransferActive()     (1)
#define  USB_CDC_GetNumBytesInBuffer()           USBD_CDC_GetNumBytesInBuffer(0)
#define  USB_CDC_GetNumBytesRemToRead()          USBD_CDC_GetNumBytesRemToRead(0)
#define  USB_CDC_GetNumBytesToWrite()            USBD_CDC_GetNumBytesRemToWrite(0)
#define  USB_CDC_TxIsPending()                   USBD_CDC_TxIsPending(0)

#define  USB_CDC_CancelReadEx(h)                 USBD_CDC_CancelRead(h)
#define  USB_CDC_CancelWriteEx(h)                USBD_CDC_CancelWrite(h)
#define  USB_CDC_ReadEx(h, p, n)                 USBD_CDC_Read(h, p, n, 0)
#define  USB_CDC_ReadOverlappedEx(h, p, n)       USBD_CDC_ReadOverlapped(h, p, n)
#define  USB_CDC_ReadTimedEx(h,p, n, t)          USBD_CDC_Read(h, p, n, t)
#define  USB_CDC_ReceiveEx(h, p, n)              USBD_CDC_Receive(h, p, n, 0)
#define  USB_CDC_ReceiveTimedEx(h, p, n, t)      USBD_CDC_Receive(h, p, n, t)
#define  USB_CDC_SetOnBreakEx(h, pf)             USBD_CDC_SetOnBreak(h, pf)
#define  USB_CDC_SetOnLineCodingEx(h, pf)        USBD_CDC_SetOnLineCoding(h, pf)
#define  USB_CDC_SetOnControlLineStateEx(h, pf)  USBD_CDC_SetOnControlLineState(h, pf)
#define  USB_CDC_UpdateSerialStateEx(h, ps)      USBD_CDC_UpdateSerialState(h, ps)
#define  USB_CDC_WaitForTXEx(h)                  USBD_CDC_WaitForTX(h, 0)
#define  USB_CDC_WaitForRXEx(h)                  USBD_CDC_WaitForRX(h, 0)
#define  USB_CDC_WriteEx(h, p, n)                USBD_CDC_Write(h, p, n, 0)
#define  USB_CDC_WriteOverlappedEx(h, p, n)      USBD_CDC_Write(h, p, n, -1)
#define  USB_CDC_WriteTimedEx(h, p, n, t)        USBD_CDC_Write(h, p, n, t)
#define  USB_CDC_WriteSerialStateEx(h)           USBD_CDC_WriteSerialState(h)
#define  USB_CDC_StartReadTransferEx(h)          USBD_CDC_Receive(h, NULL, 0, -1)
/*lint -esym(621, USB_CDC_IsStartReadTransferActiveEx) MISRA C:2012 Rule 5.1, required */
#define  USB_CDC_IsStartReadTransferActiveEx(h)  (1)
#define  USB_CDC_GetNumBytesInBufferEx(h)        USBD_CDC_GetNumBytesInBuffer(h)
#define  USB_CDC_GetNumBytesRemToReadEx(h)       USBD_CDC_GetNumBytesRemToRead(h)
#define  USB_CDC_GetNumBytesToWriteEx(h)         USBD_CDC_GetNumBytesRemToWrite(h)
#define  USB_CDC_TxIsPendingEx(h)                USBD_CDC_TxIsPending(h)

/*********************************************************************
*  End of Wrapper
**********************************************************************/

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
