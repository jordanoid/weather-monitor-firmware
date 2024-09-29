
#ifndef __ML8511_H__
#define __ML8511_H__

#include "esp_adc/adc_oneshot.h"

void ml8511_init (adc_unit_t unit, adc_channel_t channel);
void get_uv_intensity(adc_channel_t channel, float *data);
#endif