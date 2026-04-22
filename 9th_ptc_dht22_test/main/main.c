#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht.h" 
#define SENSOR_TYPE DHT_TYPE_AM2301 
#define SENSOR_GPIO 32              
#define LED_GPIO 2
#define BASE_STACK_SIZE 1024        

static const char *TAG = "DHT_XX_TEST";

void dht_test_task(void *pvParameters)
{
    (void)pvParameters;
    float temperature, humidity;

    while (1)
    {
        esp_err_t res = dht_read_float_data(SENSOR_TYPE, SENSOR_GPIO, &humidity, &temperature);

        if (res == ESP_OK)
        {
            ESP_LOGI(TAG, "Humidity: %.1f%% | Temperature: %.1f C", humidity, temperature);
        }
        else
        {
            ESP_LOGE(TAG, "Sensor read error (Code: %d)", res);
            printf("SERIAL -> Sensor read error (Code: %d)\n", res);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void led_signal_task(void *pvParameters)
{
    (void)pvParameters;
    int level = 0;

    while (1)
    {
        level = !level;
        gpio_set_level(LED_GPIO, level);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    xTaskCreate(dht_test_task, "dht_test", BASE_STACK_SIZE * 3, NULL, 5, NULL);
    xTaskCreate(led_signal_task, "led_signal", BASE_STACK_SIZE * 2, NULL, 5, NULL);
}