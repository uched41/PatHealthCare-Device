#ifndef BLE_TRANSFER_QUEUE_H
#define BLE_TRANSFER_QUEUE_H

#define BLE_QUEUE_PRIORITY_LEVELS   4   // Number of priority levels
#define BLE_QUEUE_HIGH_PRIORITY     0
#define BLE_QUEUE_COMMAND_PRIORITY  1
#define BLE_QUEUE_DATA_PRIORITY     2
#define BLE_QUEUE_LOW_PRIORITY      3

/* Functions */
void ble_queue_init(void);

void ble_queue_loop();

void ble_clear_buffers(void);

#endif


