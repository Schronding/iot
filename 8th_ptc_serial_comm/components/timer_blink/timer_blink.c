#include "timer_blink.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "TimerBlink";

// Structure to hold context for each timer (pin + timer handle)
typedef struct {
    int gpio_pin;
    TimerHandle_t timer_handle;
} blink_context_t;

// Global array to store contexts (supports multiple timers)
#define MAX_BLINK_TIMERS 4
static blink_context_t blink_contexts[MAX_BLINK_TIMERS];
static int blink_context_count = 0;

/**
 * @brief Timer callback function that toggles the LED.
 */
static void blink_timer_callback(TimerHandle_t xTimer)
{
    blink_context_t *context = (blink_context_t *)pvTimerGetTimerID(xTimer);
    if (context != NULL) {
        int current_level = gpio_get_level(context->gpio_pin);
        gpio_set_level(context->gpio_pin, !current_level);
    }
}

TimerHandle_t start_blink_timer(int gpio_pin, uint32_t interval_ms)
{
    if (blink_context_count >= MAX_BLINK_TIMERS) {
        ESP_LOGE(TAG, "Max number of blink timers reached");
        return NULL;
    }

    // Configure GPIO as output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    if (gpio_config(&io_conf) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO pin %d", gpio_pin);
        return NULL;
    }

    // Initialize LED to off
    gpio_set_level(gpio_pin, 0);

    // Store context
    blink_context_t *context = &blink_contexts[blink_context_count];
    context->gpio_pin = gpio_pin;
    blink_context_count++;

    // Create timer
    char timer_name[16];
    snprintf(timer_name, sizeof(timer_name), "blink_%d", gpio_pin);
    
    context->timer_handle = xTimerCreate(
        timer_name,                           // Timer name
        pdMS_TO_TICKS(interval_ms),          // Period
        pdTRUE,                               // Auto-reload
        (void *)context,                      // Timer ID (context)
        blink_timer_callback                  // Callback function
    );

    if (context->timer_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create blink timer for GPIO %d", gpio_pin);
        blink_context_count--;
        return NULL;
    }

    // Start timer
    if (xTimerStart(context->timer_handle, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to start blink timer for GPIO %d", gpio_pin);
        xTimerDelete(context->timer_handle, 0);
        blink_context_count--;
        return NULL;
    }

    ESP_LOGI(TAG, "Blink timer started on GPIO %d with interval %u ms", gpio_pin, interval_ms);
    return context->timer_handle;
}

void stop_blink_timer(TimerHandle_t timer_handle)
{
    if (timer_handle == NULL) {
        ESP_LOGW(TAG, "Attempted to stop NULL timer handle");
        return;
    }

    if (xTimerStop(timer_handle, 0) == pdPASS) {
        xTimerDelete(timer_handle, 0);
        ESP_LOGI(TAG, "Blink timer stopped and deleted");
    } else {
        ESP_LOGE(TAG, "Failed to stop blink timer");
    }
}
