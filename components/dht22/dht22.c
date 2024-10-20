#include <stdio.h>
#include "dht22.h"
#include "driver/gpio.h"
#include "esp_log.h"

gpio_num_t dht22_pin;
static const char TAG[] = "DHT22";

void dht22_set_pin (gpio_num_t gpio_pin){
    dht22_pin = gpio_pin;
}

static bool check_response( uint8_t duration, bool state){
    uint8_t loop = 0;
    while(gpio_get_level(dht22_pin) == state){
        if(loop > duration){
            return 0;
        }
        loop += 1;
        esp_rom_delay_us(1);
    }
    ESP_LOGI(TAG, "success response in %d ms and state %d", loop, gpio_get_level(dht22_pin));
    return 1;
}

dht_err_t dht22_read(float *temp, float *hum){

    uint8_t raw_data[5] = {0, 0, 0, 0, 0};

    //starting
    gpio_set_direction(dht22_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dht22_pin, 0);
    esp_rom_delay_us(2000);
    gpio_set_level(dht22_pin, 1);
    esp_rom_delay_us(40);

    //check connection
    gpio_set_direction(dht22_pin, GPIO_MODE_INPUT);

    ESP_LOGI(TAG, "after set to input, state : %d", gpio_get_level(dht22_pin));

    if(!check_response(85, 0)){
        *temp = -1;
        *hum = -1;
        ESP_LOGI(TAG,"TIMEOUT 1");
        return DHT_ERR_TIMEOUT;
    }

    if(!check_response(85, 1)){
        *temp = -1;
        *hum = -1;
        ESP_LOGI(TAG,"TIMEOUT 2");
        return DHT_ERR_TIMEOUT;
    }

    //start reading data
    for(int i = 0; i < 5; i++){
        for(int j = 7; j >= 0; j--){
            if(!check_response(55, 0)){
                *temp = -1;
                *hum = -1;
                ESP_LOGI(TAG,"TIMEOUT 3");
                return DHT_ERR_TIMEOUT;
            }
            
            if(!check_response(35, 1)){
                raw_data[i] |= (1<<j);
            }
            while(gpio_get_level(dht22_pin));
        }
    }

    *hum = (float)((raw_data[0] << 8) | raw_data[1])/10;
    *temp = (float)((raw_data[2] << 8) | raw_data[3])/10;

     if(raw_data[4] == raw_data[0] + raw_data[1] + raw_data[2] + raw_data[3]){
        return DHT_OK;
     }else{
        return DHT_ERR_CHECKSUM;
     }
}
