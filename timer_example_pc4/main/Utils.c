#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdlib.h>
#include "Utils.h"
#include "Statistics.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"

esp_err_t config_ADC(){
	adc_oneshot_unit_init_cfg_t init_config1 = {
		.unit_id = ADC_UNIT_1,
	};
	adc_oneshot_new_unit(&init_config1, &adc1_handle);
	
	adc_oneshot_chan_cfg_t config = {
		.bitwidth = ADC_BITWIDTH_DEFAULT,
		.atten = ADC_ATTEN,
	};
	adc_oneshot_config_channel(adc1_handle, ADC1_CHAN0, &config);
	
	return ESP_OK;
	
}

// adc_oneshot_unit_handle_t adc1_handle;
/* Having this variable here (`adc1_handle`) doesn't seem well. I think I should give it as
an argument for the function in the `main.c` file */
#define ADC1_CHAN0 ADC_CHANNEL_4 
#define ADC_ATTEN ADC_ATTEN_DB_12

esp_err_t get_ADC_value(float *temperature_out, adc_oneshot_unit_handle_t handler){
    static int adc_raw;
    static float voltage;

    float TEMP_CAL_GAIN = 1.0;
    float TEMP_CAL_OFFSET_C = 0.0;
	adc_oneshot_read(adc1_handle, ADC1_CHAN0, &adc_raw);
	
	voltage = (adc_raw * 3.3/4095.0);
    *temperature_out = (voltage * 100.0f) * TEMP_CAL_GAIN + TEMP_CAL_OFFSET_C;

    return ESP_OK;
	
}

int linear_search(float arr[], float value, int max_array){
    int c; 

    for (c = 0; c < max_array; c++){
        if(arr[c] == value)
            return c;
    }

    return -1;

}


float * BubbleSort(float arr[], int max_array){
    int i, j, temp;

    for (i = 0; i < max_array - 1; i++){
        for (j = 0; j < max_array - i - 1; j++){
            if (arr[j] > arr[j + 1]){
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }

    return arr;
}

/* When you create a function that works with arrays, you always need to provide as a parameter the size of that array. */

void PrintArrayFlo(float arr[], int maxarray){
    int c;
    for (c = 0; c < maxarray; c++){
        printf("\nValue %f in %i index", arr[c], c);
    }

    printf("\n");
}

void PrintArrayInt(int arr[], int maxarray){
    int c;
    for (c = 0; c < maxarray; c++){
        printf("\nValue %f in %i index", arr[c], c);
    }

    printf("\n");
}

float MinMax(float arr[], int max_array, char * type){
    float * temp_array = BubbleSort(arr, max_array);
    if (!strcmp(type, "min"))
        return temp_array[0];
    else if (!strcmp(type, "max"))
        return temp_array[max_array - 1];
    else
        printf("\n --- ERROR: Not a valid option --- \n");
}