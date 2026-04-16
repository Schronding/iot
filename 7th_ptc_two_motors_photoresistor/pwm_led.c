#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/timers.h"
#include "driver/ledc.h"

#define LED_PIN 2

uint8_t led_level = 0;
static const char *TAG = "PWM example";
TimerHandle_t xTimer;
int interval = 50;
int timerId = 1;
int duty = 0;

esp_err_t init_LED();
esp_err_t blink_LED();
esp_err_t set_timer();
esp_err_t set_PWM();
esp_err_t set_PWM_duty();

void vTimerCallback(TimerHandle_t pxTimer){
    
    duty += 10;

    if (duty > 1023)
        duty = 0;

    blink_LED();
    set_PWM_duty();
}

void app_main(void){
    init_LED();
    set_PWM();
    set_timer();
}

esp_err_t init_LED(){
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    return ESP_OK;
}

esp_err_t blink_LED(){
    led_level = !led_level;
    gpio_set_level(LED_PIN, led_level);

    return ESP_OK;
}

esp_err_t set_timer(){
    ESP_LOGI(TAG, "Timer init configuration");

    xTimer = xTimerCreate("Timer", /* Just a text name */
                            (pdMS_TO_TICKS(interval)), /* The timer period in ticks */
                            pdTRUE, /* The timer will auto-reload themselves when they expire */
                            (void *)timerId, /* Assign each timer a unique id equal to its array index */
                            vTimerCallback /* Each timer calls the same callback when it expires */
    );

    if (xTimer == NULL){
        /* The timer was not created */
        ESP_LOGE(TAG, "The timer was not created");
    }
    else{
        if(xTimerStart(xTimer, 0) != pdPASS){
            /* The timer could not be set into the active state */
            ESP_LOGE(TAG, "The timer could not be set into the active state");
        }
    }

    return ESP_OK;
}

esp_err_t set_PWM(){
    ledc_channel_config_t channelConfig = {0};
    channelConfig.gpio_num = 33;
    channelConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
    channelConfig.channel = LEDC_CHANNEL_0;
    channelConfig.intr_type = LEDC_INTR_DISABLE;
    channelConfig.timer_sel = LEDC_TIMER_0;
    channelConfig.duty = 0;

    ledc_channel_config(&channelConfig);

    ledc_timer_config_t timerConfig = {0};
    timerConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
    timerConfig.duty_resolution = LEDC_TIMER_10_BIT;
    timerConfig.timer_num = LEDC_TIMER_0;
    timerConfig.freq_hz = 20000; /* 20 KHz */

    ledc_timer_config(&timerConfig);

    return ESP_OK;
}

esp_err_t set_PWM_duty(){
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0,
                  duty);


    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

    return ESP_OK;
}