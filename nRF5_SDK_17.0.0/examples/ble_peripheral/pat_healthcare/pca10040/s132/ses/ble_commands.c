#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "BLE"
#include "log.h"

#include "nrf_log.h"
#include "app_error.h"
#include "ble_transfer_queue.h"
#include "ble_commands.h"
#include "ble_nus.h"

extern uint16_t m_conn_handle;  /* Connection handle controlled in main.c */
extern ble_nus_t m_nus;         /* NUS object defined in main module */

/**
 * @brief Parse BLE commands sent by app
 * @Params: uint8_t* buf -> Buffer containing data
 *          uint8_t len  -> length of buffer
 * Ret: None
 */
void ble_command_parser(const uint8_t* buf, uint8_t len){

}

