#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/mcpwm_prelude.h"

/* Set the parameters according to your servo */
#define SERVO_MIN_PULSEWIDTH_US 500 /* Minimum pulse width in microsecond */
#define SERVO_MAX_PULSEWIDTH_US 2400 /* Maximum pulse width in microsecond */
#define SERVO_MIN_DEGREE 0 /* Minimum angle */
#define SERVO_MAX_DEGREE 180 /* Maximum angle */
#define FRST_SERVO_PULSE_GPIO 21 /* GPIO connects to the PWM signal line */
#define SCND_SERVO_PULSE_GPIO 22
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000 /* 1MHz, 1us per tick */
#define SERVO_TIMEBASE_PERIOD 20000 /* 20000 ticks, 20ms */
#define SERVO_STEP_DEGREE 5
#define SERVO_STEP_DELAY_MS 50
#define IR_POLL_DELAY_MS 20
/* GPIO36 is input-only on ESP32, use an output-capable pin for buzzer PWM */
#define BUZZER_OUT 27
#define IR_IN 32
#define BUZZER_FREQ_HZ 20000
#define BUZZER_DUTY_ON 512
#define BUZZER_DUTY_OFF 0

static const char *TAG = "PWM servo";

mcpwm_timer_handle_t timer = NULL;
mcpwm_oper_handle_t oper = NULL;
mcpwm_cmpr_handle_t comparator_frst = NULL;
mcpwm_cmpr_handle_t comparator_scnd = NULL;
mcpwm_gen_handle_t generator_frst = NULL;
mcpwm_gen_handle_t generator_scnd = NULL;

static esp_err_t mcpwm_config(void);
static esp_err_t io_config(void);
static esp_err_t buzzer_config(void);
static inline bool ir_beam_blocked(void);
static inline uint32_t angle_to_compare(int angle);
static void set_servo_angles(int frst_angle, int scnd_angle);
static void buzzer_set(bool enabled);
static void run_scan_sequence(void);

void app_main(void){
    ESP_ERROR_CHECK(io_config());
    ESP_ERROR_CHECK(buzzer_config());
    ESP_ERROR_CHECK(mcpwm_config());

    set_servo_angles(SERVO_MIN_DEGREE, SERVO_MIN_DEGREE);
    buzzer_set(false);

    while (1) {
        bool beam_blocked = ir_beam_blocked();

        if (beam_blocked) {
            ESP_LOGI(TAG, "IR Blocked: Starting servomotor scanning");
            run_scan_sequence();
            ESP_LOGI(TAG, "Scanning finalized");
        }
        vTaskDelay(pdMS_TO_TICKS(IR_POLL_DELAY_MS));
    }
}

static esp_err_t io_config(void)
{
    gpio_config_t ir_cfg = {
        .pin_bit_mask = 1ULL << IR_IN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    return gpio_config(&ir_cfg);
}

static inline bool ir_beam_blocked(void)
{
    // J44 IR input is pull-up in this wiring, so blocked state reads high.
    return gpio_get_level(IR_IN) == 1;
}

static esp_err_t buzzer_config(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = BUZZER_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    ledc_channel_config_t channel_cfg = {
        .gpio_num = BUZZER_OUT,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = BUZZER_DUTY_OFF,
        .hpoint = 0,
    };

    return ledc_channel_config(&channel_cfg);
}

static esp_err_t mcpwm_config(void){
    ESP_LOGI(TAG, "Create timer and operator");
    
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = SERVO_TIMEBASE_RESOLUTION_HZ,
        .period_ticks = SERVO_TIMEBASE_PERIOD,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    
    mcpwm_operator_config_t operator_config = {
        .group_id = 0,
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));

    ESP_LOGI(TAG, "Connect timer and operator");
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    ESP_LOGI(TAG, "Create comparator and generator from the operator");
    
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator_frst));
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator_scnd));

    
    mcpwm_generator_config_t generator_config_frst = {
        .gen_gpio_num = FRST_SERVO_PULSE_GPIO,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config_frst, &generator_frst));

    mcpwm_generator_config_t generator_config_scnd = {
        .gen_gpio_num = SCND_SERVO_PULSE_GPIO,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config_scnd, &generator_scnd));

    // set the initial compare value, so that the servo will spin to the center position
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator_frst, angle_to_compare(0)));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator_scnd, angle_to_compare(0)));

    ESP_LOGI(TAG, "Set generator action on timer and compare event");
    // go high on counter empty
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator_frst, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator_scnd, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator_frst, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_frst, MCPWM_GEN_ACTION_LOW)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator_scnd, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_scnd, MCPWM_GEN_ACTION_LOW)));

    ESP_LOGI(TAG, "Enable and start timer");
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    return ESP_OK;
}

static inline uint32_t angle_to_compare(int angle){
    return (angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

static void set_servo_angles(int frst_angle, int scnd_angle)
{
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator_frst, angle_to_compare(frst_angle)));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator_scnd, angle_to_compare(scnd_angle)));
}

static void buzzer_set(bool enabled)
{
    uint32_t duty = enabled ? BUZZER_DUTY_ON : BUZZER_DUTY_OFF;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}

static void run_scan_sequence(void)
{
    int scnd_angle;
    int frst_angle;

    buzzer_set(true);

    while (ir_beam_blocked()) {
        for (scnd_angle = SERVO_MIN_DEGREE; scnd_angle <= SERVO_MAX_DEGREE && ir_beam_blocked(); scnd_angle += SERVO_STEP_DEGREE) {
            for (frst_angle = SERVO_MIN_DEGREE; frst_angle <= SERVO_MAX_DEGREE && ir_beam_blocked(); frst_angle += SERVO_STEP_DEGREE) {
                set_servo_angles(frst_angle, scnd_angle);
                vTaskDelay(pdMS_TO_TICKS(SERVO_STEP_DELAY_MS));
            }

            for (frst_angle = SERVO_MAX_DEGREE; frst_angle >= SERVO_MIN_DEGREE && ir_beam_blocked(); frst_angle -= SERVO_STEP_DEGREE) {
                set_servo_angles(frst_angle, scnd_angle);
                vTaskDelay(pdMS_TO_TICKS(SERVO_STEP_DELAY_MS));
            }
        }
    }

    set_servo_angles(SERVO_MIN_DEGREE, SERVO_MIN_DEGREE);
    buzzer_set(false);
}
