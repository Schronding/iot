#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#define LOW 0
#define HIGH 1

#define IN_M1A GPIO_NUM_18
#define IN_M1B GPIO_NUM_19

#define IN_M2A GPIO_NUM_2
#define IN_M2B GPIO_NUM_4

#define IN_PHOTORESISTOR GPIO_NUM_36
#define IN_PUSH_BUTTON GPIO_NUM_39
#define IN_IR GPIO_NUM_22

#define ENA_MOTOR GPIO_NUM_33
#define ENB_MOTOR GPIO_NUM_25


#define MOTOR_PWM_FREQ_HZ 20000
#define MOTOR_PWM_RESOLUTION LEDC_TIMER_10_BIT
#define MOTOR_PWM_MAX_DUTY ((1 << 10) - 1)
#define MOTOR_PWM_TIMER LEDC_TIMER_0
#define MOTOR_PWM_SPEED_MODE LEDC_LOW_SPEED_MODE

#define MOTOR_A_PWM_CHANNEL LEDC_CHANNEL_0
#define MOTOR_B_PWM_CHANNEL LEDC_CHANNEL_1

#define PHOTORESISTOR_ADC_CHANNEL ADC_CHANNEL_0
#define PHOTORESISTOR_INVERT 0

#define BUTTON_ACTIVE_LEVEL HIGH
#define BUTTON_DEBOUNCE_MS 40

#define IR_OBJECT_DETECTED_LEVEL LOW

#define MOTOR_B_DIRECTION_INVERT 1

#define MOTOR_MODE_COUNT 5

static const char *TAG = "MOTOR_CTRL";

typedef enum {
    MOTOR_MODE_STOP = 0,
    
    MOTOR_MODE_FORWARD = 1,
    MOTOR_MODE_REVERSE = 2,
    MOTOR_MODE_LEFT = 3,
    MOTOR_MODE_RIGHT = 4,
} motor_mode_t;

static const char *motor_mode_to_string(motor_mode_t mode)
{
    switch (mode) {
    case MOTOR_MODE_STOP:
        return "STOP";
    case MOTOR_MODE_FORWARD:
        return "FORWARD";
    case MOTOR_MODE_REVERSE:
        return "REVERSE";
    case MOTOR_MODE_LEFT:
        return "LEFT";
    case MOTOR_MODE_RIGHT:
        return "RIGHT";
    default:
        return "UNKNOWN";
    }
}

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

static esp_err_t configure_ir_sensor(void)
{
    esp_err_t err = gpio_reset_pin(IN_IR);
    if (err != ESP_OK) {
        return err;
    }

    err = gpio_set_direction(IN_IR, GPIO_MODE_INPUT);
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

static esp_err_t configure_motor_pwm(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode = MOTOR_PWM_SPEED_MODE,
        .duty_resolution = MOTOR_PWM_RESOLUTION,
        .timer_num = MOTOR_PWM_TIMER,
        .freq_hz = MOTOR_PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    esp_err_t err = ledc_timer_config(&timer_config);
    if (err != ESP_OK) {
        return err;
    }

    const ledc_channel_config_t channel_configs[] = {
        {
            .gpio_num = ENA_MOTOR,
            .speed_mode = MOTOR_PWM_SPEED_MODE,
            .channel = MOTOR_A_PWM_CHANNEL,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = MOTOR_PWM_TIMER,
            .duty = 0,
            .hpoint = 0,
        },
        {
            .gpio_num = ENB_MOTOR,
            .speed_mode = MOTOR_PWM_SPEED_MODE,
            .channel = MOTOR_B_PWM_CHANNEL,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = MOTOR_PWM_TIMER,
            .duty = 0,
            .hpoint = 0,
        },
    };

    for (size_t i = 0; i < (sizeof(channel_configs) / sizeof(channel_configs[0])); ++i) {
        err = ledc_channel_config(&channel_configs[i]);
        if (err != ESP_OK) {
            return err;
        }
    }

    return ESP_OK;
}

static esp_err_t configure_photoresistor(adc_oneshot_unit_handle_t *adc_handle)
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

    return adc_oneshot_config_channel(*adc_handle, PHOTORESISTOR_ADC_CHANNEL, &channel_config);
}

static esp_err_t set_single_motor_direction(gpio_num_t in_a, gpio_num_t in_b, int direction)
{
    int level_a = LOW;
    int level_b = LOW;

    if (direction > 0) {
        level_a = HIGH;
        level_b = LOW;
    } else if (direction < 0) {
        level_a = LOW;
        level_b = HIGH;
    }

    esp_err_t err = gpio_set_level(in_a, level_a);
    if (err != ESP_OK) {
        return err;
    }

    err = gpio_set_level(in_b, level_b);
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

static esp_err_t set_motor_pwm_duty(ledc_channel_t channel, uint32_t duty)
{
    esp_err_t err = ledc_set_duty(MOTOR_PWM_SPEED_MODE, channel, duty);
    if (err != ESP_OK) {
        return err;
    }

    err = ledc_update_duty(MOTOR_PWM_SPEED_MODE, channel);
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

static esp_err_t apply_motor_mode(motor_mode_t mode, uint32_t duty)
{
    int motor_1_direction = 0;
    int motor_2_direction = 0;
    int motor_2_effective_direction = 0;

    switch (mode) {
    case MOTOR_MODE_FORWARD:
        motor_1_direction = 1;
        motor_2_direction = 1;
        break;
    case MOTOR_MODE_REVERSE:
        motor_1_direction = -1;
        motor_2_direction = -1;
        break;
    case MOTOR_MODE_LEFT:
        motor_1_direction = -1;
        motor_2_direction = 1;
        break;
    case MOTOR_MODE_RIGHT:
        motor_1_direction = 1;
        motor_2_direction = -1;
        break;
    case MOTOR_MODE_STOP:
    default:
        motor_1_direction = 0;
        motor_2_direction = 0;
        break;
    }

    motor_2_effective_direction = motor_2_direction;
#if MOTOR_B_DIRECTION_INVERT
    motor_2_effective_direction = -motor_2_effective_direction;
#endif

    const uint32_t applied_duty = (mode == MOTOR_MODE_STOP) ? 0 : duty;

    esp_err_t err = set_single_motor_direction(IN_M1A, IN_M1B, motor_1_direction);
    if (err != ESP_OK) {
        return err;
    }

    err = set_single_motor_direction(IN_M2A, IN_M2B, motor_2_effective_direction);
    if (err != ESP_OK) {
        return err;
    }

    err = set_motor_pwm_duty(MOTOR_A_PWM_CHANNEL, applied_duty);
    if (err != ESP_OK) {
        return err;
    }

    return set_motor_pwm_duty(MOTOR_B_PWM_CHANNEL, applied_duty);
}

static uint32_t map_adc_to_pwm(int adc_raw)
{
    if (adc_raw < 0) {
        adc_raw = 0;
    }

    if (adc_raw > 4095) {
        adc_raw = 4095;
    }

    uint32_t normalized_raw = (uint32_t)adc_raw;

#if PHOTORESISTOR_INVERT
    normalized_raw = 4095U - normalized_raw;
#endif

    return (normalized_raw * MOTOR_PWM_MAX_DUTY) / 4095U;
}

static motor_mode_t advance_motor_mode(motor_mode_t mode)
{
    return (motor_mode_t)(((int)mode + 1) % MOTOR_MODE_COUNT);
}

void app_main(void)
{
    adc_oneshot_unit_handle_t adc_handle = NULL;
    int adc_raw = 0;
    motor_mode_t motor_mode = MOTOR_MODE_STOP;

    int last_button_raw = LOW;
    int button_state = LOW;
    TickType_t last_bounce_tick = 0;

    ESP_ERROR_CHECK(configure_pin_motors());
    ESP_ERROR_CHECK(configure_button());
    ESP_ERROR_CHECK(configure_ir_sensor());
    ESP_ERROR_CHECK(configure_motor_pwm());
    ESP_ERROR_CHECK(configure_photoresistor(&adc_handle));
    ESP_ERROR_CHECK(apply_motor_mode(motor_mode, 0));
    ESP_LOGI(TAG, "Initial mode: %d (%s)", motor_mode, motor_mode_to_string(motor_mode));

    while (1) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, PHOTORESISTOR_ADC_CHANNEL, &adc_raw));
        uint32_t duty = map_adc_to_pwm(adc_raw);

        if (gpio_get_level(IN_IR) == IR_OBJECT_DETECTED_LEVEL) {
            if (motor_mode != MOTOR_MODE_STOP) {
                motor_mode = MOTOR_MODE_STOP;
                ESP_LOGW(TAG, "IR object detected -> mode reset to %d (%s)",
                         motor_mode, motor_mode_to_string(motor_mode));
            }

            ESP_ERROR_CHECK(apply_motor_mode(MOTOR_MODE_STOP, 0));
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

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
                motor_mode = advance_motor_mode(motor_mode);
                ESP_LOGI(TAG, "Button press -> mode: %d (%s), PWM duty: %lu",
                         motor_mode,
                         motor_mode_to_string(motor_mode),
                         (unsigned long)duty);
            }
        }

        ESP_ERROR_CHECK(apply_motor_mode(motor_mode, duty));

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
