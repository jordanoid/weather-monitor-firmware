#include <stdio.h>
#include "esp_log.h"
#include "ml8511.h"
#include "bmp280.h"
#include "dht22.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

float uvValue;
static const char TAG[] = "weather monitor";

void readUV(void *pvParameters){
    char buffer[15];
    ml8511_init(ADC_UNIT_1, ADC_CHANNEL_7);
    for(;;){
        get_uv_intensity(ADC_CHANNEL_7, &uvValue);
        sprintf(buffer, "UV: %.6f", uvValue);
        esp_log_buffer_char(TAG, buffer, sizeof(buffer)/sizeof(char));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}



void app_main(void)
{
    ESP_LOGI(TAG, "Test Main");
    xTaskCreatePinnedToCore(readUV, "UV Sensor Read", 4096, NULL, 0, NULL, 0);
}
