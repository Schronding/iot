#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "Utils.h"
#include "Statistics.h"

/* I need to include the flag that checks whether the signal has been
blocked... so I need it to put again in the GPIO32. For what I understand
I need to put the temperature sensor to always read, but to give the
mean temperature every 10 seconds... for what I understand once I feel
presence I need to read those 5 seconds, get the mean and print them. If I
come to put in the sensor on the second 6 or higher then I don't print the
last one, I get the last temperature every 10 seconds, no matter what; I might
get both printed at the same time. */

#define INFRARED_IN 32
#define LM35_IN 25
#define TRUE 1
#define FALSE 0

adc_oneshot_unit_handle_t adc1_handle;

void app_main(void)
{
    const int sample_period_ms = 200; /* It seems that this is practically
    a given for any sampling with ESP32*/
    const int window_seconds = 5;

    const int n_samples = (window_seconds * 1000) / sample_period_ms;

    float temp_samples[n_samples] /* The array that I will feed to my
    standard deviation and mean functions */

        /* While Solis says that I can simply copy the files to the folder I am
        going to use, it is annoying to me to have so many versions of the same
        files over and over in the different projects. I do need to find a way
        to use routes in order to have a cleaner repository */

        /* The following seems to be a constructor that I call with its pointer*/
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << INFRARED_IN),
            /* What does it mean 1ULL? and bitmask? */
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            /* why do I need to disable the pulldown if I am just using the pull up? */
            .intr_type = GPIO_INTR_DISABLE};

    gpio_config(&io_conf);

    /* This variable will help me know when there is a change in the bit that
    describes whether there is something in the way (0) and when the IR LEDs
    are facing one another (1) if I remember well*/
    int last_logged_state = gpio_get_level(INFRARED_IN);

    while (TRUE)
    {
        int current_state = gpio_set_level(INFRARED_IN);
        bool beam_blocked = (current_state != TRUE);
        /* I need this variable because I might make use of the comprobation
        function in order to know for sure which value I am receiving... but
        to be honest I know practically for sure that the value will be one*/

        if (current_state != last_logged_state)
        {
            ESP_LOGI(TAG, "Presence detected. Getting temp", INFRARED_IN, last_logged_state, current_state);
            last_logged_state = current_state;
            /* Here I should put a delay with RTOS, as I was told explictly that
            the program can only display 2 averaged temps in a window of 10 seconds
            That makes me think that the timer should also have a variable to
            determine its cycle... it doesn't seem necessary actually, as these
            two different parts of the circuit doens't necessarily have to live within
            one common timeframe. The problem here is that this delay might interrupt
            the program all together... I am thinking about using another timer*/
        }

        /* This variable is the one that allows me to force the sequential
        liberation and blocking of the IR... but I don't think I need it now
        */
        // bool waiting_for_release = false;
    }
}
