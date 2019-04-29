#ifndef CC2420_CTRL_H_
#define CC2420_CTRL_H_

/**
\addtogroup BSP
\{
\addtogroup CC2420_CTRL
\{

\brief Low Lever Driver (LLD) for CC2420 Controller module.

\author Konstantinos Nakos <konstantinos.nakos@diasemi.com>, October 2014.
*/

#include <stdint.h>
//#include "spi.h"

#define SIMULATION    (0)

/*********************************************************************************
 *** delay_us()
 ***/
static __inline void delay_us(uint16_t value)
{
volatile uint16_t i;

//DBG_VAL(value);
for (i=0; i<value; i++)
    ;

//DBG_LINE();
}

//===================== Read/Write functions ===================================
uint32_t cc2420_spi_access(uint32_t dataToSend);
/**
\brief Read byte from the SPI RX/TX register.

\return The read byte
*/
uint32_t cc2420_spi_read_byte(void);

/**
\brief Write byte to the CC2420 SPI TX/RX register.

\param[in] Byte to be written
*/
void cc2420_spi_write_byte(uint32_t data);

//============== Interrupt handling ============================================

//==================== Configuration functions =================================

/**
 * \brief Initialize the CC2420 controller.
 */
void cc2420_init(void);

/**
 * \brief Set CC2420 SPI in loop mode
 *
 * \param [in] lb Should be 1 to enable loop mode and 0 to disable.
 */
void cc2420_spi_loop_set(uint8_t lb);

/**
 * \brief Set CC2420 Controller in Auto mode
 *
 * \param [in] auto_mode Should be 1 to enable Auto mode and 0 to disable.
 */
void cc2420_ctrl_set_auto(uint8_t auto_mode);

//=========================== CS handling function =============================

/**
\brief Set CC2420 SPI CS high.
*/
void cc2420_spi_cs_high(void);

/**
\brief Set CC2420 SPI CS low.
*/
void cc2420_spi_cs_low(void);

//=========================== FIFO status functions ============================

/**
 * \brief Check empty flag of RX FIFO.
 *
 * \return Empty flag of RX FIFO.
 */
uint8_t cc2420_spi_rx_fifo_empty(void);

/**
 * \brief Check full flag of RX FIFO.
 *
 * \return full flag of RX FIFO.
 */
uint8_t cc2420_spi_rx_fifo_full(void);

/**
 * \brief Check empty flag of TX FIFO.
 *
 * \return Empty flag of TX FIFO.
 */
uint8_t cc2420_spi_tx_fifo_empty(void);

/**
 * \brief Check full flag of TX FIFO.
 *
 * \return full flag of TX FIFO.
 */
uint8_t cc2420_spi_tx_fifo_full(void);

//=========================== OpenWSN API functions ============================
//=========================== Other functions ==================================

/**
 * \brief Get CC2420 SPI busy status.
 *
 * \return Status of CC2420 SPI BUSY bit.
 */
uint8_t cc2420_spi_busy(void);



#endif /* CC2420_CTRL_H_ */

/**
 * \}
 * \}
 * \}
 */

