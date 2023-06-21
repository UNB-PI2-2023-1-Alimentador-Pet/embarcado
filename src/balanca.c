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

void hx711_init()
{
    gpio_reset_pin(DOUT_PIN);
    gpio_reset_pin(SCK_PIN);
    gpio_set_direction(DOUT_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(SCK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(SCK_PIN, 0);
}

int hx711_get_raw_data()
{
    while (gpio_get_level(DOUT_PIN))
        ;
    
    int32_t raw_data = 0;
    for (int i = 0; i < 24; i++)
    {
        gpio_set_level(SCK_PIN, 1);
        gpio_set_level(SCK_PIN, 0);
        
        raw_data = (raw_data << 1) | gpio_get_level(DOUT_PIN);
    }
    
    for (int i = 0; i < 1; i++)
    {
        gpio_set_level(SCK_PIN, 1);
        gpio_set_level(SCK_PIN, 0);
    }
    
    raw_data = raw_data ^ 0x800000;
    
    return raw_data;
}

void balanca(void)
{
    hx711_init();

    while (1)
    {
        int32_t raw_data = hx711_get_raw_data();
        
        float peso=0;
        peso=raw_data-8586650;
        printf("Raw Data: %ld", raw_data);
        peso=peso/396;
        printf("peso = %f\n", peso);
        // Convert raw data to weight
        // Use calibration factors to convert raw data to weight value
        // The formula will depend on the characteristics of your load cell and calibration procedure

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}