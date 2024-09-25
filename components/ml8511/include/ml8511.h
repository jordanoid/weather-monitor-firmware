
#ifndef __ML8511_H__
#define __ML8511_H__

#include "hal/adc_types.h"

void ml8511_init (adc_unit_t unit, adc_channel_t channel);
void get_uv_intensity(adc_channel_t channel, int *data);
#endif