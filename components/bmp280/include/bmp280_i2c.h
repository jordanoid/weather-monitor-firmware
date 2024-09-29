#ifndef __BMP280_I2C_H__
#define __BMP280_I2C_H__

#include "driver/i2c_master.h"

void i2c_master_init(uint8_t SCL_IO_PIN, uint8_t SDA_IO_PIN);

void bmp280_i2c_init(void);

#endif
