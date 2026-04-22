#ifndef TIMER_BLINK_H
#define TIMER_BLINK_H

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

/**
 * @brief Initialize and start a blink timer on a specified GPIO pin.
 * 
 * This function abstracts all the complexity of setting up a FreeRTOS timer
 * and GPIO configuration. Simply call this once to start a periodic blink.
 * 
 * @param gpio_pin The GPIO pin number to control the LED.
 * @param interval_ms The interval in milliseconds between each blink toggle.
 * @return TimerHandle_t The timer handle, or NULL if initialization failed.
 */
TimerHandle_t start_blink_timer(int gpio_pin, uint32_t interval_ms);

/**
 * @brief Stop the blink timer and clean up GPIO.
 * 
 * @param timer_handle The timer handle returned by start_blink_timer.
 */
void stop_blink_timer(TimerHandle_t timer_handle);

#endif // TIMER_BLINK_H
