#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "Utils.h"
#include "Statistics.h"

/* --- ANSWERS ON HOW LIBRARIES & LINKING WORK IN ESP-IDF ---
 * First, the reason your build failed is likely because you have both `main.c` and
 * `timer_example.c` in your SRCS list inside `main/CMakeLists.txt`, and BOTH of them 
 * define an `app_main(void)` function! You can't have two `app_main` entrance points.
 * 
 * To stop copy-pasting your utilities (like Utils.c and Statistics.c) into every 
 * new project, you should use ESP-IDF's "Components" system. It is much cleaner than 
 * absolute routes inside a single CMakeLists.txt. Here is exactly how to do it:
 * 
 * 1. Create a "components" folder in your root directory:
 *    `/home/schronding/repos/iot/components/shared_utils/`
 * 
 * 2. Move your library files there:
 *    `Utils.h`, `Utils.c`, `Statistics.h`, `Statistics.c`
 * 
 * 3. Create a `CMakeLists.txt` file INSIDE the `shared_utils` folder that says:
 *    idf_component_register(SRCS "Utils.c" "Statistics.c"
 *                           INCLUDE_DIRS ".")
 * 
 * 4. In EVERY project (like timer_example_pc4), edit its root `CMakeLists.txt` 
 *    (the one outside the `main` folder) to point to your new components folder 
 *    BEFORE the `include(...)` line:
 *    
 *    cmake_minimum_required(VERSION 3.16)
 *    set(EXTRA_COMPONENT_DIRS "/home/schronding/repos/iot/components") # ADD THIS!
 *    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
 *    project(timer_example_pc4)
 * 
 * 5. Then in your project's `main/CMakeLists.txt`, just compile your main file:
 *    idf_component_register(SRCS "main.c"
 *                           INCLUDE_DIRS "."
 *                           REQUIRES shared_utils)
 * 
 * Your professor is right that everything roots down to linking correctly inside 
 * CMakeLists.txt. The ESP-IDF build system (Ninja/CMake) handles all the `-I` flags 
 * (include routes) and linking behind the scenes if you organize it as a "Component". 
 * It automatically finds your component, builds it once as a static library (.a file), 
 * and links it into `main.c`.
 */

/* --- ANSWERS TO YOUR QUESTIONS FROM OTHER FILES ---
 * Q from Utils.c: "Where does the array come from? Assign each timer a unique id equal to its array index"
 * A: That is just boilerplate text from FreeRTOS examples assuming you create multiple timers in an array. 
 *    You can just pass any integer (cast to void*) to identify the timer.
 * 
 * Q from Utils.c: "What does pdTRUE mean?"
 * A: pdTRUE is a FreeRTOS macro for 1. In xTimerCreate, it means the timer is an auto-reload (periodic) timer instead of one-shot (pdFALSE).
 *
 * Q from Utils.c: "A void function that is also a pointer? What does this mean? ... why I don't have a name or parenthesis?"
 * A: A `void *` (void pointer) is a generic pointer to any data type. Passing `vTimerCallback` without parentheses 
 *    passes the *address* of the function (a function pointer) so the operating system can call it later when the timer expires.
 */

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

static const char *TIMER_TAG = "Timer";
/* I wonder about the TAG variable because I might need different TAGs for 
different ESP_LOGs. It makes sense that it could be any variable.*/
/* ANSWER: Yes! TAG is just a string used by ESP_LOGI to identify the origin of the message. You can have as many as you want. */
static const char *IR_TAG = "Infrared";


TimerHandle_t xTimer;
/* Indeed it seems that the ESP32 ecosystem makes a lot of use of constructors.*/
int timerId = 1;
int interval = 500;

adc_oneshot_unit_handle_t adc1_handle;

/* Task to run the 10-second timer independently */
void ten_second_temp_task(void *pvParameter)
{
    while(1) {
        float current_temp;
        get_ADC_value(&current_temp, adc1_handle);
        ESP_LOGI(TIMER_TAG, "Current temperature (10s interval): %f", current_temp);
        
        /* Block this specific task for 10 seconds. Other tasks will keep running! */
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void app_main(void)
{
    const int sample_period_ms = 200; /* It seems that this is practically
    a given for any sampling with ESP32*/
    const int window_seconds = 5;
    const int n_samples = (window_seconds * 1000) / sample_period_ms;
    
    /* While Solis says that I can simply copy the files to the folder I am
    going to use, it is annoying to me to have so many versions of the same
    files over and over in the different projects. I do need to find a way
    to use routes in order to have a cleaner repository */

    /* The following seems to be a constructor that I call with its pointer*/
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << INFRARED_IN),
        /* What does it mean 1ULL? and bitmask? */
        /* ANSWER: 1ULL means an Unsigned Long Long (64-bit integer) value of 1. A bitmask uses the bit position to select pins. 
           Shifting 1 by 32 positions (<< 32) selects GPIO 32. */
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, /* Enabled pull-up as you mentioned it's a pull-up config */
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        /* why do I need to disable the pulldown if I am just using the pull up? */
        /* ANSWER: It ensures the internal pulldown resistor is off so it doesn't fight the pull-up resistor and create a voltage divider. */
        .intr_type = GPIO_INTR_DISABLE};

    gpio_config(&io_conf);
    config_ADC(); 

    /* Create the 10-second temperature task. It runs concurrently with this main loop! */
    xTaskCreate(ten_second_temp_task, "10s_temp_task", 2048, NULL, 5, NULL);

    int last_logged_state = gpio_get_level(INFRARED_IN);

    while (TRUE)
    {
        int current_state = gpio_get_level(INFRARED_IN); /* Fix: changed gpio_set_level to gpio_get_level */
        bool beam_blocked = (current_state == FALSE); 
        /* I need this variable because I might make use of the comprobation
        function in order to know for sure which value I am receiving... but
        to be honest I know practically for sure that the value will be one*/

        if (beam_blocked && current_state != last_logged_state)
        {
            ESP_LOGI(IR_TAG, "Motion detected! Sampling for 5 seconds...");
            float adc_samples[n_samples]; 
            
            for (int c = 0; c < n_samples; c++){
                /* We don't use set_timer here. We just read and delay. The RTOS scheduler handles the wait. */
                get_ADC_value(&adc_samples[c], adc1_handle); 
                /* To be honest I don't understand why do we need this loop
                though. I want just the last temperature read every 10 seconds
                but as I do not have python negative indexing I wonder how 
                I might do to get the last value... this is not the read after
                every 10 seconds, this is the mean of the 5 seconds*/
                
                vTaskDelay(pdMS_TO_TICKS(sample_period_ms)); /* Delay 200ms between samples */
            }

            /* Am I forced to include a tag in the `ESP_LOG` functions?*/
            /* ANSWER: Yes, ESP_LOG requires a string tag as the first argument so it knows who printed the message. */
            ESP_LOGI(IR_TAG, "Mean temperature (IR switch): %f", get_mean(adc_samples, n_samples));
            
            /* Here I should put a delay with RTOS, as I was told explictly that
            the program can only display 2 averaged temps in a window of 10 seconds
            That makes me think that the timer should also have a variable to
            determine its cycle... it doesn't seem necessary actually, as these
            two different parts of the circuit doens't necessarily have to live within
            one common timeframe. The problem here is that this delay might interrupt
            the program all together... I am thinking about using another timer*/
            /* ANSWER: Because the 10-second timer is in its own FreeRTOS task, this loop can take 5 seconds 
               to sample without interrupting or delaying the 10-second prints at all! */
        }

        last_logged_state = current_state;

        /* This variable is the one that allows me to force the sequential
        liberation and blocking of the IR... but I don't think I need it now
        */
        // bool waiting_for_release = false;

        /* Give the CPU a small break to prevent the watchdog timer from triggering */
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
