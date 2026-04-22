#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_PIN 2  
#define BLINK_INTERVAL_MS 1000  

static const char *TAG = "MainApp";

static void blink_timer_callback(TimerHandle_t xTimer)
{
    static int led_state = 0; 
    
    led_state = !led_state;   
    
    gpio_set_level(LED_PIN, led_state); 
}

void app_main(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    TimerHandle_t blink_timer = xTimerCreate(
        "led_timer",                      
        pdMS_TO_TICKS(BLINK_INTERVAL_MS), 
        pdTRUE,                          
        (void *)0,                        
        blink_timer_callback              
    );

    if (blink_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create timer");
        return;
    }

    if (xTimerStart(blink_timer, 0) == pdPASS) {
        ESP_LOGI(TAG, "Timer started successfully. Blinking every %u ms", (unsigned int)BLINK_INTERVAL_MS);
    } else {
        ESP_LOGE(TAG, "Failed to start timer");
    }

    while (1) {
        ESP_LOGI(TAG, "Greetings from the virtual world!");
        vTaskDelay(pdMS_TO_TICKS(5000));  
    }
}