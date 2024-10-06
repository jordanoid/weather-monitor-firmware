#include <stdio.h>
#include "bmp280_i2c.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

i2c_master_bus_handle_t master_handle;
i2c_master_dev_handle_t bmp280_handle;
bmp280_calib_t calib_data;

void i2c_master_init(uint8_t SCL_IO_PIN, uint8_t SDA_IO_PIN){
    i2c_master_bus_config_t i2c_master_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = -1,
    .scl_io_num = SCL_IO_PIN,
    .sda_io_num = SDA_IO_PIN,
    .glitch_ignore_cnt = 7,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_config, &master_handle));
}

static void bmp280_get_cmpst(bmp280_calib_t *data){

    uint8_t reg = CALIB_LO_REG;
    uint8_t data_rd[26];

    ESP_ERROR_CHECK(i2c_master_transmit_receive(bmp280_handle, &reg, sizeof(reg), data_rd, sizeof(data_rd), 1000));

    data->T1 = data_rd[0] | (data_rd[1] << 8);
    data->T2 = data_rd[2] | (data_rd[3] << 8);
    data->T3 = data_rd[4] | (data_rd[5] << 8);
    data->P1 = data_rd[6] | (data_rd[7] << 8);
    data->P2 = data_rd[8] | (data_rd[9] << 8);
    data->P3 = data_rd[10] | (data_rd[11] << 8);
    data->P4 = data_rd[12] | (data_rd[13] << 8);
    data->P5 = data_rd[14] | (data_rd[15] << 8);
    data->P6 = data_rd[16] | (data_rd[17] << 8);
    data->P7 = data_rd[18] | (data_rd[19] << 8);
    data->P8 = data_rd[20] | (data_rd[21] << 8);
    data->P9 = data_rd[22] | (data_rd[23] << 8);
}

void bmp280_i2c_init(bmp280_config_t *cfg, bmp280_ctrl_meas_t *ctrl){

    uint8_t config_data[2] = {CONFIG_ADDRESS, ((cfg->t_sdby << 5) | (cfg->filter << 2))};
    uint8_t ctrl_data[2] = {CTRL_MEAS_ADDRESS, ((ctrl->t_oversampling << 5) | (ctrl->t_oversampling << 2) | (ctrl->mode))}; 
    uint8_t reset_data[2] ={RESET_ADDRESS, RESET_VALUE};

    i2c_device_config_t bmp280_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x76,
        .scl_speed_hz = 200000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(master_handle, &bmp280_cfg, &bmp280_handle));

    ESP_ERROR_CHECK(i2c_master_transmit(bmp280_handle, reset_data, sizeof(reset_data), 1000));
    ESP_ERROR_CHECK(i2c_master_transmit(bmp280_handle, config_data, sizeof(config_data), 1000));
    ESP_ERROR_CHECK(i2c_master_transmit(bmp280_handle, ctrl_data, sizeof(ctrl_data), 1000));

    bmp280_get_cmpst(&calib_data);
}

int32_t t_fine;

static int32_t t_compensate(int32_t adc_t){
    int32_t var1, var2, T;
    var1 = ((((adc_t >> 3) - ((int32_t)calib_data.T1 << 1))) * ((int32_t)calib_data.T2)) >> 11;
    var2 = (((((adc_t >> 4) - ((int32_t)calib_data.T1)) * ((adc_t >> 4) - ((int32_t)calib_data.T1))) >> 12) * ((int32_t)calib_data.T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

static uint32_t p_compensate(int32_t adc_p){
    int64_t var1, var2, P; 
    var1 = (int64_t)t_fine - 128000; 
    var2 = var1 * var1 * (int64_t)calib_data.P6; 
    var2 = var2 + ((var1 * (int64_t)calib_data.P5) << 17); 
    var2 = var2 + (((int64_t)calib_data.P4) << 35); 
    var1 = ((var1 * var1 * (int64_t)calib_data.P3) >> 8) + ((var1 * (int64_t)calib_data.P2) << 12); 
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib_data.P1) >> 33; 
    if (var1 == 0) { 
        return 0;
    }
    P = 1048576 - adc_p; 
    P = (((P << 31) - var2) * 3125) / var1; 
    var1 = (((int64_t)calib_data.P9) * (P >> 13) * (P >> 13)) >> 25; 
    var2 = (((int64_t)calib_data.P8) * P) >> 19; 
    P = ((P + var1 + var2) >> 8) + (((int64_t)calib_data.P7) << 4); 
    return (uint32_t)P;
}

void bmp280_get_temp(float *temp){
    uint8_t buffer[3];
    uint8_t reg = TEMP_MSB;
    
    i2c_master_transmit_receive(bmp280_handle, &reg, sizeof(reg), buffer, sizeof(buffer), 1000);
    *temp = (float)t_compensate((buffer[0] << 12) | (buffer[1] << 4)| (buffer[2] >> 4))/100;
}

void bmp280_get_pressure(float *pressure){
    uint8_t buffer[3];
    uint8_t reg = PRESSURE_MSB;
    
    i2c_master_transmit_receive(bmp280_handle, &reg, sizeof(reg), buffer, sizeof(buffer), 1000);
    *pressure = (float)p_compensate((buffer[0] << 12) | (buffer[1] << 4) | (buffer[2] >> 4))/256;
}
