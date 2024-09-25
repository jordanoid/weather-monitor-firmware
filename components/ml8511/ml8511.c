#include "esp_log.h"
#include "ml8511.h"
#include "esp_adc/adc_oneshot.h"

adc_oneshot_unit_handle_t handle;

void ml8511_init (adc_unit_t unit, adc_channel_t channel){

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
};

void get_uv_intensity(adc_channel_t channel, int *data){
    int adc_val;
    float voltage;
    ESP_ERROR_CHECK(adc_oneshot_read(handle, channel, &adc_val));

    voltage = (float)adc_val * 3.3 / 4096;

    if( voltage == 0){
        *data = -1;
    }else{
        if(voltage < 0.99){
        voltage = 0.99;
        }else if(voltage > 2.8){
            voltage = 2.8;
        }
        *data = ((voltage - 0.99) / (2.8 - 0.99)) *  15; 
    }
};