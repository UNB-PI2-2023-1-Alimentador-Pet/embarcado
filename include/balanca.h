#ifndef BALANCA_H
#define BALANCA_H

#include <stdio.h>
#include "esp_system.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/i2c.h"
#include "esp_err.h"

#define DOUT_PIN GPIO_NUM_23
#define SCK_PIN GPIO_NUM_22

void hx711_init();
int hx711_get_raw_data();
int balanca(void);

#endif  // BALANCA_H
