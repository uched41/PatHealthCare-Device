#ifndef MAX30003_H
#define MAX30003_H

#include "stdint.h"

#define RAW_ECG_PACKET_LENGTH 240

typedef enum{
  MAX30003_FIFO_FULL_EVT,
  MAX30003_FIFO_OVF_EVT,
  MAX30003_RR_EVT,
  MAX30003_LDOFF_EVT, // lead off detection
  MAX30003_LDON_EVT,
  MAX30003_OTHER_EVT
}max30003_int_evt_type_t;

typedef enum{
  MAX30003_RAW_DATA_MODE,
  MAX30003_HR_RR_MODE
}max30003_data_mode_t;

typedef struct{
  max30003_data_mode_t data_mode;
}max30003_status_t;

typedef struct{
  uint16_t hr;
  uint16_t rr;
}max30003_hr_rr_data_t;

typedef struct{
  uint8_t buf[RAW_ECG_PACKET_LENGTH];
  uint8_t cnt;
}max30003_ecg_data_packet;

extern max30003_status_t max30003_status;

void max30003_init(void);

void max30003_loop(void);

void max30003_stop_sampling(void);

void max30003_start_sampling(void);

void max30003_sw_reset(void);

void max30003_synch(void);

void max30003_set_mode(max30003_data_mode_t mode);

void max30003_read_info(void);

void max30003_sleep(void);

uint16_t max30003_get_rr(void);

uint16_t max30003_get_hr(void);

void max30003_fifo_rst(void);

uint32_t max30003_read_status(void);

#endif