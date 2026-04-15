#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"

#define LOW 0
#define HIGH 1

#define IN_M1A GPIO_NUM_18
#define IN_M1B GPIO_NUM_19
#define IN_POTENCIOMETER GPIO_NUM_36
#define IN_PUSH_BUTTON GPIO_NUM_39

#define ENA_MOTOR GPIO_NUM_33

#define MOTOR_PWM_FREQ_HZ 20000
#define MOTOR_PWM_RESOLUTION LEDC_TIMER_10_BIT
#define MOTOR_PWM_MAX_DUTY ((1 << 10) - 1)

#define BUTTON_ACTIVE_LEVEL HIGH
#define BUTTON_DEBOUNCE_MS 40

static esp_err_t configure_pin_motors(void)
{
    const gpio_num_t motor_pins[] = {IN_M1A, IN_M1B};
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

static esp_err_t configure_button(void)
{
    esp_err_t err = gpio_reset_pin(IN_PUSH_BUTTON);
    if (err != ESP_OK) {
        return err;
    }

    err = gpio_set_direction(IN_PUSH_BUTTON, GPIO_MODE_INPUT);
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

static esp_err_t configure_motor_pwm(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = MOTOR_PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = MOTOR_PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    esp_err_t err = ledc_timer_config(&timer_config);
    if (err != ESP_OK) {
        return err;
    }

    ledc_channel_config_t channel_config = {
        .gpio_num = ENA_MOTOR,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };

    return ledc_channel_config(&channel_config);
}

static esp_err_t configure_potentiometer(adc_oneshot_unit_handle_t *adc_handle)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    esp_err_t err = adc_oneshot_new_unit(&init_config, adc_handle);
    if (err != ESP_OK) {
        return err;
    }

    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };

    return adc_oneshot_config_channel(*adc_handle, ADC_CHANNEL_0, &channel_config);
}

static esp_err_t set_motor_direction(bool forward)
{
    esp_err_t err = gpio_set_level(IN_M1A, forward ? HIGH : LOW);
    if (err != ESP_OK) {
        return err;
    }

    err = gpio_set_level(IN_M1B, forward ? LOW : HIGH);
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

static esp_err_t set_motor_speed(uint32_t duty)
{
    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    if (err != ESP_OK) {
        return err;
    }

    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

static uint32_t map_adc_to_pwm(int adc_raw)
{
    if (adc_raw < 0) {
        adc_raw = 0;
    }

    if (adc_raw > 4095) {
        adc_raw = 4095;
    }

    return ((uint32_t)adc_raw * MOTOR_PWM_MAX_DUTY) / 4095;
}

void app_main(void)
{
    adc_oneshot_unit_handle_t adc_handle = NULL;
    int adc_raw = 0;
    bool motor_forward = true;

    int last_button_raw = LOW;
    int button_state = LOW;
    TickType_t last_bounce_tick = 0;

    ESP_ERROR_CHECK(configure_pin_motors());
    ESP_ERROR_CHECK(configure_button());
    ESP_ERROR_CHECK(configure_motor_pwm());
    ESP_ERROR_CHECK(configure_potentiometer(&adc_handle));
    ESP_ERROR_CHECK(set_motor_direction(motor_forward));

    while (1) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_0, &adc_raw));
        ESP_ERROR_CHECK(set_motor_speed(map_adc_to_pwm(adc_raw)));

        int button_raw = gpio_get_level(IN_PUSH_BUTTON);
        TickType_t now = xTaskGetTickCount();

        if (button_raw != last_button_raw) {
            last_bounce_tick = now;
            last_button_raw = button_raw;
        }

        if ((now - last_bounce_tick) >= pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS) &&
            button_raw != button_state) {
            button_state = button_raw;
            if (button_state == BUTTON_ACTIVE_LEVEL) {
                motor_forward = !motor_forward;
                ESP_ERROR_CHECK(set_motor_direction(motor_forward));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
