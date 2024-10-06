
#ifndef __ML8511_H__
#define __ML8511_H__

#include "hal/adc_types.h"

typedef adc_channel_t ml8511_channel_t;

void ml8511_init (adc_unit_t unit, adc_channel_t channel);
void get_uv_intensity(float *data);
#endif