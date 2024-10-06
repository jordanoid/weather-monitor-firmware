#include <stdio.h>
#include "esp_log.h"
#include "ml8511.h"
#include "bmp280_i2c.h"
#include "dht22.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

bmp280_config_t cfg;
bmp280_ctrl_meas_t ctrl_meas;
ml8511_channel_t adc_channel;

float uvValue, temp, pressure;


static const char TAG[] = "weather monitor";

static void bmp280_task(void *pvParameters){
    cfg.t_sdby = STANDBY_1000M;
    cfg.filter = IIR_NONE;

    ctrl_meas.t_oversampling = OVERSAMPLING_X1;
    ctrl_meas.p_oversampling = OVERSAMPLING_X1;
    ctrl_meas.mode = FORCED_MODE;

    i2c_master_init(22, 21);
    bmp280_i2c_init(&cfg, &ctrl_meas);

    char buffer[15];

    for(;;){

        bmp280_get_temp(&temp);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        bmp280_get_pressure(&pressure);
        sprintf(buffer, "Temp: %.2f", temp);
        esp_log_buffer_char(TAG, buffer, sizeof(buffer)/sizeof(char));
        sprintf(buffer, "Press: %.2f", pressure/100);
        esp_log_buffer_char(TAG, buffer, sizeof(buffer)/sizeof(char));

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void ml8511_task(void *pvParameters){
    char buffer[15];
    ml8511_init(ADC_UNIT_1, ADC_CHANNEL_7);

    for(;;){

        get_uv_intensity(&uvValue);
        sprintf(buffer, "UV: %.6f", uvValue);
        esp_log_buffer_char(TAG, buffer, sizeof(buffer)/sizeof(char));

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Test Main");
    xTaskCreatePinnedToCore(ml8511_task, "UV Sensor Read", 4096, NULL, 0, NULL, 0);
    xTaskCreatePinnedToCore(bmp280_task, "BMP280 Read", 4096, NULL, 0, NULL, 0);
}
