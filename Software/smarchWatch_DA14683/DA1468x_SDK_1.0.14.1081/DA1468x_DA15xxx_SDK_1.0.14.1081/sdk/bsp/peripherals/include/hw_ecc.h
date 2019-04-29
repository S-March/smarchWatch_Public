/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup ECC
 * \{
 * \brief ECC Engine
 *
 * # General description
 *
 * In order to use the ECC engine, the following steps must be followed:
 * 1. Enable its clock.
 * 2. Load the engine microcode.
 * 3. Configure the RAM address that the engine will use for input/output and
 *    intermediate data.
 * 4. Write input data in the ECC RAM (locations depend on the operation).
 * 5. Configure the command register and the configuration register (if the operation
 *    requires to configure operands).
 * 6. Start the engine.
 * 7. Wait until the engine completes it operation.
 * 8. Check the status register and if no error occurred and the operation generates
 *    data, read the output data from the ECC memory.
 *
 * The ECC RAM is used for storing input/output data and also intermediate data that
 * the ECC engine calculates during its operation. This RAM block must be aligned to
 * a 1KByte address. This RAM area is segmented into 256-bit locations. The first 16
 * locations are used for input/output data while the rest are used by the ECC engine
 * for storing intermediate data. The amount of RAM used depends on the operation and
 * it can reach up to 2KBytes.
 *
 * ## Primitive arithmetic operations
 *
 * These include modular addition, subtraction, multiplication, reduction, division,
 * inversion on prime numbers p or other numbers N and non-modular multiplication.
 *
 * *C = A op B mod p or N*
 *
 * Operand  | Location
 * -------- | ---------
 * p        | 0 
 * N        | 1
 * A, B, C  | 1 - 15
 *
 * ## Primitive ECC operations
 *
 * These include the following:
 * - Point addition *P3 = P1 + P2*
 * - Point doubling *P3 = 2P1*
 * - Point multiplication *P3 = kP1*
 * - Check a and b parameters *a < p and b < p*
 * - Check n *n not equal to p*
 * - Check point coordinates * Pi = (Xi, Yi), Xi < p and Yi < p*
 * - Check point Pi = (Xi, Yi) is on curve
 *
 * Operand A must point to the location of P1
 * Operand B must point to the location of P2 or k
 * Operand C must point to the location of P3
 *
 * Operand                 | Location
 * ----------------------- | ---------
 * field size q            | 0
 * order n                 | 1
 * Gx                      | 2
 * Gy                      | 3
 * a                       | 4
 * b                       | 5
 * k or Point coordinate X | 6, 8, 10, 12, 14
 * Point coordinate Y      | 7, 9, 11, 13, 15
 *
 * ## ECDSA
 *
 * The ECDSA operations are
 * - Signature generation that produces the signature pair r, s
 * - Signature verification that updates the status register accordingly
 * - Domain parameters validation that updates the status register accordingly
 *
 * Operand                   | Location
 * ------------------------- | ---------
 * field size q              | 0
 * order n                   | 1
 * Gx                        | 2
 * Gy                        | 3
 * a                         | 4
 * b                         | 5
 * private key dA            | 6
 * random number k           | 7
 * public key Q x coordinate | 8
 * public key Q y coordinate | 9
 * r                         | 10
 * s                         | 11
 * message hash h            | 12
 *
 */

/**
 ****************************************************************************************
 *
 * @file hw_ecc.h
 *
 * @brief Definition of API for the ECC Engine Low Level Driver.
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_ECC_H_
#define HW_ECC_H_

#if dg_configUSE_HW_ECC

#include "sdk_defs.h"
#include <string.h>

/**
 * \brief Busy status bit mask
 *
 * This is the bit of the status register that indicates that the engine is processing data.
 * It is active high and goes to low when the selected operation is finished.
 *
 * \sa hw_ecc_read_status()
 */
#define HW_ECC_STATUS_BUSY                      ECC_ECC_STATUS_REG_ECC_Busy_Msk

/**
 * \brief Not invertible bit mask
 *
 * This bit of the status register is set to 1 when the operand is not invertible in a
 * modular inversion operation.
 *
 * \sa hw_ecc_read_status()
 */
#define HW_ECC_STATUS_NOT_INVERTIBLE            ECC_ECC_STATUS_REG_ECC_NotInvertible_Msk

/**
 * \brief Invalid A and B parameters bit mask
 *
 * This bit of the status register is set to 1 when parameters A, B are not valid
 * (e.g. 4A + 27B = 0). It is updated with #HW_ECC_CMD_OP_CHECK_AB command.
 *
 * \sa hw_ecc_read_status()
 */
#define HW_ECC_STATUS_PARAM_AB_NOT_VALID        ECC_ECC_STATUS_REG_ECC_Param_AB_NotValid_Msk

/**
 * \brief Invalid signature bit mask
 *
 * This bit of the status register is set to 1 when a signature must be rejected.
 *
 * \sa hw_ecc_read_status()
 */
#define HW_ECC_STATUS_SIGNATURE_NOT_VALID       ECC_ECC_STATUS_REG_ECC_Signature_NotValid_Msk

/**
 * \brief Invalid N parameter bit mask
 *
 * This bit of the status register is set to 1 when parameter N is not valid. It is
 * updated with #HW_ECC_CMD_OP_CHECK_N command.
 *
 * \sa hw_ecc_read_status()
 */
#define HW_ECC_STATUS_PARAM_N_NOT_VALID         ECC_ECC_STATUS_REG_ECC_Param_n_NotValid_Msk

/**
 * \brief Invalid X, Y couple bit mask
 *
 * This bit of the status register is set to 1 when parameter the couple X, Y is not valid
 * (e.g. not smaller than the prime). It is updated with #HW_ECC_CMD_OP_CHECK_PXY command.
 *
 * \sa hw_ecc_read_status()
 */
#define HW_ECC_STATUS_COUPLE_NOT_VALID          ECC_ECC_STATUS_REG_ECC_Couple_NotValid_Msk

/**
 * \brief Point Px at infinity bit mask
 *
 * This bit of the status register is set to 1 when point Px is at the infinity in any
 * ECC operation.
 *
 * \sa hw_ecc_read_status()
 */
#define HW_ECC_STATUS_POINT_PX_ATINFINITY       ECC_ECC_STATUS_REG_ECC_Point_Px_AtInfinity_Msk

/**
 * \brief Point Px not on curve bit mask
 *
 * This bit of the status register is set to 1 when point Px is not on the defined curve.
 * This bit is updated with #HW_ECC_CMD_OP_POINT_ON_CURVE and #HW_ECC_CMD_OP_ED25519_PNTONC
 * commands.
 *
 * \sa hw_ecc_read_status()
 */
#define HW_ECC_STATUS_POINT_PX_NOTONCURVE       ECC_ECC_STATUS_REG_ECC_Point_Px_NotOnCurve_Msk

/**
 * \brief Fail location bit mask
 *
 * These 4 bits in the status register give the location in the ECC Data RAM of the last
 * point failure (e.g. not valid, at infinity, not on curve).
 *
 * \sa hw_ecc_read_status()
 */
#define HW_ECC_STATUS_FAIL_LOCATION             ECC_ECC_STATUS_REG_ECC_Fail_Address_Msk

/**
 * \brief Calculate R^2modN for the next operation
 *
 * This value is written to the command register. RmodN needs to be calculated when a new prime
 * has been programmed. It is automatically cleared when R^2modN is calculated.
 *
 * \sa HW_ECC_CMD_SIGNB, HW_ECC_CMD_SIGNA, HW_ECC_CMD_FIELD, HW_ECC_CMD_TYPEOF_OP
 *     HW_ECC_CMD_OP_SIZE
 * \sa hw_ecc_write_command_register()
 */
typedef enum {
        HW_ECC_CMD_CALCR2_FALSE = 0UL,  /**< No effect */
        HW_ECC_CMD_CALCR2_TRUE = 1UL    /**< Forces HW to re-calculate R^2 mod N */
} HW_ECC_CMD_CALCR2;

/**
 * \brief Sign of parameter B in y^2 = x^3 + A*x + B
 *
 * \note This field is also used to differentiate from different types of supported
 *       operations.
 *
 * \sa HW_ECC_CMD_CALCR2, HW_ECC_CMD_SIGNA, HW_ECC_CMD_FIELD, HW_ECC_CMD_TYPEOF_OP
 *     HW_ECC_CMD_OP_SIZE
 * \sa hw_ecc_write_command_register()
 */
typedef enum {
        HW_ECC_CMD_SIGNB_POS = 0UL,     /**< B is positive */
        HW_ECC_CMD_SIGNB_NEG = 1UL      /**< B is negative */
} HW_ECC_CMD_SIGNB;

/**
 * \brief Sign of parameter A in y^2 = x^3 + A*x + B
 *
 * \sa HW_ECC_CMD_CALCR2, HW_ECC_CMD_SIGNB, HW_ECC_CMD_FIELD, HW_ECC_CMD_TYPEOF_OP
 *     HW_ECC_CMD_OP_SIZE
 * \sa hw_ecc_write_command_register()
 */
typedef enum {
        HW_ECC_CMD_SIGNA_POS = 0UL,     /**< A is positive */
        HW_ECC_CMD_SIGNA_NEG = 1UL      /**< A is negative */
} HW_ECC_CMD_SIGNA;

/**
 * \brief Operand size
 *
 * \sa HW_ECC_CMD_CALCR2, HW_ECC_CMD_SIGNB, HW_ECC_CMD_SIGNA, HW_ECC_CMD_TYPEOF_OP
 *     HW_ECC_CMD_FIELD
 * \sa hw_ecc_write_command_register()
 */
typedef enum {
        HW_ECC_CMD_OP_SIZE_64B = 1UL,   /**< 64-bit operands */
        HW_ECC_CMD_OP_SIZE_128B = 2UL,  /**< 128-bit operands */
        HW_ECC_CMD_OP_SIZE_256B = 4UL,  /**< 256-bit operands */
} HW_ECC_CMD_OP_SIZE;

/**
 * \brief Field type
 *
 * \sa HW_ECC_CMD_CALCR2, HW_ECC_CMD_SIGNB, HW_ECC_CMD_SIGNA, HW_ECC_CMD_TYPEOF_OP,
 *     HW_ECC_CMD_OP_SIZE
 * \sa hw_ecc_write_command_register()
 */
typedef enum {
        HW_ECC_CMD_FIELD_FP = 0UL,      /**< Field is prime: F(p) */
        HW_ECC_CMD_FIELD_F2M = 1UL      /**< Field is binary: F(2m) */
} HW_ECC_CMD_FIELD;

/**
 * \brief Type of operation
 *
 * \sa HW_ECC_CMD_CALCR2, HW_ECC_CMD_SIGNB, HW_ECC_CMD_SIGNA, HW_ECC_CMD_FIELD
 *     HW_ECC_CMD_OP_SIZE
 * \sa hw_ecc_write_command_register()
 */
typedef enum {
        HW_ECC_CMD_OP_MODADD_P = 0x00000001UL,          /**< Modular Addition C = A + B mod p */
        HW_ECC_CMD_OP_MODADD_N = 0x40000001UL,          /**< Modular Addition C = A + B mod N */
        HW_ECC_CMD_OP_MODSUB_P = 0x00000002UL,          /**< Modular Substraction C = A - B mod p */
        HW_ECC_CMD_OP_MODSUB_N = 0x40000002UL,          /**< Modular Substraction C = A - B mod N */
        HW_ECC_CMD_OP_MODMLT_P = 0x00000003UL,          /**< Modular Multiplication C = A * B mod p */
        HW_ECC_CMD_OP_MODMLT_N = 0x40000003UL,          /**< Modular Multiplication C = A * B mod N */
        HW_ECC_CMD_OP_MODRED_P = 0x00000004UL,          /**< Modular Reduction C = B mod p */
        HW_ECC_CMD_OP_MODRED_N = 0x40000004UL,          /**< Modular Reduction C = B mod N */
        HW_ECC_CMD_OP_MODDIV_P = 0x00000005UL,          /**< Modular Division C = A / B mod p */
        HW_ECC_CMD_OP_MODDIV_N = 0x40000005UL,          /**< Modular Division C = A / B mod N */
        HW_ECC_CMD_OP_MODINV_P = 0x00000006UL,          /**< Modular Inversion C = 1 / B mod p */
        HW_ECC_CMD_OP_MODINV_N = 0x40000006UL,          /**< Modular Inversion C = 1 / B mod N */
        HW_ECC_CMD_OP_MULT     = 0x00000008UL,          /**< Multiplication C = A * B */
        HW_ECC_CMD_OP_MODINV_E = 0x00000009UL,          /**< Modular Inversion C = 1 / B mod N (Even N) */
        HW_ECC_CMD_OP_MODRED_E = 0x0000000AUL,          /**< Modular Reduction C = B mod N (Even N) */
        HW_ECC_CMD_OP_JPAKE_MULMODN = 0x40000010UL,     /**< J-PAKE r = (a * b) mod N */
        HW_ECC_CMD_OP_JPAKE_GENZKP = 0x00000011UL,      /**< J-PAKE GenZKP r = (v - x * h) mod N */
        HW_ECC_CMD_OP_EDDSA_MULTADDN = 0x40000011UL,    /**< EdDSA MultAddN C = A + B * H mod N */
        HW_ECC_CMD_OP_ECMQV_PT1 = 0x40000012UL,         /**< ECMQV Part 1 v = h * (x + a * Xbar) */
        HW_ECC_CMD_OP_POINT_DBL = 0x00000020UL,         /**< Point Doubling */
        HW_ECC_CMD_OP_POINT_ADD = 0x00000021UL,         /**< Point Addition */
        HW_ECC_CMD_OP_POINT_ADD3 = 0x40000021UL,        /**< 3 Points Addition */
        HW_ECC_CMD_OP_POINT_MLT = 0x00000022UL,         /**< Point Multiplication */
        HW_ECC_CMD_OP_GENSESSKEY = 0x40000022UL,        /**< Generate Session Key P4 = (B=X4*x2s)*x2 */
        HW_ECC_CMD_OP_CHECK_AB = 0x00000023UL,          /**< Check parameters a and b */
        HW_ECC_CMD_OP_CHECK_N = 0x00000024UL,           /**< Check order n of point G */
        HW_ECC_CMD_OP_CHECK_PXY = 0x00000025UL,         /**< Check Point Coordinates */
        HW_ECC_CMD_OP_CHECK_LESS_N = 0x40000025UL,      /**< Check single value less than N */
        HW_ECC_CMD_OP_POINT_ON_CURVE = 0x00000026UL,    /**< Check Point on Curve */
        HW_ECC_CMD_OP_C25519_PNT_MULT = 0x00000028UL,   /**< Curve25519 Point Multiplication*/
        HW_ECC_CMD_OP_ED25519_XRECOVER = 0x00000029UL,  /**< Ed25519: Recover x based on y for a point on curve*/
        HW_ECC_CMD_OP_ED25519_PNTONC = 0x40000029UL,    /**< Ed25519: Check point on curve */
        HW_ECC_CMD_OP_ED25519_SCLMLT = 0x0000002AUL,    /**< Ed25519: Point Multiplication */
        HW_ECC_CMD_OP_ED25519_CHECK_SIG = 0x0000002BUL, /**< Ed25519: Check signature */
        HW_ECC_CMD_OP_ECDSA_GEN_SIG = 0x00000030UL,     /**< ECDSA: Signature Generation */
        HW_ECC_CMD_OP_ECMQV_PT2 = 0x40000030UL,         /**< ECMQV Part 2 S = (Y + Ybar*B)*v */
        HW_ECC_CMD_OP_ECDSA_VER_SIG = 0x00000031UL,     /**< ECDSA: Signature Verification */
        HW_ECC_CMD_OP_JPAKE_VERZKP = 0x40000031UL,      /**< J-PAKE Verify ZKP */
        HW_ECC_CMD_OP_ECDSA_DOMAIN_VLD = 0x00000032UL,  /**< ECDSA: Domain Parameters Validation */
} HW_ECC_CMD_TYPEOF_OP;

/**
 * \brief Set the base address for the ECC Data RAM
 *
 * The ECC engine requires up to 2kBytes of the system's DataRAM where input and output 
 * data are stored. This function configures the engine to use for this purpose two consecutive 
 * 1kByte pages starting from the page within which the input address falls. In case the
 * input address points to the last 1kByte page of SysRAM, then the top and the bottom
 * 1KByte pages of SysRAM will be used. This address space is divided into locations of
 * 256 bits, from which the first 16 are used for input and output data.
 *
 * \param [in] base_addr An address within SysRAM range. If not already aligned to 1KByte
 *                       the function will use the closest previous 1kByte aligned address.
 */
void hw_ecc_set_base_addr(volatile void *base_addr);

/**
 * \brief Get address of an ECC RAM location for a specific base address
 *
 * \param [in] base_addr An address within SysRAM range. Assumed to be aligned to 1KByte.
 * \param [in] location The location number (0 to 15).
 * \returns The address of the location.
 */
static inline volatile void *hw_ecc_get_location_address(unsigned int location, volatile void *base_addr)
{
        /* Only 16 256-bit locations are available in the Data RAM used by ECC */
        ASSERT_WARNING(location < 16);

        /* The address needs to be in SysRAM and aligned to 1KByte */
        ASSERT_WARNING(IS_SYSRAM_ADDRESS(base_addr) && !((unsigned long)base_addr & 0x3FFUL));

        return (volatile void *)((volatile uint8_t *)base_addr + location * 32);
}

/**
 * \brief Get address of an ECC RAM location based on the configured base address
 *
 * \param [in] location The location number (0 to 15).
 * \returns The address of the location.
 *
 * \sa hw_ecc_set_base_addr()
 */
static inline volatile void *hw_ecc_get_configured_location_address(unsigned int location)
{
        return hw_ecc_get_location_address(location, (void *)(MEMORY_SYSRAM_BASE + 1024 * GPREG->ECC_BASE_ADDR_REG));
}

/**
 * \brief Load the ECC engine microcode
 *
 * \param[in] ucode A pointer to the memory area that contains the microcode to be loaded.
 * \param[in] ucode_size The size of the microcode in 32-bit words
 */
static inline void hw_ecc_load_ucode(const uint32_t *ucode, unsigned int ucode_size)
{
        OPT_MEMCPY((uint32_t *)MEMORY_ECC_UCODE_BASE, ucode, ucode_size);
}

/**
 * \brief Enable ECC engine clock
 */
static inline void hw_ecc_enable_clock(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, ECC_CLK_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable ECC engine clock
 */
static inline void hw_ecc_disable_clock(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, CLK_AMBA_REG, ECC_CLK_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Check if ECC engine clock is enabled
 *
 * \return Non-zero value if enabled, 0 otherwise
 */
static inline int hw_ecc_clock_is_enabled(void)
{
        return (CRG_TOP->CLK_AMBA_REG) & (CRG_TOP_CLK_AMBA_REG_ECC_CLK_ENABLE_Msk);
}

/**
 * \brief Configure operand locations
 *
 * When executing primitive arithmetic operations, the location of the operands and results
 * are configured with this function. Usable locations are only 0x0 to 0xF.
 *
 * \param [in] a First operand location.
 * \param [in] b Second operand location.
 * \param [in] c Result location.
 */
static inline void hw_ecc_cfg_ops(unsigned int a, unsigned int b, unsigned int c)
{
        /* Only 16 256-bit locations are available in the Data RAM used by ECC */
        ASSERT_ERROR(a <= 0xF);
        ASSERT_ERROR(b <= 0xF);
        ASSERT_ERROR(c <= 0xF);

        ECC->ECC_CONFIG_REG = (c << 16) | (b << 8) | a;
}

/**
 * \brief Write the command register
 *
 * \param [in] calcr2 Calculate R2modN
 * \param [in] signb Sign of parameter B
 * \param [in] signa Sign of parameter A
 * \param [in] operands_size In multiples of 64. Up to 256-bits parameters are supported.
 * \param [in] field Prime or binary field selection
 * \param [in] typeof_op Type of operation
 *
 * \note signb bit in command register is used in some cases in order to select among different types
 *       of operations. In these cases, the value passed through this function is ignored.
 *
 * \sa HW_ECC_CMD_CALCR2, HW_ECC_CMD_SIGNB, HW_ECC_CMD_SIGNA, HW_ECC_CMD_FIELD, HW_ECC_CMD_TYPEOF_OP
 */
static inline void hw_ecc_write_command_register(HW_ECC_CMD_CALCR2 calcr2,
                                                 HW_ECC_CMD_SIGNB signb,
                                                 HW_ECC_CMD_SIGNA signa,
                                                 HW_ECC_CMD_OP_SIZE operands_size,
                                                 HW_ECC_CMD_FIELD field,
                                                 HW_ECC_CMD_TYPEOF_OP typeof_op)
{
        /* Operands can only be up to 256-bits */
        ASSERT_ERROR(operands_size > 0 && operands_size < 0x5);

        ECC->ECC_COMMAND_REG = (calcr2 << ECC_ECC_COMMAND_REG_ECC_CalcR2_Pos) |
                               (signb << ECC_ECC_COMMAND_REG_ECC_SignB_Pos) |
                               (signa << ECC_ECC_COMMAND_REG_ECC_SignA_Pos) |
                               (operands_size << ECC_ECC_COMMAND_REG_ECC_SizeOfOperands_Pos) |
                               (field << ECC_ECC_COMMAND_REG_ECC_Field_Pos) |
                               (typeof_op << ECC_ECC_COMMAND_REG_ECC_TypeOperation_Pos);
}

/**
 * \brief Write raw value to the command register
 *
 * \warning This function does not perform any check on the value correctness.
 *
 * \sa hw_ecc_write_command_register()
 */
static inline void hw_ecc_write_command_register_raw(uint32_t value)
{
        ECC->ECC_COMMAND_REG = value;
}

/**
 * \brief Start ECC operation
 *
 * This function starts an ECC operation. All input data must be stored in the ECC Data RAM and all
 * other registers must properly configured for this operation before calling this function.
 */ 
static inline void hw_ecc_start(void)
{
        ECC->ECC_CONTROL_REG = 1;
}

/**
 * \brief Read status register
 *
 * The result of this function can be used along with various mask defined in macros to get information
 * about ECC status engine, operation outputs and errors.
 *
 * \sa HW_ECC_STATUS_BUSY, HW_ECC_STATUS_PARAM_AB_NOT_VALID, HW_ECC_STATUS_SIGNATURE_NOT_VALID,
 *     HW_ECC_STATUS_PARAM_N_NOT_VALID, HW_ECC_STATUS_COUPLE_NOT_VALID, HW_ECC_STATUS_POINT_PX_ATINFINITY,
 *     HW_ECC_STATUS_POINT_PX_NOTONCURVE, HW_ECC_STATUS_FAIL_LOCATION
 */
static inline unsigned int hw_ecc_read_status(void)
{
        return ECC->ECC_STATUS_REG;
}

/**
 * \brief Clear ECC interrupt source
 */
static inline void hw_ecc_clear_interrupt_source(void)
{
        volatile uint32_t dummy __attribute__((unused));
        dummy = ECC->ECC_STATUS_REG;
}

/**
 * \brief Write a 256-bit operand to ECC engine data RAM
 *
 * \param [in] location The location where the operand will be written (0 to 15)
 * \param [in] data The buffer containing the 256-bit operand
 * \param [in] base_addr The base address of the ECC engine data RAM (must be 1KiB aligned)
 */
void hw_ecc_write256(unsigned int location, const uint8_t *data, volatile uint8_t *base_addr);

/**
 * \brief Write a 256-bit operand to ECC engine data RAM in reversed byte order
 *
 * \param [in] location The location where the operand will be written (0 to 15)
 * \param [in] data The buffer containing the 256-bit operand
 * \param [in] base_addr The base address of the ECC engine data RAM (must be 1KiB aligned)
 */
void hw_ecc_write256_r(unsigned int location, const uint8_t *data, volatile uint8_t *base_addr);

/**
 * \brief Read a 256-bit operand from ECC engine data RAM
 *
 * \param [in] location The location from where the operand will be read (0 to 15)
 * \param [in] data The buffer where the 256-bit operand will be stored
 * \param [in] base_addr The base address of the ECC engine data RAM (must be 1KiB aligned)
 */
void hw_ecc_read256(unsigned int location, uint8_t *data, volatile uint8_t *base_addr);

/**
 * \brief Read a 256-bit operand from ECC engine data RAM in reversed byte order
 *
 * \param [in] location The location from where the operand will be read (0 to 15)
 * \param [in] data The buffer where the 256-bit operand will be stored
 * \param [in] base_addr The base address of the ECC engine data RAM (must be 1KiB aligned)
 */
void hw_ecc_read256_r(unsigned int location, uint8_t *data, volatile uint8_t *base_addr);

/**
 * \brief Write a 128-bit operand to ECC engine data RAM
 *
 * \param [in] location The location where the operand will be written (0 to 15)
 * \param [in] data The buffer containing the 128-bit operand
 * \param [in] base_addr The base address of the ECC engine data RAM (must be 1KiB aligned)
 */
void hw_ecc_write128(unsigned int location, const uint8_t *data, volatile uint8_t *base_addr);

/**
 * \brief Write a 128-bit operand to ECC engine data RAM in reversed byte order
 *
 * \param [in] location The location where the operand will be written (0 to 15)
 * \param [in] data The buffer containing the 128-bit operand
 * \param [in] base_addr The base address of the ECC engine data RAM (must be 1KiB aligned)
 */
void hw_ecc_write128_r(unsigned int location, const uint8_t *data, volatile uint8_t *base_addr);

/**
 * \brief Read a 128-bit operand from ECC engine data RAM
 *
 * \param [in] location The location from where the operand will be read (0 to 15)
 * \param [in] data The buffer where the 128-bit operand will be stored
 * \param [in] base_addr The base address of the ECC engine data RAM (must be 1KiB aligned)
 */
void hw_ecc_read128(unsigned int location, uint8_t *data, volatile uint8_t *base_addr);

/**
 * \brief Read a 128-bit operand from ECC engine data RAM in reversed byte order
 *
 * \param [in] location The location from where the operand will be read (0 to 15)
 * \param [in] data The buffer where the 128-bit operand will be stored
 * \param [in] base_addr The base address of the ECC engine data RAM (must be 1KiB aligned)
 */
void hw_ecc_read128_r(unsigned int location, uint8_t *data, volatile uint8_t *base_addr);

#endif /* dg_configUSE_HW_ECC */

#endif /* HW_ECC_H_ */

/**
 * \}
 * \}
 * \}
 */

