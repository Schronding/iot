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

/* You are experiencing this error because in C, if a function is declared to return a value (like float MinMax(...))
, you must ensure that every possible logical path through the function physically returns a value of that type.*/

/* ESP-IDF's compiler is specially configured with a strict rule called -Werror=return-type. This tells the compiler,
 "If you see a scenario where a non-void function might finish without returning a value, don't just warn the 
 user—throw a fatal error and stop compiling entirely." This is a safety measure to prevent undefined behavior in 
 embedded systems. */

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

adc_oneshot_unit_handle_t adc1_handle;

#define SAMPLE_PERIOD_MS 200
#define WINDOW_SECONDS 5
#define TIMER_SECONDS 10
#define N_SAMPLES ((WINDOW_SECONDS * 1000) / SAMPLE_PERIOD_MS)

static TimerHandle_t temp_timer_10s = NULL;
static TimerHandle_t sample_timer_200ms = NULL;
static TimerHandle_t window_timer_5s = NULL;

static bool sampling_active = false;
static int sample_index = 0;
static float adc_samples[N_SAMPLES];

static void ten_second_timer_cb(TimerHandle_t xTimer)
{
    float current_temp = 0.0f;
    if (get_ADC_value(&current_temp, adc1_handle) == ESP_OK) {
        ESP_LOGI(TIMER_TAG, "Current temperature (10s timer): %f", current_temp);
    } else {
        ESP_LOGE(TIMER_TAG, "Failed to read ADC on 10s timer");
    }
}

static void sample_timer_cb(TimerHandle_t xTimer)
{
    if (!sampling_active) {
        return;
    }

    if (sample_index < N_SAMPLES) {
        if (get_ADC_value(&adc_samples[sample_index], adc1_handle) == ESP_OK) {
            sample_index++;
        } else {
            ESP_LOGE(IR_TAG, "Failed to read ADC in 200ms sample timer");
        }
    }
}

static void window_timer_cb(TimerHandle_t xTimer)
{
    if (!sampling_active) {
        return;
    }

    sampling_active = false;
    xTimerStop(sample_timer_200ms, 0);

    if (sample_index > 0) {
        ESP_LOGI(IR_TAG, "Mean temperature (5s timer, %d samples): %f", sample_index, get_mean(adc_samples, sample_index));
    } else {
        ESP_LOGW(IR_TAG, "5s window ended but no ADC samples were collected");
    }

    sample_index = 0;
}

static void start_ir_sampling_window(void)
{
    if (sampling_active) {
        ESP_LOGW(IR_TAG, "IR window already active; new trigger ignored");
        return;
    }

    sampling_active = true;
    sample_index = 0;
    ESP_LOGI(IR_TAG, "Motion detected, starting explicit 5s timer window");

    if (xTimerStart(sample_timer_200ms, 0) != pdPASS) {
        sampling_active = false;
        ESP_LOGE(IR_TAG, "Failed to start 200ms sample timer");
        return;
    }

    if (xTimerStart(window_timer_5s, 0) != pdPASS) {
        sampling_active = false;
        xTimerStop(sample_timer_200ms, 0);
        ESP_LOGE(IR_TAG, "Failed to start 5s window timer");
    }
}

void app_main(void)
{
    /* It seems that this is practically a given for any sampling with ESP32 */
    
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

    /* Using explicit FreeRTOS software timers, as requested by your professor. */
    temp_timer_10s = xTimerCreate("temp_10s", pdMS_TO_TICKS(TIMER_SECONDS * 1000), pdTRUE, NULL, ten_second_timer_cb);
    sample_timer_200ms = xTimerCreate("sample_200ms", pdMS_TO_TICKS(SAMPLE_PERIOD_MS), pdTRUE, NULL, sample_timer_cb);
    window_timer_5s = xTimerCreate("window_5s", pdMS_TO_TICKS(WINDOW_SECONDS * 1000), pdFALSE, NULL, window_timer_cb);

    if (temp_timer_10s == NULL || sample_timer_200ms == NULL || window_timer_5s == NULL) {
        ESP_LOGE(TIMER_TAG, "Timer creation failed");
        return;
    }

    if (xTimerStart(temp_timer_10s, 0) != pdPASS) {
        ESP_LOGE(TIMER_TAG, "Failed to start 10s temperature timer");
        return;
    }

    int last_logged_state = gpio_get_level(INFRARED_IN);
    ESP_LOGI(IR_TAG, "Initial IR state: raw=%d (%s)", last_logged_state, (last_logged_state == 0) ? "BLOCKED" : "CLEAR");

    while (TRUE)
    {
        int current_state = gpio_get_level(INFRARED_IN); /* Fix: changed gpio_set_level to gpio_get_level */
        bool beam_blocked = (current_state == FALSE); 
        /* I need this variable because I might make use of the comprobation
        function in order to know for sure which value I am receiving... but
        to be honest I know practically for sure that the value will be one*/

        if (current_state != last_logged_state)
        {
            ESP_LOGI(IR_TAG, "IR state changed: raw=%d (%s)", current_state, (current_state == 0) ? "BLOCKED" : "CLEAR");

            if (beam_blocked) {
                /* We now start explicit timers here instead of blocking with delays. */
                start_ir_sampling_window();
                
                /* Here I should put a delay with RTOS, as I was told explictly that
                the program can only display 2 averaged temps in a window of 10 seconds
                That makes me think that the timer should also have a variable to
                determine its cycle... it doesn't seem necessary actually, as these
                two different parts of the circuit doens't necessarily have to live within
                one common timeframe. The problem here is that this delay might interrupt
                the program all together... I am thinking about using another timer*/
                /* ANSWER: With explicit software timers, the 10-second report timer and the
                   5-second IR window timer run concurrently without blocking each other. */
            }
        }

        last_logged_state = current_state;

        /* This variable is the one that allows me to force the sequential
        liberation and blocking of the IR... but I don't think I need it now
        */
        // bool waiting_for_release = false;

        /* We still keep a short delay in this polling loop for GPIO edge detection. */
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
