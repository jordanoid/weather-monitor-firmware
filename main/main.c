#include <stdio.h>
#include "esp_log.h"
#include "ml8511.h"
#include "bmp280_i2c.h"
#include "dht22.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

bmp280_config_t bmp280_cfg;
bmp280_ctrl_meas_t bmp280_ctrl_meas;

SemaphoreHandle_t xBinarySemaphore;

float uvValue, temp, pressure, dht_temp, dht_rh;

static const char TAG[] = "weather monitor";

static void bmp280_task(void *pvParameters);
static void ml8511_task(void *pvParameters);
static void dht22_task(void *pvParameters);

void app_main(void)
{  
    ESP_LOGI(TAG, "Test Main");
    xBinarySemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(xBinarySemaphore);
    // xTaskCreatePinnedToCore(ml8511_task, "UV Sensor Read", 4096, NULL, 0, NULL, 0);
    // xTaskCreatePinnedToCore(bmp280_task, "BMP280 Read", 4096, NULL, 0, NULL, 0);
    xTaskCreatePinnedToCore(dht22_task, "DHT22 Read", 4096, NULL, 0, NULL, 0);
}

static void bmp280_task(void *pvParameters){

    bmp280_cfg.t_sdby = STANDBY_1000M;
    bmp280_cfg.filter = IIR_NONE;

    bmp280_ctrl_meas.t_oversampling = OVERSAMPLING_X1;
    bmp280_ctrl_meas.p_oversampling = OVERSAMPLING_X1;
    bmp280_ctrl_meas.mode = FORCED_MODE;

    i2c_master_init(22, 21);
    bmp280_i2c_init(&bmp280_cfg, &bmp280_ctrl_meas);

    char buffer[15];

    for(;;){

        if( xSemaphoreTake(xBinarySemaphore, (TickType_t)10) == pdTRUE){
            bmp280_get_temp(&temp);
            bmp280_get_pressure(&pressure);
            xSemaphoreGive(xBinarySemaphore);
        }
        sprintf(buffer, "Temp: %.2f", temp);
        esp_log_buffer_char(TAG, buffer, sizeof(buffer)/sizeof(char));
        sprintf(buffer, "Press: %.2f", pressure/100);
        esp_log_buffer_char(TAG, buffer, sizeof(buffer)/sizeof(char));

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void ml8511_task(void *pvParameters){
    
    ml8511_init(ADC_UNIT_1, ADC_CHANNEL_7);

    char buffer[15];

    for(;;){

        if(xSemaphoreTake(xBinarySemaphore, (TickType_t)10) == pdTRUE ){
            get_uv_intensity(&uvValue);
            xSemaphoreGive(xBinarySemaphore);
        }
        sprintf(buffer, "UV: %.6f", uvValue);
        esp_log_buffer_char(TAG, buffer, sizeof(buffer)/sizeof(char));

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void dht22_task(void *pvParameters){
    dht_err_t ret = 0;

    dht22_set_pin(GPIO_NUM_13);
  
    char buffer[25];  // Increased size to fit longer messages

    for(;;){
        ret = dht22_read(&dht_temp, &dht_rh);  // No semaphore used for DHT22

        if (ret == DHT_OK) {
            sprintf(buffer, "T: %.2f C, RH: %.2f", dht_temp, dht_rh);
            esp_log_buffer_char(TAG, buffer, sizeof(buffer)/sizeof(char));
        } else if (ret == DHT_ERR_TIMEOUT) {
            sprintf(buffer, "DHT ERR TIMEOUT");
            esp_log_buffer_char(TAG, buffer, sizeof(buffer)/sizeof(char));
        } else if (ret == DHT_ERR_CHECKSUM) {
            sprintf(buffer, "DHT ERR CHECKSUM");
            esp_log_buffer_char(TAG, buffer, sizeof(buffer)/sizeof(char));
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Delay between reads
    }
}