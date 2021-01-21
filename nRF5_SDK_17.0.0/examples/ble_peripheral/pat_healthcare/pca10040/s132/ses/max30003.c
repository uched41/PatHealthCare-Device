#define LOG_HEAD "ECG"
#include "log.h"

#include "max30003_reg.h"
#include "max30003.h"
#include "hardware.h"
#include "ble_transfer_queue.h"

#include "string.h"
#include "app_error.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"

#define MAX30003_CS_HIGH() nrf_drv_gpiote_out_set(MAX30003_CS)
#define MAX30003_CS_LOW()  nrf_drv_gpiote_out_clear(MAX30003_CS)

extern const nrf_drv_spi_t    gen_spi;
static nrf_ppi_channel_t      ppi_channel;
static const nrf_drv_timer_t  clk_timer = NRF_DRV_TIMER_INSTANCE(MAX30003_CLK_TIMER);

static max30003_ecg_data_packet ecg_data;

/* Write MAX30003 Register */
static void max30003_write_reg(uint8_t reg, uint32_t data){
  uint8_t buf[4];
  buf[0] = (reg << 1) | WREG;
  buf[1] = data >> 16;
  buf[2] = data >> 8;
  buf[3] = data;

  MAX30003_CS_LOW();
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&gen_spi, buf, 4, NULL, 0));
  MAX30003_CS_HIGH();
}

/* Read MAX30003 Register */
static void max30003_read_reg(uint8_t reg, uint32_t* data){
  uint8_t in_buf = (reg << 1) | RREG;;
  uint8_t out_buf[4];

  MAX30003_CS_LOW();
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&gen_spi, &in_buf, 1, out_buf, 4+1));
  MAX30003_CS_HIGH();

  *data = (out_buf[1] << 16) | (out_buf[2] << 8) | (out_buf[3]);
}

void timer_handler(nrf_timer_event_t event_type, void * p_context){}

static void max30003_fclck_setup(void){
  uint32_t err_code;

  /* Configure GPIO for toggling task */
  nrfx_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_TASK_TOGGLE(true);
  APP_ERROR_CHECK(nrfx_gpiote_out_init(MAX30003_FCLK, &out_config));
  nrfx_gpiote_out_task_enable(MAX30003_FCLK);

  /* Initialize PPI Peripheral */
  APP_ERROR_CHECK(nrf_drv_ppi_init());

  nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
  timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
  err_code = nrf_drv_timer_init(&clk_timer, &timer_cfg, timer_handler);
  APP_ERROR_CHECK(err_code);

  uint32_t ticks = nrf_drv_timer_us_to_ticks(&clk_timer, 31);

  nrf_drv_timer_extended_compare(&clk_timer, NRF_TIMER_CC_CHANNEL0, 245, 
        NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);
  nrf_drv_timer_enable(&clk_timer);

  uint32_t timer_compare_event_addr = 
        nrf_drv_timer_compare_event_address_get(&clk_timer, NRF_TIMER_CC_CHANNEL0);

  /* Initialize PPI Channel */
  APP_ERROR_CHECK(nrf_drv_ppi_channel_alloc(&ppi_channel));

  APP_ERROR_CHECK(nrf_drv_ppi_channel_assign(ppi_channel, timer_compare_event_addr, nrfx_gpiote_out_task_addr_get(MAX30003_FCLK)));

  APP_ERROR_CHECK(nrf_drv_ppi_channel_enable(ppi_channel));

  LOGD("32KHz Clock Initialized");
}


/* MAX30003 Interrupt handler */
void max30003_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
  uint32_t val =  max30003_read_status();
  //LOGD("INT 0x%X", val);

  if(val & _ECG3_RRINT_MASK){
    LOGD("RR INT");
  }
  if(val & _ECG3_EINT_MASK){
    max30003_read_fifo();
    //LOGD("FIFO FULL INT");
  }
  if(val & _ECG3_EOVF_MASK){
    max30003_fifo_rst();
    LOGD("FIFO OVF INT");
  }
  if(val & _ECG3_DCLOFF_INT_MASK){
    LOGD("LDOFF INT");
  }

}


/* MAX30003 Register initialization */
void max30003_reg_init(void){
  max30003_sw_reset();
  nrf_delay_ms(100);
 
  max30003_write_reg( _ECG3_CNFG_GEN_REG, 
    _ECG3_FMSTR_32768HZ_ECG_512HZ | _ECG3_ECG_CHANN_EN | _ECG3_DCLOFF_EN | 
    _ECG3_RBIAS_100M_OHM | _ECG3_RBIASP_EN | _ECG3_RBIASN_EN 
  );

  max30003_write_reg( _ECG3_CNFG_CAL_REG, 
    _ECG3_VCAL_EN | _ECG3_VMODE_BIPOL | _ECG3_VMAG_500MICROV 
  );

  max30003_write_reg( _ECG3_CNFG_EMUX_REG, 
    _ECG3_ECGP_EN | _ECG3_ECGN_EN | _ECG3_ECGP_CAL_VCALP | _ECG3_ECGN_CAL_VCALN 
  );

  max30003_write_reg( _ECG3_CNFG_ECG_REG, 
    0x000000 | _ECG3_DHPF_500MILIHZ | _ECG3_DLPF_40HZ   // 0x8->128 sps, 0x4->256, 0x00->512
  );

  max30003_write_reg( _ECG3_CNFG_RTOR1_REG,             // RTOR Configuration
    _ECG3_WNDW_12 | _ECG3_RRGAIN_AUTO_SCALE | _ECG3_RTOR_EN | _ECG3_PAVG_8 | 0x000600 
  );
}


/* MAX30003 Interrupt Initialization */
void max30003_int_enable(void){
  /* Note: FIFO Interrupt generated when 16 records are present 
           This value can be configured in MNGR_INT REG */ 
  max30003_write_reg( _ECG3_EN_INT_REG, 
    _ECG3_EINT_MASK | _ECG3_RRINT_MASK | _ECG3_DCLOFF_INT_MASK | _ECG3_LONINT_MASK |
     _ECG3_INTB_CMOS 
  );
}


/* Initialize MAX30003 Sensor */
void max30003_init(void){
  /* CS Pin */
  nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
  APP_ERROR_CHECK(nrf_drv_gpiote_out_init(MAX30003_CS, &out_config));

  /* Initialize Int0 */
  nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
  in_config.pull = NRF_GPIO_PIN_PULLUP;
  APP_ERROR_CHECK(nrf_drv_gpiote_in_init(MAX30003_INT1, &in_config, max30003_cb));

  max30003_fclck_setup();
  //APP_ERROR_CHECK(nrf_drv_gpiote_out_init(MAX30003_FCLK, &out_config));


  /* Reg Init */
  max30003_reg_init();
  max30003_read_info();
  max30003_int_enable();
  max30003_synch();

  /* Clear Status and FIFO Reg */
  uint32_t val;
  max30003_read_reg(_ECG3_STAT_REG , &val);
  max30003_fifo_rst();
  nrf_delay_ms(100);

  LOGD("Init complete.");
}


/* Enable interrupts to start sampling */
void max30003_start_sampling(void){
  nrf_drv_gpiote_in_event_enable(MAX30003_INT1, true);

  /* Clear Status and FIFO Reg */
  uint32_t val;
  max30003_read_reg(_ECG3_STAT_REG , &val);
  max30003_fifo_rst();

  ecg_data.cnt = 0;
  LOGD("Sampling started");
}


/* Disable interrupts to stop sampling */
void max30003_stop_sampling(void){
  nrf_drv_gpiote_in_event_disable(MAX30003_INT1);
  LOGD("Sampling stopped");
}


/* Read MAX30003 information */
void max30003_read_info(void){
  uint32_t val;
  max30003_read_reg(_ECG3_INFO_REG , &val);
  LOGD("0x%X", val);
} 


/* Set MAX30003 Operation Mode*/
void max30003_set_mode(max30003_data_mode_t mode){
  max30003_status.data_mode = mode;
}


/* Put MAX30003 Sensor in low-power sleep mode */
void max30003_sleep(void){}


/* Wake MAX30003 Sensor from sleep */
void max30003_wakeup(void){}


/* Get HR Data */
uint16_t max30003_get_hr(void){
  uint32_t rrTemp;
  float res;

  max30003_read_reg( _ECG3_RTOR_REG, &rrTemp );
  rrTemp >>= 10;
  res = 60 / ((float)rrTemp * 0.0078125);
  return (uint16_t) res;
}


/* Get RR Data */
uint16_t max30003_get_rr(void){
  /* Note: RR Internval resolution is 7.8125.
           This is determined by the selected MSTR. */
  uint32_t rrTemp;
  float res;

  max30003_read_reg( _ECG3_RTOR_REG, &rrTemp );
  rrTemp >>= 10;
  res = (uint16_t)rrTemp * 7.8125;
  return (uint16_t) res;
}


/* Get RR Data */
max30003_hr_rr_data_t max30003_get_hr_rr(void){
  uint32_t rrTemp;
  float res;

  max30003_hr_rr_data_t data;

  max30003_read_reg( _ECG3_RTOR_REG, &rrTemp );
  rrTemp >>= 10;
  res = 60 / ((float)rrTemp * 0.0078125);
  data.hr = res;

  res = (uint16_t)rrTemp * 7.8125;
  data.rr = res;
  return data;
}


/* Get Interrupt Event type */
uint32_t max30003_read_status(void){
  uint32_t val;
  max30003_read_reg(_ECG3_STAT_REG , &val);
  return val;
}


/* Software reset */
void max30003_sw_reset(void){
  max30003_write_reg(_ECG3_SW_RST_REG,0x000000);
  nrf_delay_ms(100);
}


/* Synchronize MAX30003 */
void max30003_synch(void){
  max30003_write_reg(_ECG3_SYNC_REG,0x000000);
}


/* MAX30003 Reset FIFO */
void max30003_fifo_rst(void){
  max30003_write_reg(_ECG3_FIFO_RST_REG,0x000000);
}


/* Read FIFO Data */
void max30003_read_fifo(void){
  /* FIFO Interrupt is sent when there are 16 unread samples */
  uint8_t in_buf = (_ECG3_ECG_FIFO_BURST_REG << 1) | RREG;
  uint8_t out_buf[51+1];

  MAX30003_CS_LOW();
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&gen_spi, &in_buf, 1, out_buf, 51+1));
  MAX30003_CS_HIGH();

  if(ecg_data.cnt < RAW_ECG_PACKET_LENGTH){
    memcpy(&ecg_data.buf[ecg_data.cnt], &out_buf[1], 48);
    ecg_data.cnt += 48;

    if(ecg_data.cnt >= RAW_ECG_PACKET_LENGTH){  // Packet filled
      ble_add_to_queue(ecg_data.buf, ecg_data.cnt, BLE_QUEUE_DATA_PRIORITY);
      ecg_data.cnt = 0;
    }
  }
  else{
    ecg_data.cnt = 0;
  }

}

void max30003_loop(void){}