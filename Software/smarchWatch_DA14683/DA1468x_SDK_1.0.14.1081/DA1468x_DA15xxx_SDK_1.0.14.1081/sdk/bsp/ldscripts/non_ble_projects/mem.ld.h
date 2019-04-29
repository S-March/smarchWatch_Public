/* Linker script to configure memory regions.
 * May need to be modified for a specific board.
 *   ROM.ORIGIN: starting address of read-only RAM area
 *   ROM.LENGTH: length of read-only RAM area
 *   RetRAMx.ORIGIN: starting address of retained RAMx area
 *   RetRAMx.LENGTH: length of retained RAMx area
 *   RAM.ORIGIN: starting address of read-write RAM area
 *   RAM.LENGTH: length of read-write RAM area
 *
 **************************************************************************************************
 *
 * The positioning of the ROM changes depending on the type of the non-volatile memory used and the
 * execution mode. More specifically,
 *
 *   |------------------|------------------|------------------|
 *   |   execution mode |     mirrored     |      cached      |
 *   |code is stored in |                  |                  |
 *   |------------------|------------------|------------------|
 *   |    JTAG Download |  RAM (0x7FC0000) |        N/A       |
 *   |------------------|------------------|------------------|
 *   |              OTP |  RAM (0x7FC0000) |  OTP (0x7F80000) |
 *   |------------------|------------------|------------------|
 *   |            Flash |        N/A       | QSPI (0x8000000) |
 *   |------------------|------------------|------------------|
 *
 * The positioning of the RAM and the RetRAM areas depends on the positioning of the ROM area and
 * the placement of the ROM variables.
 *
 * In theory, up to 3 non-continuous Retention Memory areas may be defined. Up to 3
 * non-continuous RAM Memory areas may be defined, also. Note that the GNU Linker does not support
 * automatic splitting of sections. So, for example, having the RAM area lying at [0x7FC8000 -
 * 0x7FCA000] and [0x7FD4000 - 0x7FD6000] is not possible. Two separate RAM sections should be
 * defined, RAM1 and RAM2, and a non-automatic way of placing data into these sections should be
 * derived. This is application specific.
 *
 * Parameters that control the final memory layout:
 * - CODE_SIZE     The size of the code in the format the linker understands (e.g. 64K).
 * - RETRAM_x_SIZE The size of the Retention RAM x.
 * - RAM_SIZE      The size of the RAM.
 * - RETRAM_FIRST  A switch that controls whether the RetRAM will be placed before the RAM or not.
 *
 * Limitations
 * - This version supports only 2 Retention RAM and 1 RAM areas.
 *
 * Note
 *
 * In DA14680/1-01 mirrored mode the highest memory location that the code uses, is the one defined
 * by its actual size. In principle, this could be as high as 0x7FDC000 (when the BLE is used) but
 * this would mean that (i) no retained data have been defined (which is ok, since in mirrored mode
 * all RAM + Cache is retained) and (ii) that the data should fit inside 16K (the Cache).
 */
#if (dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B)
/*
 * CODE_SIZE in DA14682/3-XX mirrored mode must cover the highest memory location the code will use.
 * Due to the relocation that takes place, this location may be quite high, close to the end of the
 * RAM or the Cache. So, it is safe to set CODE_SIZE to (144 * 1024) in this case. Note though, that
 * the code is always placed after RetRAM0 and RAM in this mode. So, it must eventually fit in the
 * available memory (end below 0x7FE4000).
 */
#endif

/*** Do not change anything below this line! ***/

/* --------------------------------------------------------------------------------------------- */
#if (dg_configOPTIMAL_RETRAM == 1)
        #undef RETRAM_FIRST
        #define RETRAM_FIRST    1
#endif

#if (dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A)
        #define IVT_AREA_OVERHEAD               0x100
#elif (dg_configEXEC_MODE == MODE_IS_CACHED)
        #define IVT_AREA_OVERHEAD               0x200
#else
        #define IVT_AREA_OVERHEAD               0x400

#endif

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
        #if (dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A)
                #define CODE_BASE_ADDRESS               (0x7FC0000)
                #define RAM_BASE_ADDRESS                (CODE_BASE_ADDRESS + CODE_SIZE)
                #define __RAM_OVERHEAD                  (0)
        #else
                #define CODE_BASE_ADDRESS               (0x7FC0000)
                #define RAM_BASE_ADDRESS                (0x7FC0000) // CODE area and RAM overlap!!!
                #define __RAM_OVERHEAD                  (IVT_AREA_OVERHEAD)
        #endif

#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_OTP)
        #if (dg_configEXEC_MODE != MODE_IS_CACHED)
                #define CODE_BASE_ADDRESS       (0x7FC0000)
                #if (dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A)
                        #define RAM_BASE_ADDRESS        (CODE_BASE_ADDRESS + CODE_SIZE)
                        #define __RAM_OVERHEAD          (0)
                #else
                        #define RAM_BASE_ADDRESS        (0x7FC0000) // CODE area and RAM overlap!!!
                        #define __RAM_OVERHEAD          (IVT_AREA_OVERHEAD)
                #endif
        #else
                #define CODE_BASE_ADDRESS       (0x7F80000)
                #define RAM_BASE_ADDRESS        (0x7FC0000)
                #define __RAM_OVERHEAD          (IVT_AREA_OVERHEAD)
        #endif

#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH)
        #if (dg_configEXEC_MODE != MODE_IS_CACHED)
                #error "QSPI mirrored execution mode is not supported!"
        #else
                #define CODE_BASE_ADDRESS       (0x8000000 + dg_configIMAGE_FLASH_OFFSET)
                #define RAM_BASE_ADDRESS        (0x7FC0000)
                #define __RAM_OVERHEAD          (IVT_AREA_OVERHEAD)
        #endif

#else
        #error "Unknown code location type..."
#endif

#if (RETRAM_1_SIZE > IVT_AREA_OVERHEAD)
        #define RETRAM_1_SZ                     (RETRAM_1_SIZE - IVT_AREA_OVERHEAD)
#else
        #define RETRAM_1_SZ                     0
#endif

#ifdef RETRAM_0_BASE
        #undef RETRAM_0_OFFSET
        #define RETRAM_0_OFFSET                 (RETRAM_0_BASE - (RAM_BASE_ADDRESS))
#endif

#ifdef RAM_BASE
        #undef RAM_OFFSET
        #define RAM_OFFSET                      (RAM_BASE + RETRAM_1_SZ - (RAM_BASE_ADDRESS))
#endif

#if (RETRAM_FIRST == 1)
        #if (RETRAM_1_SZ != 0)
                #error "RETRAM_1 has been defined while RETRAM_FIRST == 1..."
        #endif

        #ifndef RETRAM_0_OFFSET
                #define RETRAM_0_OFFSET         (__RAM_OVERHEAD) /* offset from RAM_BASE_ADDRESS */
        #endif

        #if (RETRAM_0_SIZE > IVT_AREA_OVERHEAD)
                #define RETRAM_0_SZ             (RETRAM_0_SIZE - __RAM_OVERHEAD)
        #else
                #define RETRAM_0_SZ             (0) /* Wrong configuration */
        #endif

        #ifndef RAM_OFFSET
                #define RAM_OFFSET              (RETRAM_0_SIZE)
        #endif

        #define RAM_SZ                          (RAM_SIZE)

#else
        #ifndef RAM_OFFSET
                #if (RETRAM_1_SZ == 0)
                #define RAM_OFFSET              (__RAM_OVERHEAD) /* offset from RAM_BASE_ADDRESS */
                #else
                #define RAM_OFFSET              (RETRAM_1_SIZE)
                #endif
        #endif

        #if (RAM_SIZE > IVT_AREA_OVERHEAD)
                #if (RETRAM_1_SZ == 0)
                #define RAM_SZ                  (RAM_SIZE - __RAM_OVERHEAD)
                #else
                #define RAM_SZ                  (RAM_SIZE)
                #endif
        #else
                #define RAM_SZ                  (0) /* Wrong configuration */
        #endif

        #ifndef RETRAM_0_OFFSET
                #define RETRAM_0_OFFSET         (RAM_SIZE + RETRAM_1_SIZE)
        #endif

        #define RETRAM_0_SZ                     (RETRAM_0_SIZE)
#endif

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE) || (dg_configEXEC_MODE != MODE_IS_CACHED)
        #define RAM_UPPER_LIMIT                 (0x7FE4000)
#else
        #define RAM_UPPER_LIMIT                 (0x7FE0000)
#endif

#if ((RAM_BASE_ADDRESS + RETRAM_0_OFFSET) < 0x7FC0000) || \
    ((RAM_BASE_ADDRESS + RETRAM_0_OFFSET) > RAM_UPPER_LIMIT) || \
    ((RAM_BASE_ADDRESS + RETRAM_0_OFFSET + RETRAM_0_SZ) > RAM_UPPER_LIMIT)
        #error "RetRAM0 area is out of bounds!"
#endif

#if ((RAM_BASE_ADDRESS + RAM_OFFSET) < 0x7FC0000) || \
    ((RAM_BASE_ADDRESS + RAM_OFFSET) > RAM_UPPER_LIMIT) || \
    ((RAM_BASE_ADDRESS + RAM_OFFSET + RAM_SZ) > RAM_UPPER_LIMIT)
        #error "RAM area is out of bounds!"
#endif

/* --------------------------------------------------------------------------------------------- */

#define _RETRAM_0_BASE_ADDR             ((RAM_BASE_ADDRESS + RETRAM_0_OFFSET) & 0x7FFF000)

#if (dg_configMEM_RETENTION_MODE & 1)
        #define _MEM_1_SZ               0x2000
#else
        #define _MEM_1_SZ               0x0
#endif

#if (dg_configSHUFFLING_MODE == 0)
        #define _MEM_1_BASE             0x7FC0000
        #define RMEM_1_START            (_MEM_1_BASE)
        #define RMEM_1_END              (_MEM_1_BASE + _MEM_1_SZ)
#elif (dg_configSHUFFLING_MODE == 1)
        #define _MEM_1_BASE             0x7FC6000
        #define RMEM_2_START            (_MEM_1_BASE)
        #define RMEM_2_END              (_MEM_1_BASE + _MEM_1_SZ)
#elif (dg_configSHUFFLING_MODE == 2)
        #define _MEM_1_BASE             0x7FC8000
        #define RMEM_2_START            (_MEM_1_BASE)
        #define RMEM_2_END              (_MEM_1_BASE + _MEM_1_SZ)
#else
        #define _MEM_1_BASE             0x7FCE000
        #define RMEM_3_START            (_MEM_1_BASE)
        #define RMEM_3_END              (_MEM_1_BASE + _MEM_1_SZ)
#endif


#if (dg_configMEM_RETENTION_MODE & 2)
        #define _MEM_2_SZ               0x6000
#else
        #define _MEM_2_SZ               0x0
#endif

#if (dg_configSHUFFLING_MODE == 0)
        #define _MEM_2_BASE             0x7FC2000
        #define RMEM_2_START            (_MEM_2_BASE)
        #define RMEM_2_END              (_MEM_2_BASE + _MEM_2_SZ)
#elif (dg_configSHUFFLING_MODE == 1)
        #define _MEM_2_BASE             0x7FC0000
        #define RMEM_1_START            (_MEM_2_BASE)
        #define RMEM_1_END              (_MEM_2_BASE + _MEM_2_SZ)
#elif (dg_configSHUFFLING_MODE == 2)
        #define _MEM_2_BASE             0x7FCA000
        #define RMEM_3_START            (_MEM_2_BASE)
        #define RMEM_3_END              (_MEM_2_BASE + _MEM_2_SZ)
#else
        #define _MEM_2_BASE             0x7FC8000
        #define RMEM_2_START            (_MEM_2_BASE)
        #define RMEM_2_END              (_MEM_2_BASE + _MEM_2_SZ)
#endif


#if (dg_configMEM_RETENTION_MODE & 4)
        #define _MEM_3_SZ               0x8000
#else
        #define _MEM_3_SZ               0x0
#endif

#if (dg_configSHUFFLING_MODE == 0)
        #define _MEM_3_BASE             0x7FC8000
        #define RMEM_3_START            (_MEM_3_BASE)
        #define RMEM_3_END              (_MEM_3_BASE + _MEM_3_SZ)
#elif (dg_configSHUFFLING_MODE == 1)
        #define _MEM_3_BASE             0x7FC8000
        #define RMEM_3_START            (_MEM_3_BASE)
        #define RMEM_3_END              (_MEM_3_BASE + _MEM_3_SZ)
#elif (dg_configSHUFFLING_MODE == 2)
        #define _MEM_3_BASE             0x7FC0000
        #define RMEM_1_START            (_MEM_3_BASE)
        #define RMEM_1_END              (_MEM_3_BASE + _MEM_3_SZ)
#else
        #define _MEM_3_BASE             0x7FC0000
        #define RMEM_1_START            (_MEM_3_BASE)
        #define RMEM_1_END              (_MEM_3_BASE + _MEM_3_SZ)
#endif

#if (dg_configMEM_RETENTION_MODE & 8)
        #define _MEM_4_SZ               0x8000
#else
        #define _MEM_4_SZ               0x0
#endif
#define _MEM_4_BASE                     0x7FD0000
#define RMEM_4_START                    (_MEM_4_BASE)
#define RMEM_4_END                      (_MEM_4_BASE + _MEM_4_SZ)

#if (dg_configMEM_RETENTION_MODE & 16)
        #define _MEM_5_SZ               0x8000
#else
        #define _MEM_5_SZ               0x0
#endif
#define _MEM_5_BASE                     0x7FD8000
#define RMEM_5_START                    (_MEM_5_BASE)
#define RMEM_5_END                      (_MEM_5_BASE + _MEM_5_SZ)

// Retention RAM can be up to 3 blocks.
#if (RMEM_1_START != RMEM_1_END)
        #define RETBLOCK_1_START        RMEM_1_START
# if (RMEM_2_START != RMEM_2_END)
#  if (RMEM_3_START != RMEM_3_END)
#   if (RMEM_4_START != RMEM_4_END)
#    if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_1_END          RMEM_5_END
#    else
        #define RETBLOCK_1_END          RMEM_4_END
#    endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_START        0
        #define RETBLOCK_2_END          0
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#   else // (RMEM_4_START != RMEM_4_END)
        #define RETBLOCK_1_END          RMEM_3_END
#    if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_START        RMEM_5_START
        #define RETBLOCK_2_END          RMEM_5_END
#    else
        #define RETBLOCK_2_START        0
        #define RETBLOCK_2_END          0
#    endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#   endif // (RMEM_4_START != RMEM_4_END)
#  else // (RMEM_3_START != RMEM_3_END)
        #define RETBLOCK_1_END          RMEM_2_END
#   if (RMEM_4_START != RMEM_4_END)
        #define RETBLOCK_2_START        RMEM_4_START
#    if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_END          RMEM_5_END
#    else
        #define RETBLOCK_2_END          RMEM_4_END
#    endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#   else // (RMEM_4_START != RMEM_4_END)
#    if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_START        RMEM_5_START
        #define RETBLOCK_2_END          RMEM_5_END
#    else
        #define RETBLOCK_2_START        0
        #define RETBLOCK_2_END          0
#    endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#   endif // (RMEM_4_START != RMEM_4_END)
#  endif // (RMEM_3_START != RMEM_3_END)
# else // (RMEM_2_START != RMEM_2_END)
        #define RETBLOCK_1_END          RMEM_1_END
#  if (RMEM_3_START != RMEM_3_END)
        #define RETBLOCK_2_START        RMEM_3_START
#   if (RMEM_4_START != RMEM_4_END)
#    if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_END          RMEM_5_END
#    else
        #define RETBLOCK_2_END          RMEM_4_END
#    endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#   else // (RMEM_4_START != RMEM_4_END)
        #define RETBLOCK_2_END          RMEM_3_END
#    if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_3_START        RMEM_5_START
        #define RETBLOCK_3_END          RMEM_5_END
#    else
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#    endif  // (RMEM_5_START != RMEM_5_END)
#   endif  // (RMEM_4_START != RMEM_4_END)
#  else // (RMEM_3_START != RMEM_3_END)
#   if (RMEM_4_START != RMEM_4_END)
        #define RETBLOCK_2_START        RMEM_4_START
#    if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_END          RMEM_5_END
#    else
        #define RETBLOCK_2_END          RMEM_4_END
#    endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#   else // (RMEM_4_START != RMEM_4_END)
#    if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_START        RMEM_5_START
        #define RETBLOCK_2_END          RMEM_5_END
#    else
        #define RETBLOCK_2_START        0
        #define RETBLOCK_2_END          0
#    endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#   endif // (RMEM_4_START != RMEM_4_END)
#  endif // (RMEM_3_START != RMEM_3_END)
# endif // (RMEM_2_START != RMEM_2_END)

#elif (RMEM_2_START != RMEM_2_END)
        #define RETBLOCK_1_START        RMEM_2_START
# if (RMEM_3_START != RMEM_3_END)
#  if (RMEM_4_START != RMEM_4_END)
#   if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_1_END          RMEM_5_END
#   else
        #define RETBLOCK_1_END          RMEM_4_END
#   endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_START        0
        #define RETBLOCK_2_END          0
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#  else // (RMEM_4_START != RMEM_4_END)
        #define RETBLOCK_1_END          RMEM_3_END
#   if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_START        RMEM_5_START
        #define RETBLOCK_2_END          RMEM_5_END
#   else
        #define RETBLOCK_2_START        0
        #define RETBLOCK_2_END          0
#   endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#  endif // (RMEM_4_START != RMEM_4_END)
# else // (RMEM_3_START != RMEM_3_END)
        #define RETBLOCK_1_END          RMEM_2_END
#  if (RMEM_4_START != RMEM_4_END)
        #define RETBLOCK_2_START        RMEM_4_START
#   if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_END          RMEM_5_END
#   else
        #define RETBLOCK_2_END          RMEM_4_END
#   endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#  else // (RMEM_4_START != RMEM_4_END)
#   if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_START        RMEM_5_START
        #define RETBLOCK_2_END          RMEM_5_END
#   else
        #define RETBLOCK_2_START        0
        #define RETBLOCK_2_END          0
#   endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#  endif // (RMEM_4_START != RMEM_4_END)
# endif // (RMEM_3_START != RMEM_3_END)

#elif (RMEM_3_START != RMEM_3_END)
        #define RETBLOCK_1_START        RMEM_3_START
# if (RMEM_4_START != RMEM_4_END)
#  if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_1_END          RMEM_5_END
#  else
        #define RETBLOCK_1_END          RMEM_4_END
#  endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_START        0
        #define RETBLOCK_2_END          0
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
# else // (RMEM_4_START != RMEM_4_END)
        #define RETBLOCK_1_END          RMEM_3_END
#  if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_START        RMEM_5_START
        #define RETBLOCK_2_END          RMEM_5_END
#  else
        #define RETBLOCK_2_START        0
        #define RETBLOCK_2_END          0
#  endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
# endif // (RMEM_4_START != RMEM_4_END)

#elif (RMEM_4_START != RMEM_4_END)
        #define RETBLOCK_1_START        RMEM_4_START
# if (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_1_END          RMEM_5_END
# else
        #define RETBLOCK_1_END          RMEM_4_END
# endif // (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_2_START        0
        #define RETBLOCK_2_END          0
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0

#elif (RMEM_5_START != RMEM_5_END)
        #define RETBLOCK_1_START        RMEM_4_START
        #define RETBLOCK_1_END          RMEM_5_END
        #define RETBLOCK_2_START        0
        #define RETBLOCK_2_END          0
        #define RETBLOCK_3_START        0
        #define RETBLOCK_3_END          0
#endif

/* Take Cache into account */
#if (dg_configEXEC_MODE != MODE_IS_CACHED)
        #if (RETBLOCK_1_END == 0x7FE0000)
                #undef RETBLOCK_1_END
                #define RETBLOCK_1_END  0x7FE4000
        #endif

        #if (RETBLOCK_2_END == 0x7FE0000)
                #undef RETBLOCK_2_END
                #define RETBLOCK_2_END  0x7FE4000
        #endif

        #if (RETBLOCK_3_END == 0x7FE0000)
                #undef RETBLOCK_3_END
                #define RETBLOCK_3_END  0x7FE4000
        #endif
#endif

#if (RETRAM_0_SIZE != 0)
        #if (dg_configMEM_RETENTION_MODE == 0)
                #error "RetRAM is used but dg_configMEM_RETENTION_MODE is 0!"
        #endif

        #if (_RETRAM_0_BASE_ADDR >= RETBLOCK_1_START) \
                && ((_RETRAM_0_BASE_ADDR + RETRAM_0_SIZE) <= RETBLOCK_1_END)
                // RetRAM0 belongs to a retained block
        #elif (_RETRAM_0_BASE_ADDR >= RETBLOCK_2_START) \
                && ((_RETRAM_0_BASE_ADDR + RETRAM_0_SIZE) <= RETBLOCK_2_END)
                // RetRAM0 belongs to a retained block
        #elif (_RETRAM_0_BASE_ADDR >= RETBLOCK_3_START) \
                && ((_RETRAM_0_BASE_ADDR + RETRAM_0_SIZE) <= RETBLOCK_3_END)
                // RetRAM0 belongs to a retained block
        #else
                #error "RetRAM0 is used but dg_configMEM_RETENTION_MODE (or dg_configSHUFFLING_MODE) is not correct!"
        #endif
#endif

#if (RETRAM_1_SZ > 0)
        #define _RETRAM_1_BASE_ADDR     (RETBLOCK_1_START + IVT_AREA_OVERHEAD)
#else
        #define _RETRAM_1_BASE_ADDR     (0)
#endif

#if (RETBLOCK_1_START == 0x7FC0000) && ((_RETRAM_1_BASE_ADDR + RETRAM_1_SZ) <= RETBLOCK_1_END)
        // RetRAM1 belongs to a retained block
#else
        #error "RetRAM for Heaps is used but dg_configMEM_RETENTION_MODE (or dg_configSHUFFLING_MODE) is not correct!"
#endif

MEMORY
{
        ROM (rx)     : ORIGIN = CODE_BASE_ADDRESS,                  LENGTH = CODE_SIZE
        RetRAM0 (rwx): ORIGIN = RAM_BASE_ADDRESS + RETRAM_0_OFFSET, LENGTH = RETRAM_0_SZ
        RetRAM1 (rwx): ORIGIN = _RETRAM_1_BASE_ADDR,                LENGTH = RETRAM_1_SZ
        RAM (rw)     : ORIGIN = RAM_BASE_ADDRESS + RAM_OFFSET,      LENGTH = RAM_SZ
}
