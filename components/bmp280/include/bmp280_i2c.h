#ifndef __BMP280_I2C_H__
#define __BMP280_I2C_H__

#include "driver/i2c_master.h"

#define BMP280_DEFAULT_CHIP_ID    0X58
#define BMP280_RESET_VALUE        0XB6

#define SEA_LEVEL_PRESSURE        101325

#define ID_ADDRESS                0XD0
#define RESET_ADDRESS             0XE0
#define STATUS_ADDRESS            0XF3
#define CTRL_MEAS_ADDRESS         0XF4
#define CONFIG_ADDRESS            0XF5
#define PRESSURE_MSB              0xF7
#define PRESSURE_LSB              0xF8
#define PRESSURE_XLSB             0xF9
#define TEMP_MSB                  0xFA
#define TEMP_LSB                  0xFB
#define TEMP_XLSB                 0xFC

#define IM_UPDATE_BIT             0X00
#define MEASURING_BIT             0X01
#define MODE_BIT                  0X00
#define OSRS_P_BIT                0X02
#define OSRS_T_BIT                0X05
#define SPI3W_EN_BIT              0X00
#define FILTER_BIT                0X02
#define T_SB_BIT                  0X05

#define IM_UPDATE_LENGTH          0X01
#define MEASURING_LENGTH          0X03
#define MODE_LENGTH               0X02
#define OSRS_P_LENGTH             0X03
#define OSRS_T_LENGTH             0X03
#define SPI3W_EN_LENGTH           0X01
#define FILTER_LENGTH             0X03
#define T_SB_LENGTH               0X03

#define T1_ADDRESS                0X88
#define T2_ADDRESS                0X8A
#define T3_ADDRESS                0X8C
#define P1_ADDRESS                0X8E
#define P2_ADDRESS                0X90
#define P3_ADDRESS                0X92
#define P4_ADDRESS                0X94
#define P5_ADDRESS                0X96
#define P6_ADDRESS                0X98
#define P7_ADDRESS                0X9A
#define P8_ADDRESS                0X9C
#define P9_ADDRESS                0X9E

void i2c_master_init(uint8_t SCL_IO_PIN, uint8_t SDA_IO_PIN);

void bmp280_i2c_init(void);

#endif
