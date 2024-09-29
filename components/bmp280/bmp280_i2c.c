#include <stdio.h>
#include "bmp280_i2c.h"

i2c_master_bus_handle_t master_handle;

void i2c_master_init(uint8_t SCL_IO_PIN, uint8_t SDA_IO_PIN){
    i2c_master_bus_config_t i2c_master_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = -1,
    .scl_io_num = SCL_IO_PIN,
    .sda_io_num = SDA_IO_PIN,
    .glitch_ignore_cnt = 7,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_config, &master_handle));
};

void bmp280_i2c_init(){
    i2c_device_config_t bmp280_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x76,
        .scl_speed_hz = 200000,
    };
};