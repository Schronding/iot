#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"

#define LED_PIN 2
#define TOUCH_PAD_GPIO13_CHANNEL TOUCH_PAD_NUM4

esp_err_t print_touch_pad_value();

void app_main(void){
    /* 1. Initialization of touch pad driver */
    touch_pad_init();

    /* 2. Configuration of touch pad GPIO pins */
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    touch_pad_config(TOUCH_PAD_GPIO13_CHANNEL, -1);
    touch_pad_filter_start(10);

    while(true){
        print_touch_pad_value();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

esp_err_t print_touch_pad_value(){
    uint16_t value = 0;
    uint16_t raw_value = 0;
    uint16_t filtered_value = 0;

    /* 3. Taking measurements */
    touch_pad_read_raw_data(TOUCH_PAD_GPIO13_CHANNEL, &raw_value);

    /* 4. Filtering measurements */
    touch_pad_read_filtered(TOUCH_PAD_GPIO13_CHANNEL, &filtered_value);
    touch_pad_read(TOUCH_PAD_GPIO13_CHANNEL, &value);

    printf("val_touch_gpio13 = %d raw_value = %d filtered_value = %d\n", value, raw_value, filtered_value);

    return ESP_OK;
}
