/**
 ****************************************************************************************
 *
 * @file bsp_memory_layout.h
 *
 * @brief Board Support Package. System Configuration file default values.
 *
 * Copyright (C) 2017-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BSP_MEMORY_LAYOUT_H_
#define BSP_MEMORY_LAYOUT_H_

/*************************************************************************************************\
 * Default configuration for retention RAM
 */

#if !defined(RELEASE_BUILD) && (dg_configOPTIMAL_RETRAM == 1)
        /* WARNING: retRAM optimizations are disabled in DEBUG builds! */
        #undef dg_configOPTIMAL_RETRAM
        #define dg_configOPTIMAL_RETRAM         (0)
#elif (dg_configEXEC_MODE != MODE_IS_CACHED)
        /* WARNING: retRAM optimizations are not applicable in MIRRORED mode! */
        #undef dg_configOPTIMAL_RETRAM
        #define dg_configOPTIMAL_RETRAM         (0)
#endif

#if (dg_configOPTIMAL_RETRAM == 0)
        #undef  dg_configMEM_RETENTION_MODE
        #define dg_configMEM_RETENTION_MODE     (0x1F)
        #undef  dg_configSHUFFLING_MODE
        #define dg_configSHUFFLING_MODE         (0x3)


#endif

/*************************************************************************************************\
 * Memory layout configuration
 */

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_OTP)
        #if (dg_configEXEC_MODE == MODE_IS_CACHED)
                #if (dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A)
                        // CODE_SIZE cannot be more than 58K
                        #define CODE_SIZE       dg_configOTP_CODE_SIZE_AE
                        /* DA14681-01
                         * RAM goes first, RetRAM0 follows. RetRAM1 is added at the beginning when
                         * optimized RetRAM configuration is used (so that the IVT is preserved).
                         * RAM size should be defined such that it covers the whole empty space
                         * between RetRAM1, if it exists, and RetRAM0.
                         */
                        #define RETRAM_FIRST    0

                        #define RAM_SIZE        dg_configOTP_CACHED_RAM_SIZE_AE

                        #if (dg_configOPTIMAL_RETRAM == 0)
                                #define RETRAM_0_SIZE   dg_configOTP_CACHED_RETRAM_0_SIZE_AE
                                #define RETRAM_1_SIZE   dg_configOTP_CACHED_RETRAM_1_SIZE_AE
                        #else
                                #define RETRAM_0_SIZE   dg_configOTP_CACHED_OPTIMAL_RETRAM_0_SIZE_AE
                                #define RETRAM_1_SIZE   dg_configOTP_CACHED_OPTIMAL_RETRAM_1_SIZE_AE
                        #endif
                #else
                        // CODE_SIZE cannot be more than 58K
                        #define CODE_SIZE       dg_configOTP_CODE_SIZE_AE
                        /* DA14682/3-00, DA15XXX-00
                         * RetRAM goes first, RAM follows
                         */
                        #define RETRAM_FIRST    1

                        #define RAM_SIZE        dg_configOTP_CACHED_RAM_SIZE_BB
                        #define RETRAM_0_SIZE   dg_configOTP_CACHED_RETRAM_0_SIZE_BB
                        #define RETRAM_1_SIZE   dg_configOTP_CACHED_RETRAM_1_SIZE_BB
                #endif
        #else // MIRRORED
                #if (dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A)
                        #define CODE_SIZE       dg_configOTP_CODE_SIZE_AE
                        /* DA14681-01
                         * CODE is first, RetRAM follows. RAM is last, always 16K.
                         *
                         * RetRAM uses all RAM5 block. RAM uses CACHE.
                         */
                        #define RETRAM_FIRST    1

                        #define RAM_SIZE        dg_configOTP_MIRROR_RAM_SIZE_AE
                        #define RETRAM_0_SIZE   (128 * 1024 - CODE_SIZE)
                        #define RETRAM_1_SIZE   dg_configOTP_MIRROR_RETRAM_1_SIZE_AE
                #else
                        #define CODE_SIZE       dg_configOTP_CODE_SIZE_BB
                        /* DA14682/3-00, DA15XXX-00
                         * RetRAM0 is first, RAM follows. CODE is last!
                         */
                        #define RETRAM_FIRST    1

                        #define RAM_SIZE        dg_configOTP_MIRROR_RAM_SIZE_BB
                        #define RETRAM_0_SIZE   dg_configOTP_MIRROR_RETRAM_0_SIZE_BB
                        #define RETRAM_1_SIZE   dg_configOTP_MIRROR_RETRAM_1_SIZE_BB
                #endif
        #endif

        #if (CODE_SIZE > (58 * 1024))
                #error "maximum CODE size when OTP is used is 58K!"
        #endif

#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH)
        #if (dg_configEXEC_MODE == MODE_IS_CACHED)
                #if (dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A)
                        #define CODE_SIZE       dg_configQSPI_CODE_SIZE_AE
                        /* DA14681-01
                         * RAM goes first, RetRAM0 follows. RetRAM1 is added at the beginning when
                         * optimized RetRAM configuration is used (so that the IVT is preserved).
                         */
                        #define RETRAM_FIRST    0

                        #define RAM_SIZE        dg_configQSPI_CACHED_RAM_SIZE_AE

                        #if (dg_configOPTIMAL_RETRAM == 0)
                                #define RETRAM_0_SIZE   dg_configQSPI_CACHED_RETRAM_0_SIZE_AE
                                #define RETRAM_1_SIZE   dg_configQSPI_CACHED_RETRAM_1_SIZE_AE
                        #else
                                #define RETRAM_0_SIZE   dg_configQSPI_CACHED_OPTIMAL_RETRAM_0_SIZE_AE
                                #define RETRAM_1_SIZE   dg_configQSPI_CACHED_OPTIMAL_RETRAM_1_SIZE_AE
                        #endif
                #else
                        #define CODE_SIZE       dg_configQSPI_CODE_SIZE_BB
                        /* DA14682/3-00, DA15XXX-00
                         * RetRAM goes first, RAM follows
                         */
                        #define RETRAM_FIRST    1

                        #define RAM_SIZE        dg_configQSPI_CACHED_RAM_SIZE_BB
                        #define RETRAM_0_SIZE   dg_configQSPI_CACHED_RETRAM_0_SIZE_BB
                        #define RETRAM_1_SIZE   dg_configQSPI_CACHED_RETRAM_1_SIZE_BB
                #endif
        #else // MIRRORED
                #error "QSPI mirrored mode is not supported!"
        #endif

#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
        #if (dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A)
                #define CODE_SIZE       dg_configRAM_CODE_SIZE_AE
        #else
                #define CODE_SIZE       dg_configRAM_CODE_SIZE_BB
        #endif

        #if (dg_configEXEC_MODE == MODE_IS_CACHED)
                #pragma message "RAM cached mode is not supported! Reset to RAM (mirrored) mode!"
                #undef dg_configEXEC_MODE
                #define dg_configEXEC_MODE      MODE_IS_RAM
        #endif

        #if (dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A)
                /* DA14681-01
                 * CODE is first, RetRAM follows. RAM is last, always 16K.
                 *
                 * RetRAM uses all RAM5 block. RAM uses CACHE.
                 */
                #define RETRAM_FIRST    1

                #define RAM_SIZE        dg_configRAM_RAM_SIZE_AE
                #define RETRAM_0_SIZE   dg_configRAM_RETRAM_0_SIZE_AE
                #define RETRAM_1_SIZE   dg_configRAM_RETRAM_1_SIZE_AE
        #else
                /* DA14682/3-00, DA15XXX-00
                 * RetRAM0 is first, RAM follows. CODE is last!
                 */
                #define RETRAM_FIRST    1

                #define RAM_SIZE        dg_configRAM_RAM_SIZE_BB
                #define RETRAM_0_SIZE   dg_configRAM_RETRAM_0_SIZE_BB
                #define RETRAM_1_SIZE   dg_configRAM_RETRAM_1_SIZE_BB
        #endif

#else
        #error "Unknown configuration..."
#endif

#endif /* BSP_MEMORY_LAYOUT_H_ */
