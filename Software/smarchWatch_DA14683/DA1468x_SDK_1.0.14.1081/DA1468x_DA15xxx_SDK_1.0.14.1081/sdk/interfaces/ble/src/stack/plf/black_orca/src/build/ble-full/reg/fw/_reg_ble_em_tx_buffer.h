#ifndef __REG_BLE_EM_TX_BUFFER_H_
#define __REG_BLE_EM_TX_BUFFER_H_

#if (RWBLE_SW_VERSION_MAJOR >= 8)
extern unsigned int REG_BLE_EM_TX_BUFFER_SIZE;
#else
#define REG_BLE_EM_TX_BUFFER_SIZE 38
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

extern unsigned int _ble_base;
#define REG_BLE_EM_TX_BUFFER_BASE_ADDR (_ble_base)


#endif // __REG_BLE_EM_TX_BUFFER_H_

