#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "Utils.h"
#include "Statistics.h"

// Variables
#define INFRARED_IN 32
#define LM35_IN 25
#define TRUE 1
#define FALSE 0
static const char *TIMER_TAG = "Timer";
static const char *IR_TAG = "Infrared";
#define SAMPLE_PERIOD_MS 200 /* This is the given variable that I need to make
sure so the ESP32 has time to sample correctly */
#define TIMER_SECONDS 10
#define SAMPLES_TAKEN 5
#define N_SAMPLES ((WINDOW_SECONDS * 1000) / SAMPLE_PERIOD_MS)
static TimerHandle_t temp_timer_10s = NULL; /* What does the static keyword mean? 
It is interesting to me that it describes a structure */
static float adc_samples[N_SAMPLES] = 0.0; /* The array where I will store all the 
samples when they're already converted. */

void initiate_array(float array, int MAX_ARRAY){
    int c;
    for (c = 0; c < MAX_ARRAY; c++){
        array[c] = 0;
    }
}

adc_oneshot_unit_handle_t adc1_handle;


/* First I will set the timer, as it is the thing that confuses me the most */

static void ten_second_timer_cb(TimerHandle_t xTimer, float avg_temp)
/* What does "..._cb" mean? */
{
    if (get_ADC_value(&avg_temp, adc1_handle) == ESP_OK) {
        ESP_LOGI(TIMER_TAG, "Last measured temperature (10s timer): %f", get_mean(avg_temp, int(SAMPLES_TAKEN/ SAMPLE_PERIOD_MS)));
    } else {
        ESP_LOGE(TIMER_TAG, "Failed to read ADC on 10s timer");
    }
}

void get_samples(float array){
    int c;
    for (c = 0; c < 5; c++){
        array[c] = get_ADC_value;
    }
} 


void app_main(void){
    gpio_config_t io_conf = {
    /* I wonder if I could take this constructor out of the main flow */
        .pin_bit_mask = (1ULL << INFRARED_IN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, 
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);
    config_ADC();

    temp_timer_10s = xTimerCreate("temp_10s", pdMS_TO_TICKS(TIMER_SECONDS * 1000), pdTRUE, NULL, ten_second_timer_cb(curr_temp));
    float curr_temp = 0.0

    while (TRUE){
        int current_state = gpio_get_level(INFRARED_IN); 
        bool beam_blocked = (current_state == FALSE); 

        if (current_state != last_logged_state){
        ESP_LOGI(IR_TAG, "IR state changed: raw=%d (%s)", current_state, (current_state == 0) ? "BLOCKED" : "CLEAR");
        }

        if (beam_blocked){
            get_samples(); 
        }

    }

    vTaskDelay(pdMS_TO_TICKS(50));
}