#include "esp_log.h"
#include "ml8511.h"

static const char TAG[] = "weather monitor";

adc_oneshot_unit_handle_t handle;
adc_cali_handle_t cali_handle;

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

    adc_cali_line_fitting_config_t cali_config = {
    .unit_id = unit,
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle));
};

void get_uv_intensity(adc_channel_t channel, float *data){
    int adc_val;
    int voltage; //in mV
    char buffer[15];
    ESP_ERROR_CHECK(adc_oneshot_read(handle, channel, &adc_val));

    //Logger
    sprintf(buffer, "ADC Val: %d", adc_val);
    esp_log_buffer_char(TAG, buffer, sizeof(buffer)/sizeof(char));


   adc_cali_raw_to_voltage(cali_handle, adc_val, &voltage);


    //Logger
    sprintf(buffer, "Voltage: %d", voltage);
    esp_log_buffer_char(TAG, buffer, sizeof(buffer)/sizeof(char));

    

    if( voltage == 0){
        *data = -1;
    }else{
        if(voltage < 990){
            voltage = 990;
        }else if(voltage > 2800){
            voltage = 2800;
        }
        *data = (((float)voltage - 990) / (2800 - 990)) *  15; 
    }
};