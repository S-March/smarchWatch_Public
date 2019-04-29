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
File    : USB_SmartMSD.h
Purpose : USB_SmartMSD API specification
--------- END-OF-HEADER ----------------------------------------------
*/

#ifndef USB_SMARTMSD_H            // Avoid multiple inclusion
#define USB_SMARTMSD_H

#include "Global.h"
#include "USB_MSD.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define USB_SMSD_ATTR_READ_ONLY       (0x01)
#define USB_SMSD_ATTR_HIDDEN          (0x02)
#define USB_SMSD_ATTR_SYSTEM          (0x04)
#define USB_SMSD_ATTR_VOLUME_ID       (0x08)
#define USB_SMSD_ATTR_DIRECTORY       (0x10)
#define USB_SMSD_ATTR_ARCHIVE         (0x20)
#define USB_SMSD_ATTR_LONG_NAME       (USB_SMSD_ATTR_READ_ONLY | USB_SMSD_ATTR_HIDDEN | USB_SMSD_ATTR_SYSTEM | USB_SMSD_ATTR_VOLUME_ID)
#define USB_SMSD_ATTR_LONG_NAME_MASK  (USB_SMSD_ATTR_READ_ONLY | USB_SMSD_ATTR_HIDDEN | USB_SMSD_ATTR_SYSTEM | USB_SMSD_ATTR_VOLUME_ID | USB_SMSD_ATTR_DIRECTORY | USB_SMSD_ATTR_ARCHIVE)

//
// For use in USB_SMSD_CONST_FILE.Flags
//
#define USB_SMSD_FILE_WRITABLE        USB_SMSD_ATTR_READ_ONLY
#define USB_SMSD_FILE_AHEAD           (1<<8)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  const char* sName;
  const U8* pData;
  int FileSize;
  U32 Flags;
} USB_SMSD_CONST_FILE;

typedef struct {
  U8  acFilename[8];
  U8  acExt[3];
  U8  DirAttr;
  U8  NTRes;
  U8  CrtTimeTenth;
  U16 CrtTime;
  U16 CrtDate;
  U16 LstAccDate;
  U16 FstClusHI;
  U16 WrtTime;
  U16 WrtDate;
  U16 FstClusLO;
  U32 FileSize;
} USB_SMSD_DIR_ENTRY_SHORT;

typedef struct {
  U8  Ord;
  U8  acName1[10];
  U8  Attr;
  U8  Type;
  U8  Chksum;
  U8  acName2[12];
  U16 FstClusLO;
  U8  acName3[4];
} USB_SMSD_DIR_ENTRY_LONG;

typedef union {
  USB_SMSD_DIR_ENTRY_SHORT ShortEntry;
  USB_SMSD_DIR_ENTRY_LONG  LongEntry;
  U8                       ac[32];
} USB_SMSD_DIR_ENTRY;

typedef struct {
  const USB_SMSD_DIR_ENTRY* pDirEntry;
} USB_SMSD_FILE_INFO;

typedef int    USB_SMSD_ON_READ_FUNC  (unsigned Lun,       U8   * pData, U32 Off, U32 NumBytes, const USB_SMSD_FILE_INFO * pFile);
typedef int    USB_SMSD_ON_WRITE_FUNC (unsigned Lun, const U8   * pData, U32 Off, U32 NumBytes, const USB_SMSD_FILE_INFO * pFile);
typedef void * USB_SMSD_MEM_ALLOC     (U32          Size);
typedef void   USB_SMSD_MEM_FREE      (void       * p);
typedef void   USB_SMSD_ON_PANIC      (const char * sErr);

typedef struct _USB_SMSD_USER_FUNC_API {
  USB_SMSD_ON_READ_FUNC  * pfOnReadSector;             // Mandatory
  USB_SMSD_ON_WRITE_FUNC * pfOnWriteSector;            // Mandatory
  USB_SMSD_MEM_ALLOC     * pfMemAlloc;                 // Optional
  USB_SMSD_MEM_FREE      * pfMemFree;                  // Optional
} USB_SMSD_USER_FUNC_API;

/*********************************************************************
*
*       Const data
*
**********************************************************************
*/
extern const USB_MSD_STORAGE_API USB_MSD_StorageSMSD;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
//
// Configuration functions that can be called within the USB_SMSD_X_Config function
//
void USBD_SMSD_AssignMemory          (U32 * p, U32 NumBytes);
void USBD_SMSD_SetUserAPI            (const USB_SMSD_USER_FUNC_API            * pUserFunc);
void USBD_SMSD_SetNumRootDirSectors  (unsigned Lun, int                         NumRootDirSectors);
int  USBD_SMSD_SetVolumeInfo         (unsigned Lun, const char                * sVolumeName, const USB_MSD_LUN_INFO * pLunInfo);
int  USBD_SMSD_AddConstFiles         (unsigned Lun, const USB_SMSD_CONST_FILE * paConstFile, int NumFiles);    // Add list of predefined files. such as Readme.txt, ...
void USBD_SMSD_SetNumSectors         (unsigned Lun, int                         NumSectors);
void USBD_SMSD_SetSectorsPerCluster  (unsigned Lun, int                         SectorsPerCluster);

void USBD_SMSD_Add                   (void);
void USBD_SMSD_Init                  (void);
void USBD_SMSD_ReInit                (void);
void USBD_SMSD_DeInit                (void);

void USB_SMSD_X_Config               (void);    // Has to be defined by user

/*********************************************************************
*
*       Wrapper for emUSB V2 migration
*
**********************************************************************
*/

#define USB_SMSD_AssignMemory           USBD_SMSD_AssignMemory
#define USB_SMSD_SetUserAPI             USBD_SMSD_SetUserAPI
#define USB_SMSD_SetNumRootDirSectors   USBD_SMSD_SetNumRootDirSectors
#define USB_SMSD_SetVolumeInfo          USBD_SMSD_SetVolumeInfo
#define USB_SMSD_AddConstFiles          USBD_SMSD_AddConstFiles
#define USB_SMSD_SetNumSectors          USBD_SMSD_SetNumSectors
#define USB_SMSD_SetSectorsPerCluster   USBD_SMSD_SetSectorsPerCluster

#define USB_SMSD_Init                   USBD_SMSD_Add
#define USB_SMSD_ReInit                 USBD_SMSD_ReInit
#define USB_SMSD_DeInit                 USBD_SMSD_DeInit

/*********************************************************************
*  End of Wrapper
**********************************************************************/

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
