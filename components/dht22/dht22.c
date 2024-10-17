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

static bool check_response( uint8_t duration, bool state){
    uint8_t loop = 0;
    while(gpio_get_level(dht22_pin) != state){
        if(loop > duration){return 0;}
        loop++;
        delay_us(1);
    }
    return 1;
}

void dht22_read(float *temp, float *hum){

    uint8_t raw_data[5];

    //starting
    gpio_set_direction(dht22_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dht22_pin, 0);
    delay_us(1500);
    gpio_set_level(dht22_pin, 1);
    delay_us(30);

    //check connection
    gpio_set_direction(dht22_pin, GPIO_MODE_INPUT);
    if(check_response(85, 1) == 1)
    
}


