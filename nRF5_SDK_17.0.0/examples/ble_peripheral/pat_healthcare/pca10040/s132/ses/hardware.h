#ifndef HARDWARE_H
#define HARDWARE_H

/*PCB Pins */
#define M1  11
#define M2  12
#define M3  13
#define M4  14
#define M5  15

/* I2C */
#define GEN_SDA           22
#define GEN_SCL           23
#define GEN_I2C_INSTANCE  1

/* GENERAL SPI */
#define GEN_SPI_SCK       M4
#define GEN_SPI_MOSI      M3
#define GEN_SPI_MISO      M2
#define GEN_SPI_INSTANCE  0

/* MAX30003 */
#define MAX30003_CS       M5
#define MAX30003_INT1     M1
#define MAX30003_FCLK     9
#define MAX30003_CLK_TIMER  1

/* MLX90632 */

/* SD */
#define SD_SPI_SCK  1
#define SD_SPI_MOSI 1
#define SD_SPI_MISO 1
#define SD_SPI_CS   1

/* CHARGING CIRCUIT */
#define CHG_STAT      3
#define CHG_POW       4

/* RTC */
#define RTC_RESET 1
#define RTC_CE    1
#define RTC_INT0  1

/* ACCEL */
#define ACCEL_INT0 1

/* PUSH BUTTON */
#define PUSH_BUTTON   8

#endif