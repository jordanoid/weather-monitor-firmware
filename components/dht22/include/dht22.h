#ifndef __DHT22_H__
#define __DHT22_H__

#include "hal/gpio_types.h"

typedef enum {
    DHT_ERR_TIMEOUT,
    DHT_ERR_CHECKSUM,
    DHT_OK
} dht_err_t;

void dht22_set_pin (gpio_num_t gpio_pin);
dht_err_t dht22_read(float *temp, float *hum);

#endif