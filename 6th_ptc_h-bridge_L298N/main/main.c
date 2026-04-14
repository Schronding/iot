#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LOW 0
#define HIGH 1

#define IN_M1A GPIO_NUM_18
#define IN_M1B GPIO_NUM_19
#define IN_M2A GPIO_NUM_21
#define IN_M2B GPIO_NUM_22

static esp_err_t configure_pin_motors(void)
{
    const gpio_num_t motor_pins[] = {IN_M1A, IN_M1B, IN_M2A, IN_M2B};
    const size_t motor_pins_count = sizeof(motor_pins) / sizeof(motor_pins[0]);

    for (size_t i = 0; i < motor_pins_count; ++i) {
        esp_err_t err = gpio_reset_pin(motor_pins[i]);
        if (err != ESP_OK) {
            return err;
        }

        err = gpio_set_direction(motor_pins[i], GPIO_MODE_OUTPUT);
        if (err != ESP_OK) {
            return err;
        }

        err = gpio_set_level(motor_pins[i], LOW);
        if (err != ESP_OK) {
            return err;
        }
    }

    return ESP_OK;
}

static esp_err_t forward_car(void)
{
    esp_err_t err = gpio_set_level(IN_M1A, HIGH);
    if (err != ESP_OK) {
        return err;
    }

    err = gpio_set_level(IN_M1B, LOW);
    if (err != ESP_OK) {
        return err;
    }

    err = gpio_set_level(IN_M2A, HIGH);
    if (err != ESP_OK) {
        return err;
    }

    err = gpio_set_level(IN_M2B, LOW);
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

static esp_err_t backward_car(void)
{
    esp_err_t err = gpio_set_level(IN_M1A, LOW);
    if (err != ESP_OK) {
        return err;
    }

    err = gpio_set_level(IN_M1B, HIGH);
    if (err != ESP_OK) {
        return err;
    }

    err = gpio_set_level(IN_M2A, LOW);
    if (err != ESP_OK) {
        return err;
    }

    err = gpio_set_level(IN_M2B, HIGH);
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

void app_main(void)
{
    ESP_ERROR_CHECK(configure_pin_motors());

    while (TRUE) {
        ESP_ERROR_CHECK(forward_car());

        vTaskDelay(pdMS_to_TICKS(500));

        ESP_ERROR_CHECK(backward_car());

        vTaskDelay(pdMS_to_TICKS(500));
    }
}
