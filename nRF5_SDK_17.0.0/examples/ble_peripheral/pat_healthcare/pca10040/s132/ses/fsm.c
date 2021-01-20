#include "app_error.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"

#define LOG_HEAD "FSM"
#include "log.h"

#include "fsm.h"
#include "ticker.h"
#include "sensors.h"
#include "ble_transfer_queue.h"
#include "max30003.h"

device_status_t device_status = {
  .state        = STATE_STARTUP,
  .ble_ready    = false,
  .dfu_ready    = false
};

/* Get String name for FSM State */
static char* get_state_string(main_state_t state){
  switch(state){
    case STATE_STARTUP:
      return "STATE_STARTUP"               ;
    case STATE_INITIALIZATION_START: 
      return "STATE_INITIALIZATION_START"  ;
    case STATE_INITIALIZATION_END: 
      return "STATE_INITIALIZATION_END"    ; 
    case STATE_IDLE: 
      return "STATE_IDLE"                  ; 
    case STATE_SAMPLING: 
      return "STATE_SAMPLING"              ; 
    case STATE_DEEP_SLEEP: 
      return "STATE_DEEP_SLEEP"            ; 
    case STATE_BATTERY_LOW: 
      return "STATE_BATTERY_LOW"           ; 
    case STATE_DFU_START: 
      return "STATE_DFU_START"             ; 
    default:
      return " "                           ; 
   }
   return " ";
}


void fsm_change_device_mode(device_mode_t mode){
 
}

/* Change FSM State */
void fsm_change_state(main_state_t new_state){
  device_status.prev_state = device_status.state;
  device_status.state = new_state;
  LOGD("%s -> %s", get_state_string(device_status.prev_state), get_state_string(device_status.state));
}


/***
 * Main FSM Loop
 */
void fsm_loop(void){
 
  switch(device_status.state){

    case STATE_STARTUP:
      APP_ERROR_CHECK(nrf_drv_gpiote_init());
      ble_queue_init();
      fsm_change_state(STATE_INITIALIZATION_START);
      nrf_delay_ms(10);
      break;

    case STATE_INITIALIZATION_START:
      ticker_init();
      sensors_init();
      fsm_change_state(STATE_INITIALIZATION_END);
      nrf_delay_ms(10);
      break;

    case STATE_INITIALIZATION_END:
      fsm_change_state(STATE_IDLE);
      break;

    case STATE_IDLE:
      ble_queue_loop();
      max30003_loop();
      break;

    case STATE_SAMPLING:
      ble_queue_loop();
      break;

    case STATE_DEEP_SLEEP:
      break;

    case STATE_BATTERY_LOW:
      break;

    case STATE_DFU_START:
      break;

    default:
      break;
  }

}