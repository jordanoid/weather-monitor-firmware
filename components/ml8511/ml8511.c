#include "esp_log.h"
#include "ml8511.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

adc_oneshot_unit_handle_t handle;
adc_cali_handle_t cali_handle;
adc_channel_t sensor_channel;


void ml8511_init (adc_unit_t unit, adc_channel_t channel){

    sensor_channel = channel;

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(handle, channel, &config));

    adc_cali_line_fitting_config_t cali_config = {
    .unit_id = unit,
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle));
};

void get_uv_intensity(float *data) {
    int adc_val;
    int voltage;
    int total_voltage = 0;
    int voltage_buffer[30];

    for (int i = 0; i < 30; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(handle, sensor_channel, &adc_val));
        adc_cali_raw_to_voltage(cali_handle, adc_val, &voltage);
        
        if (voltage < 990) {
            voltage = 990;
        } else if (voltage > 2800) {
            voltage = 2800;
        }

        voltage_buffer[i] = voltage;
    }

    for (int i = 0; i < 30; i++) {
        total_voltage += voltage_buffer[i];
    }
    voltage = total_voltage / 30;

    *data = (((float)voltage - 990) / (2800 - 990)) * 15;
}