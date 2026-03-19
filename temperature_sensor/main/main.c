#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdlib.h>
#include "Utils.h"
#include "Statistics.h"
#include "esp_adc/adc_oneshot.h"

const char *TAG = "LM35 Temperature";
const float temperatura = 3.15;
const float std_temp = 6.28;

#define LM35_IN 35
#define COLD_OUT 16
#define IDEAL_OUT 17
#define HOT_OUT 18
#define ADC1_CHAN0 ADC_CHANNEL_4    /* GPIO32 */
#define ADC_ATTEN ADC_ATTEN_DB_12

adc_oneshot_unit_handle_t adc1_handle;


typedef struct{
    float * temp;
    float * humi; 
    float * ligh; 
    float std_dev;
    float mean;
} sensor;

void app_main(void)
{
    int n = 100;
    sensor TSL2560; 
    sensor DHT22; 
    DHT22.temp = (float * )malloc(n * sizeof(float));
    DHT22.humi = (float * )malloc(n * sizeof(float));
    TSL2560.ligh = (float * )malloc(n * sizeof(float));
    int c; 
    short int TempStatus; 
    for (c = 0; c < n; c++){
        DHT22.temp[c] = generate_random(21.0, 25.0);
        DHT22.humi[c] = generate_random(0.0, 100.0);
        TSL2560.ligh[c] = generate_random(0.1, 40000.0);
    }

    /* Inputs */
    gpio_reset_pin(LM35_IN);
    gpio_set_direction(LM35_IN, GPIO_MODE_DEF_INPUT);

    /* Outputs */
    gpio_reset_pin(COLD_OUT);
    gpio_set_direction(COLD_OUT, GPIO_MODE_DEF_OUTPUT);

    gpio_reset_pin(IDEAL_OUT);
    gpio_set_direction(IDEAL_OUT, GPIO_MODE_DEF_OUTPUT);

    gpio_reset_pin(HOT_OUT);
    gpio_set_direction(HOT_OUT, GPIO_MODE_DEF_OUTPUT);

    float curr_temp = LM35_IN;


    float mean_temp; 
    DHT22.mean = get_mean(DHT22.temp, n);
    DHT22.std_dev = standard_deviation_f(DHT22.temp, DHT22.mean, n);

    for (c = 0; c < n; c+=10)
    {
        float *slice = DHT22.temp + c;  // pointer to the c-th element
        mean_temp = get_mean(slice, 10); // read 10 elements starting from index c
        if (mean_temp < 22.0){ 
            TempStatus = -1;
            printf("\nTOO COLD. Activate resistence, turn down actuators");
            continue; 
        }

        else if (mean_temp > 24.0){
            TempStatus = 1;
            printf("\nTOO HOT. Turn down resistence, activate actuators");
            continue;
        }

        TempStatus = 0; 
        printf("\nALL IDEAL. Turn down resistence, turn down actuators"); 
        
    }  
    while (true)
    {
        ESP_LOGV(TAG, "Temperatura actual: %f +- %f", temperatura, std_temp);
        if 
        ESP_LOGE(TAG, "Este es un error");
        ESP_LOGI(TAG, "Este es un error informativo");
        ESP_LOGW(TAG, "Este es un warning");

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
