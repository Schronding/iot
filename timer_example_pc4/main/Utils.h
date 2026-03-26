#ifndef UTILS_H
#define UTILS_H

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"

extern adc_oneshot_unit_handle_t adc1_handle;

esp_err_t config_ADC(void);
esp_err_t get_ADC_value(float *temperature_out, adc_oneshot_unit_handle_t handler);
esp_err_t set_timer(int interval);

int linear_search(float arr[], float value, int max_array);
float * BubbleSort(float arr[], int max_array);
void PrintArrayFlo(float arr[], int maxarray);
void PrintArrayInt(int arr[], int maxarray);
float MinMax(float arr[], int max_array, char * type);

#endif /* UTILS_H */
