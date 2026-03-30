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
#define SAMPLES_TAKEN 25

static TimerHandle_t temp_timer_10s = NULL; /* What does the static keyword mean? 
It is interesting to me that it describes a structure */


static float last_average_temperature = 0.0f;

/* The array where I will store all the samples when they're already converted. */
static float adc_samples[SAMPLES_TAKEN];


void initiate_array(float *array, int MAX_ARRAY){
    int c;
    for (c = 0; c < MAX_ARRAY; c++){
        array[c] = 0;
    }
}


adc_oneshot_unit_handle_t adc1_handle;


/* First I will set the timer, as it is the thing that confuses me the most */

static void ten_second_timer_cb(TimerHandle_t xTimer)
/* What does "..._cb" mean? */
{
    (void)xTimer;

    ESP_LOGI(TIMER_TAG, "Last average temperature (10s timer): %.2f C", last_average_temperature);
}

void get_samples(void){
    int c;
    int valid_samples = 0;

    initiate_array(adc_samples, SAMPLES_TAKEN);

    for (c = 0; c < SAMPLES_TAKEN; c++){
        float current_temp = 0.0f;
        if (get_ADC_value(&current_temp, adc1_handle) == ESP_OK) {
            adc_samples[valid_samples++] = current_temp;
            vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
        } else {
            ESP_LOGE(TIMER_TAG, "ADC read failed at sample index %d", c);
            break;
        }
    }

    if (valid_samples > 0) {
        last_average_temperature = get_mean(adc_samples, valid_samples);
        ESP_LOGI(TIMER_TAG, "Average updated after IR block: %.2f C (%d samples)", last_average_temperature, valid_samples);
    } else {
        ESP_LOGW(TIMER_TAG, "No valid samples in this batch. Keeping previous average.");
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
    bool last_logged_state = gpio_get_level(INFRARED_IN);
    bool was_beam_blocked = (last_logged_state == FALSE);

    initiate_array(adc_samples, SAMPLES_TAKEN);

    temp_timer_10s = xTimerCreate("temp_10s", pdMS_TO_TICKS(TIMER_SECONDS * 1000), pdTRUE, NULL, ten_second_timer_cb);
    if (temp_timer_10s == NULL) {
        ESP_LOGE(TIMER_TAG, "Failed to create 10 second timer");
    } else if (xTimerStart(temp_timer_10s, 0) != pdPASS) {
        ESP_LOGE(TIMER_TAG, "Failed to start 10 second timer");
    }

    while (TRUE){
        int current_state = gpio_get_level(INFRARED_IN); 
        bool beam_blocked = (current_state == FALSE); 

        if (current_state != last_logged_state){
            ESP_LOGI(IR_TAG, "IR state changed: raw=%d (%s)", current_state, (current_state == 0) ? "BLOCKED" : "CLEAR");
            last_logged_state = current_state;
        }

        if (beam_blocked && !was_beam_blocked){
            get_samples();
        }

        was_beam_blocked = beam_blocked;

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}