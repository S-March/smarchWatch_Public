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
File    : SEGGER.h
Purpose : Global types etc & general purpose utility functions
---------------------------END-OF-HEADER------------------------------
*/

#ifndef SEGGER_H            // Guard against multiple inclusion
#define SEGGER_H

#include <stdarg.h>
#include "Global.h"         // Type definitions: U8, U16, U32, I8, I16, I32

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Keywords/specifiers
*
**********************************************************************
*/

#ifndef INLINE
  #ifdef _WIN32
    //
    // Microsoft VC6 and newer.
    // Force inlining without cost checking.
    //
    #define INLINE  __forceinline
  #else
    #if (defined(__GNUC__))
      //
      // Force inlining with GCC
      //
      #define INLINE inline __attribute__((always_inline))
    #elif (defined(__ICCARM__) || defined(__CC_ARM) || defined(__RX) || defined(__ICCRX__))
      //
      // Other known compilers.
      //
      #define INLINE  inline
    #else
      //
      // Unknown compilers.
      //
      #define INLINE
    #endif
  #endif
#endif

/*********************************************************************
*
*       Function-like macros
*
**********************************************************************
*/

#define SEGGER_COUNTOF(a)          (sizeof((a))/sizeof((a)[0]))
#define SEGGER_MIN(a,b)            (((a) < (b)) ? (a) : (b))
#define SEGGER_MAX(a,b)            (((a) > (b)) ? (a) : (b))

#ifndef   SEGGER_USE_PARA                   // Some compiler complain about unused parameters.
  #define SEGGER_USE_PARA(Para) (void)Para  // This works for most compilers.
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define SEGGER_PRINTF_FLAG_ADJLEFT    (1 << 0)
#define SEGGER_PRINTF_FLAG_SIGNFORCE  (1 << 1)
#define SEGGER_PRINTF_FLAG_SIGNSPACE  (1 << 2)
#define SEGGER_PRINTF_FLAG_PRECEED    (1 << 3)
#define SEGGER_PRINTF_FLAG_ZEROPAD    (1 << 4)
#define SEGGER_PRINTF_FLAG_NEGATIVE   (1 << 5)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  char* pBuffer;
  int   BufferSize;
  int   Cnt;
} SEGGER_BUFFER_DESC;

typedef struct {
  int  CacheLineSize;                                 // 0: No Cache. Most Systems such as ARM9 use a 32 bytes cache line size.
  void (*pfDMB)       (void);                         // Optional DMB function for Data Memory Barrier to make sure all memory operations are completed.
  void (*pfClean)     (void *p, unsigned NumBytes);   // Optional clean function for cached memory.
  void (*pfInvalidate)(void *p, unsigned NumBytes);   // Optional invalidate function for cached memory.
} SEGGER_CACHE_CONFIG;

typedef struct SEGGER_SNPRINTF_CONTEXT_struct SEGGER_SNPRINTF_CONTEXT;

struct SEGGER_SNPRINTF_CONTEXT_struct {
  void*               pContext;                       // Application specific context.
  SEGGER_BUFFER_DESC* pBufferDesc;                    // Buffer descriptor to use for output.
  void (*pfFlush)(SEGGER_SNPRINTF_CONTEXT* pContext); // Callback executed once the buffer is full. Callback decides if the buffer gets cleared to store more or not.
};

typedef struct {
  void (*pfStoreChar)       (SEGGER_BUFFER_DESC* pBufferDesc, SEGGER_SNPRINTF_CONTEXT* pContext, char c);
  int  (*pfPrintUnsigned)   (SEGGER_BUFFER_DESC* pBufferDesc, SEGGER_SNPRINTF_CONTEXT* pContext, U32 v, unsigned Base, char Flags, int Width, int Precision);
  int  (*pfPrintInt)        (SEGGER_BUFFER_DESC* pBufferDesc, SEGGER_SNPRINTF_CONTEXT* pContext, I32 v, unsigned Base, char Flags, int Width, int Precision);
} SEGGER_PRINTF_API;

typedef void (*SEGGER_pFormatter)(SEGGER_BUFFER_DESC* pBufferDesc, SEGGER_SNPRINTF_CONTEXT* pContext, const SEGGER_PRINTF_API* pApi, va_list* pParamList, char Lead, int Width, int Precision);

typedef struct SEGGER_PRINTF_FORMATTER {
  struct SEGGER_PRINTF_FORMATTER* pNext;              // Pointer to next formatter.
  SEGGER_pFormatter               pfFormatter;        // Formatter function.
  char                            Specifier;          // Format specifier.
} SEGGER_PRINTF_FORMATTER;

/*********************************************************************
*
*       Utility functions
*
**********************************************************************
*/

void SEGGER_ARM_memcpy(void* pDest, const void* pSrc, int NumBytes);
void SEGGER_memcpy    (void* pDest, const void* pSrc, int NumBytes);
void SEGGER_memxor    (void* pDest, const void* pSrc, unsigned NumBytes);

void SEGGER_StoreChar    (SEGGER_BUFFER_DESC* pBufferDesc, char c);
void SEGGER_PrintUnsigned(SEGGER_BUFFER_DESC* pBufferDesc, U32 v, unsigned Base, int Precision);
void SEGGER_PrintInt     (SEGGER_BUFFER_DESC* pBufferDesc, I32 v, unsigned Base, int Precision);
int  SEGGER_snprintf     (char* pBuffer, int BufferSize, const char* sFormat, ...);
int  SEGGER_vsnprintf    (char* pBuffer, int BufferSize, const char* sFormat, va_list ParamList);
int  SEGGER_vsnprintfEx  (SEGGER_SNPRINTF_CONTEXT* pContext, const char* sFormat, va_list ParamList);

int  SEGGER_PRINTF_AddFormatter      (SEGGER_PRINTF_FORMATTER* pFormatter, SEGGER_pFormatter pfFormatter, char c);
void SEGGER_PRINTF_AddDoubleFormatter(void);
void SEGGER_PRINTF_AddIPFormatter    (void);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif                      // Avoid multiple inclusion

/*************************** End of file ****************************/
