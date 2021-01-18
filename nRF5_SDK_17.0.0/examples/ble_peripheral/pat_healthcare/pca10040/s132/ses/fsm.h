#ifndef FSM_H
#define FSM_H

#include "stdint.h"

typedef enum {
  STATE_STARTUP,
  STATE_INITIALIZATION_START,
  STATE_INITIALIZATION_END,
  STATE_IDLE,
  STATE_SAMPLING,
  STATE_DEEP_SLEEP,
  STATE_BATTERY_LOW,
  STATE_DFU_START
}main_state_t;


typedef enum{
  DEVICE_WATCH,
  DEVICE_POMODORO,
  DEVICE_STOPWATCH
}device_mode_t;


/***
 * Struct to store general device status information
 */
typedef struct{
  main_state_t prev_state;
  main_state_t state;
  device_mode_t device_mode;
  uint8_t dfu_ready;
  uint8_t ble_ready;            // BLE Connected and Notification enabled
}device_status_t;

extern device_status_t device_status;

void fsm_loop(void);

void fsm_change_state(main_state_t new_state);

void fsm_change_device_mode(device_mode_t mode);


#endif


