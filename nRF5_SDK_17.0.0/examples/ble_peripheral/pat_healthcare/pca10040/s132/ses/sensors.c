#define LOG_HEAD "Sensors"
#include "log.h"
#include "hardware.h"
#include "sensors.h"
#include "max30003.h"

#include "nrf_drv_spi.h"
#include "nrfx_twi.h"

const nrfx_twi_t gen_twi = NRFX_TWI_INSTANCE(GEN_I2C_INSTANCE);
const nrf_drv_spi_t gen_spi = NRF_DRV_SPI_INSTANCE(GEN_SPI_INSTANCE); 

static void sensors_gen_spi_init(void){                                                                     
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin    = NRF_DRV_SPI_PIN_NOT_USED;
  spi_config.mosi_pin  = GEN_SPI_MOSI;
  spi_config.miso_pin  = GEN_SPI_MISO;
  spi_config.sck_pin   = GEN_SPI_SCK;
  spi_config.frequency = NRF_DRV_SPI_FREQ_2M;
  spi_config.mode      = NRF_DRV_SPI_MODE_0;
  APP_ERROR_CHECK(nrf_drv_spi_init(&gen_spi, &spi_config, NULL, NULL));
}

static void sensors_gen_i2c_init(void){
  const nrfx_twi_config_t twi_mpu_config = {
       .scl                = GEN_SDA,
       .sda                = GEN_SDA,
       .frequency          = NRF_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .hold_bus_uninit     = false
  };
  APP_ERROR_CHECK(nrfx_twi_init(&gen_twi, &twi_mpu_config, NULL, NULL));    
  nrfx_twi_enable(&gen_twi);
}

void sensors_init(void){
  sensors_gen_spi_init();
  sensors_gen_i2c_init();

  max30003_init();
  LOGD("Init Complete");
  max30003_start_sampling();
}

void sensors_start_sampling(void){
  max30003_start_sampling();
  LOGD("Sampling started");
}

void sensors_stop_sampling(void){
  max30003_stop_sampling();
  LOGD("Sampling stopped");
}