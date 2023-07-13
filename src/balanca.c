#include <stdio.h>
#include "esp_system.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "mqtt.h"
#include "string.h"
#include "variaveis_globais.h"

#define DOUT_PIN GPIO_NUM_23
#define SCK_PIN GPIO_NUM_22

// #define DOUT_PIN GPIO_NUM_34
// #define SCK_PIN GPIO_NUM_35

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

float balanca(void)
{
    hx711_init();
    float peso_final;

    for (int i = 0; i < 5; i++)
    {
        int32_t raw_data = hx711_get_raw_data();
        
        float peso=0;
        peso=raw_data-8671380.2;
        //peso=raw_data-8388608;
        char raw_data_str[50];
        // char peso_str[25];
        memset(raw_data_str, 0, 25);
        // memset(peso_str, 0, 25);
        printf("Raw Data: %ld", raw_data);

        peso=peso/396;
        sprintf(raw_data_str, "raw_data: %ld\npeso: %lf", raw_data, peso);
        mqtt_app_publish("feeder/testebalanca", raw_data_str, 1);
        printf("peso = %f\n", peso);
        //if (peso > 0 && peso < 3000){
        peso_final = peso;
        //}
        // Convert raw data to weight
        // Use calibration factors to convert raw data to weight value
        // The formula will depend on the characteristics of your load cell and calibration procedure

        vTaskDelay(pdMS_TO_TICKS(100));
    }
    printf("Peso Final: %f\n", peso_final);
    setPesoBandeja(peso_final);
    return peso_final;
}