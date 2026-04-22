#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "dht.h" 

#define SENSOR_TYPE DHT_TYPE_AM2301 
#define SENSOR_GPIO 32              
#define LED_GPIO 2
#define BASE_STACK_SIZE 1024        

#define UART_PORT_NUM      UART_NUM_1
#define UART_BAUD_RATE     115200
#define UART_TXD           1
#define UART_RXD           3
#define UART_RTS           UART_PIN_NO_CHANGE
#define UART_CTS           UART_PIN_NO_CHANGE
#define BUF_SIZE           1024

static const char *TAG = "DHT_XX_TEST";

void dht_test_task(void *pvParameters)
{
    (void)pvParameters;
    float temperature, humidity;
    char uart_msg[64];

    while (1)
    {
        esp_err_t res = dht_read_float_data(SENSOR_TYPE, SENSOR_GPIO, &humidity, &temperature);

        if (res == ESP_OK)
        {
            ESP_LOGI(TAG, "%.1f,%.1f", temperature, humidity);
            int len = sprintf(uart_msg, "%.1f,%.1f\r\n", temperature, humidity);
            uart_write_bytes(UART_PORT_NUM, uart_msg, len);
        }
        else
        {
            ESP_LOGE(TAG, "Sensor read error (Code: %d)", res);
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
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TXD, UART_RXD, UART_RTS, UART_CTS));

    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    xTaskCreate(dht_test_task, "dht_test", BASE_STACK_SIZE * 3, NULL, 5, NULL);
    xTaskCreate(led_signal_task, "led_signal", BASE_STACK_SIZE * 2, NULL, 5, NULL);
}