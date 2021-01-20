#define LOG_HEAD "BLE_QUEUE"
#include "log.h"

#include "ble_transfer_queue.h"
#include "transfer_queue.h"
#include "fsm.h"
#include "string.h"
#include "ble_nus.h"

transfer_queues_t* ble_packet_queues;
extern uint16_t m_conn_handle;  /* Connection handle controlled in main.c */
extern ble_nus_t m_nus;         /* NUS object defined in main module */

/* Send Buf through BLE */
tq_ret_t ble_send_func(uint8_t* buf, uint16_t len){
  return ble_nus_data_send(&m_nus, buf, &len, m_conn_handle);
}

/* Add BLE Packet to Queue */
uint8_t ble_add_to_queue( uint8_t* buf, uint8_t len, uint8_t priority){
  if(buf == NULL) return 1;
  if((!device_status.ble_ready)) return 1;
  return transfer_queue_add(ble_packet_queues, buf, len, priority);
}

/* BLE Function to be called on every iteration or loop */
void ble_queue_loop(){
  if(!device_status.ble_ready) return;
  transfer_queue_loop(ble_packet_queues);
}

/* Initialize BLE Queues */
void ble_queue_init(void){
  ble_packet_queues = transfer_queue_init(ble_send_func, BLE_QUEUE_PRIORITY_LEVELS, 256);
  LOGD("Initialized");
}

void ble_clear_buffers(void){
  transfer_queue_clear(ble_packet_queues);
}
