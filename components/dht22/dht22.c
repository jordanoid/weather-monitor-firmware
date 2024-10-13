#include <stdio.h>
#include "dht22.h"
#include "driver/gpio.h"
#include "esp_timer.h"

gpio_num_t dht22_pin;

static void delay_us(uint32_t delay){
    //get init time
    uint64_t init_time = (uint64_t)esp_timer_get_time();
    //loop while still in delay range
    while(delay >= ((uint64_t)esp_timer_get_time() - init_time));
}

void dht22_set_pin (gpio_num_t gpio_pin){
    dht22_pin = gpio_pin;
}

void dht22_read(float *temp, float *hum){

    uint8_t raw_data[5];

    //starting
    gpio_set_direction(dht22_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dht22_pin, 0);
    delay_us(2000);
    gpio_set_level(dht22_pin, 1);
    delay_us(30);

    //check connection
    

}

