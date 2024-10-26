#include <stdio.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "ml8511.h"
#include "bmp280_i2c.h"
#include "dht22.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "mqtt_client.h"
#include "cJSON.h"

bmp280_config_t bmp280_cfg;
bmp280_ctrl_meas_t bmp280_ctrl_meas;

SemaphoreHandle_t xBinarySemaphore;
esp_mqtt_client_handle_t mqtt_client;

float uvValue, bmp280_temp, bmp280_pressure, dht_temp, dht_rh;

static const char TAG[] = "weather monitor";

static void wifi_init_sta(void);
static void mqtt_client_init(void);
static void mqtt_pub_task(void *pvParameters);
static void bmp280_task(void *pvParameters);
static void ml8511_task(void *pvParameters);
static void dht22_task(void *pvParameters);

void app_main(void)
{  
    ESP_LOGI(TAG, "Test Main");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    wifi_init_sta();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    mqtt_client_init();


    xBinarySemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(xBinarySemaphore);
    xTaskCreatePinnedToCore(ml8511_task, "UV Sensor Read", 4096, NULL, 0, NULL, 0);
    xTaskCreatePinnedToCore(bmp280_task, "BMP280 Read", 4096, NULL, 0, NULL, 0);
    xTaskCreatePinnedToCore(dht22_task, "DHT22 Read", 4096, NULL, 0, NULL, 0);

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(mqtt_pub_task, "MQTT Pub", 4096, NULL, 0, NULL, 1);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Retrying Wi-Fi connection...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Connected with IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

static void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void mqtt_client_init(void){

    const esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri = "mqtt://jordanoid-iot-server"
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_start(mqtt_client);
}

static void mqtt_pub_task(void *pvParameters){
    for(;;){
         cJSON *json = cJSON_CreateObject();
        cJSON_AddNumberToObject(json, "uv", (double)uvValue);
        cJSON_AddNumberToObject(json, "bmp280_press", (double)bmp280_pressure / 100);
        cJSON_AddNumberToObject(json, "bmp280_temp", (double)bmp280_temp);
        cJSON_AddNumberToObject(json, "dht22_rh", (double)dht_rh);
        cJSON_AddNumberToObject(json, "dht22_temp", (double)dht_temp);

        char *json_string = cJSON_Print(json);
        esp_mqtt_client_publish(mqtt_client, "weather/sensor_readings", json_string, 0, 1, 1);
        ESP_LOGI(TAG, "Published : %s", json_string);

        cJSON_Delete(json);
        free(json_string);

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

static void bmp280_task(void *pvParameters){

    bmp280_cfg.t_sdby = STANDBY_1000M;
    bmp280_cfg.filter = IIR_NONE;

    bmp280_ctrl_meas.t_oversampling = OVERSAMPLING_X1;
    bmp280_ctrl_meas.p_oversampling = OVERSAMPLING_X1;
    bmp280_ctrl_meas.mode = FORCED_MODE;

    i2c_master_init(22, 21);
    bmp280_i2c_init(&bmp280_cfg, &bmp280_ctrl_meas);

    for(;;){

        if( xSemaphoreTake(xBinarySemaphore, (TickType_t)10) == pdTRUE){
            bmp280_get_temp(&bmp280_temp);
            bmp280_get_pressure(&bmp280_pressure);
            xSemaphoreGive(xBinarySemaphore);
        }
        ESP_LOGI(TAG, "Temp: %.2f C", bmp280_temp);
        ESP_LOGI(TAG, "Press: %.2f hPa", bmp280_pressure/100);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void ml8511_task(void *pvParameters){
    
    ml8511_init(ADC_UNIT_1, ADC_CHANNEL_7);

    for(;;){

        if(xSemaphoreTake(xBinarySemaphore, (TickType_t)10) == pdTRUE ){
            get_uv_intensity(&uvValue);
            xSemaphoreGive(xBinarySemaphore);
        }
        ESP_LOGI(TAG, "UV: %.3f mW/cm2", uvValue);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void dht22_task(void *pvParameters){
    dht_err_t ret = 0;

    dht22_set_pin(GPIO_NUM_13);


    for(;;){

         if(xSemaphoreTake(xBinarySemaphore, (TickType_t)10) == pdTRUE ){
            ret = dht22_read(&dht_temp, &dht_rh);
            xSemaphoreGive(xBinarySemaphore);
        }
       
        if (ret == DHT_OK) {
            ESP_LOGI(TAG, "T: %.2f C", dht_temp);
            ESP_LOGI(TAG, "RH: %.2f %%", dht_rh);
        } else if (ret == DHT_ERR_TIMEOUT) {
            ESP_LOGI(TAG, "DHT ERR TIMEOUT");
        } else if (ret == DHT_ERR_CHECKSUM) {
            ESP_LOGI(TAG, "DHT ERR CHECKSUM");
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}