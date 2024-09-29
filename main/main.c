#include <stdio.h>
#include "esp_log.h"
#include "ml8511.h"
#include "bmp280.h"
#include "dht22.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

void app_main(void)
{
    ESP_LOGI(TAG, "Test Main");
    xTaskCreatePinnedToCore(readUV, "UV Sensor Read", 4096, NULL, 0, NULL, 0);
}
