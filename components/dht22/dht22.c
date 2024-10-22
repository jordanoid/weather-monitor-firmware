#include <stdio.h>
#include "dht22.h"
#include "driver/gpio.h"
#include "esp_log.h"

gpio_num_t dht22_pin;

void dht22_set_pin(gpio_num_t gpio_pin) {
    dht22_pin = gpio_pin;
    gpio_reset_pin(dht22_pin);
    // gpio_set_direction(dht22_pin, GPIO_MODE_OUTPUT);
    // gpio_set_level(dht22_pin, 1);
}

// Function to read sensor response with proper timing adjustments
static bool check_response(uint32_t duration, bool state) {
    uint32_t loop = 0;
    while (gpio_get_level(dht22_pin) == state) {
        if (loop > duration) {
            return 0;
        }
        loop++;
        esp_rom_delay_us(1);
    } 
    return 1;
}

dht_err_t dht22_read(float *temp, float *hum) {
    uint8_t raw_data[5] = {0, 0, 0, 0, 0};

    // Send start signal
    gpio_set_direction(dht22_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dht22_pin, 0);
    esp_rom_delay_us(20000);
    gpio_set_level(dht22_pin, 1);
    esp_rom_delay_us(35);

    gpio_set_direction(dht22_pin, GPIO_MODE_INPUT);

    // Check response
    if (!check_response(100, 0)) {
        *temp = -1;
        *hum = -1;
        return DHT_ERR_TIMEOUT;
    }

    if (!check_response(100, 1)) {
        *temp = -1;
        *hum = -1;
        return DHT_ERR_TIMEOUT;
    }

    // Read data
    for (int i = 0; i < 5; i++) {
        for (int j = 7; j >= 0; j--) {
            if (!check_response(70, 0)) {
                *temp = -1;
                *hum = -1;
                return DHT_ERR_TIMEOUT;
            }

            uint32_t pulse_length = 0;
            while (gpio_get_level(dht22_pin) == 1) {
                pulse_length++;
                esp_rom_delay_us(1);
                if (pulse_length > 100) {
                    return DHT_ERR_TIMEOUT;
                }
            }

            if (pulse_length > 30) {
                raw_data[i] |= (1 << j);  // Set bit to 1
            }
        }
    }

    *hum = (float)((raw_data[0] << 8) | raw_data[1]) / 10;
    *temp = (float)((raw_data[2] << 8) | raw_data[3]) / 10;

    // Checksum
    uint8_t checksum = raw_data[0] + raw_data[1] + raw_data[2] + raw_data[3];
    if (raw_data[4] == checksum) {
        return DHT_OK;
    } else {
        return DHT_ERR_CHECKSUM;
    }
}
