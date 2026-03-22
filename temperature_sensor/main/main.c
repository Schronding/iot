#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdlib.h>
#include "Utils.h"
#include "Statistics.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
/* This library is needed for the signals of the ESP-32*/
/*
 * Note about build system:
 * - Header files (.h) are included with #include so this file can see function declarations.
 * - Source files (.c) are compiled via SRCS in main/CMakeLists.txt.
 *   For this project, SRCS should contain: main.c, Utils.c, Statistics.c.
 *
 * ADC (Analog to Digital Converter) Explanation:
 * - LM35 outputs an analog voltage (0-3.3V on ESP32)
 * - The ADC reads this voltage and converts it to a digital value (0-4095 for 12-bit ADC)
 * - Voltage → Digital mapping depends on attenuation (how much you scale the input signal)
 * 
 * In this project:
 * - LM35_IN (GPIO 35) receives the analog signal from the LM35 temperature sensor
 * - ADC1_CHAN0 is ADC_CHANNEL_4, which corresponds to GPIO 32 (check ESP32 datasheet!)
 * - ADC_ATTEN_DB_12 means ±12dB attenuation (allows full 0-3.3V range with 12-bit resolution)
 * 
 * To use the ADC:
 * 1. Initialize the ADC unit (adc_oneshot_unit_handle_t) with adc_oneshot_new_unit()
 * 2. Configure the channel with adc_oneshot_config_channel()
 * 3. Read the analog value: adc_oneshot_read(handle, channel, &raw_value)
 * 4. Convert raw digital value to voltage: voltage = (raw_value / 4095.0) * 3.3
 * 5. Convert voltage to temperature using LM35 formula (usually 100 mV per °C)
 * 6. Compare temperature thresholds and control LEDs
 */

const char *TAG = "LM35 Temperature";
const float temperatura = 3.15;
const float std_temp = 6.28;

#define LOW 0
#define HIGH 1
/* Note: LM35_IN is GPIO 35, but your ADC channel (ADC1_CHAN0) maps to GPIO 32. 
   You may need to physically connect the LM35 output to GPIO 32, or reconfigure 
   the ADC channel to match GPIO 35. Check the ESP32 pinout! */
/* Rewire suggestion based on ESP32 pinout:
    - Move LEDs away from UART2 pins (GPIO16/17) and SPI clock pin (GPIO18)
    - Use GPIO25/GPIO26/GPIO27 for stable digital outputs */
#define COLD_OUT 25
#define IDEAL_OUT 26
#define HOT_OUT 27
#define ADC1_CHAN0 ADC_CHANNEL_4    /* GPIO32 - The ADC input pin where LM35 signal arrives */
/* Why I have that ADC1_CHAN0 is ADC_CHANNEL_4? I assume the 4 is a reserved pin that maps directly to GPIO32*/
#define ADC_ATTEN ADC_ATTEN_DB_12
/* ADC_ATTEN_DB_12: Full range attenuation, covers 0–3.3V at 12-bit resolution (0-4095).
   Other options: DB_0 (0–1.1V), DB_2_5 (0–1.5V), DB_6 (0–2.2V). 
   Choose based on your LM35 voltage range. */
/* Temperature calibration:
    - If terminal temperature is lower/higher than real ambient, tune TEMP_CAL_OFFSET_C.
    - 5V jumper powers the expansion board, but ESP32 ADC input is still 0-3.3V max. */
#define TEMP_CAL_GAIN 1.0f
#define TEMP_CAL_OFFSET_C 9.0f

adc_oneshot_unit_handle_t adc1_handle;
/* ADC handle: stores the initialized ADC unit. Will be populated by config_ADC() 
   and used later in get_ADC_value() calls. */

/* I suspect that the D19 is not meant to be used as an output, as the buzzer doesn't sounds (I connected to f34 and the
leg of the buzzer that is on the same side as the "+" is on i34) I thought that maybe the resistor was making so the buzzer didn't
sound but it seems that isn't the reason. 

Another thing that I don't understand is that the yellow LED is always shining 

When I check the pinout everything it appears that it is a GPIO... that doesn't mean it is a 
General Purpose Input and Output? */
static int adc_raw;
static float voltage;

/* I wonder what `esp_err_t` stands for */
esp_err_t config_ADC(){
	adc_oneshot_unit_init_cfg_t init_config1 = {
		.unit_id = ADC_UNIT_1,
	};
	adc_oneshot_new_unit(&init_config1, &adc1_handle);
	
	adc_oneshot_chan_cfg_t config = {
		.bitwidth = ADC_BITWIDTH_DEFAULT,
		.atten = ADC_ATTEN,
	};
	adc_oneshot_config_channel(adc1_handle, ADC1_CHAN0, &config);
	
	return ESP_OK;
	
}

esp_err_t get_ADC_value(float *temperature_out){
	adc_oneshot_read(adc1_handle, ADC1_CHAN0, &adc_raw);
	
	voltage = (adc_raw * 3.3/4095.0);
    /* I recall that Jesus Solis told us to put the jumper in 5V on the 
    ESP-32 expansion card. But here I do have 3.3, would that change anything?
    That 3.3 is about the voltage that the ESP32 receives or that it sends?
    I want to know because while I have put resistors of 330 Ohms I feel that
    they light very dimly */
    /* LM35 formula: 10 mV per degree Celsius
       temperature (°C) = voltage(V) / 0.01
       or equivalently: temperature = voltage * 100 */
         *temperature_out = (voltage * 100.0f) * TEMP_CAL_GAIN + TEMP_CAL_OFFSET_C;

    return ESP_OK;
	
}

/* I have included the libraries that I need for getting
my simulated temperatures in the `CMakeLists.txt` file.
The thing is that I don't know if it will work*/

typedef struct
{
    float *temp;
    float *humi;
    float *ligh;
    float std_dev;
    float mean;
} sensor;

void app_main(void)
{
    /* You can switch to 5 seconds by changing this to 5 */
    const int window_seconds = 5;
    const int sample_period_ms = 200;
    const int n_samples = (window_seconds * 1000) / sample_period_ms;
    float temp_samples[n_samples];
    int c;
    float mean_temp;
    float std_dev_temp;
    
    /* Initialize ADC before using it */
    config_ADC();
    
    /* Inputs */
    /* NOTE: Do NOT use gpio_reset_pin/gpio_set_direction with ADC channels!
       ADC1_CHAN0 is an enum (ADC_CHANNEL_4), not a GPIO pin number.
       The ADC handles its own pin configuration internally.
       If you need to reset GPIO 32 (the physical pin for ADC_CHANNEL_4), use GPIO 32 directly. */
    // gpio_reset_pin(32);  // Uncomment if needed, but usually ADC auto-configures
    // gpio_set_direction(32, GPIO_MODE_DEF_INPUT);

    /* Outputs - Three LEDs controlled based on temperature */
    gpio_reset_pin(COLD_OUT);
    gpio_set_direction(COLD_OUT, GPIO_MODE_DEF_OUTPUT);

    gpio_reset_pin(IDEAL_OUT);
    gpio_set_direction(IDEAL_OUT, GPIO_MODE_DEF_OUTPUT);

    gpio_reset_pin(HOT_OUT);
    gpio_set_direction(HOT_OUT, GPIO_MODE_DEF_OUTPUT);

    // float curr_temp = LM35_IN;

    /* Statistical analysis of temperature readings:
       - We now use a normal temperature array (temp_samples)
       - The mean is computed over one full window (5s or 10s)
       - Then only one LED is turned on based on that mean value */

    /* Main control loop: sample for window_seconds, then decide one output */
    while (1)
    {
        for (c = 0; c < n_samples; c++)
        {
            /* Read actual ADC value and convert to temperature */
            get_ADC_value(&temp_samples[c]);
            vTaskDelay(pdMS_TO_TICKS(sample_period_ms));
        }

        /* get_mean is declared in Statistics.h and implemented in Statistics.c */
        mean_temp = get_mean(temp_samples, n_samples);
        std_dev_temp = standard_deviation_f(temp_samples, mean_temp, n_samples);

        /* Turn off all outputs before selecting one */
        gpio_set_level(HOT_OUT, LOW);
        gpio_set_level(IDEAL_OUT, LOW);
        gpio_set_level(COLD_OUT, LOW);
        
        if (mean_temp < 22.0)
        {
            gpio_set_level(COLD_OUT, HIGH);
            ESP_LOGW(TAG, "\nTOO COLD. Current: %.2f C | Avg: %.2f C | StdDev: %.2f C. Activate resistence, turn down actuators",
                     temp_samples[n_samples - 1], mean_temp, std_dev_temp);
        }
        else if (mean_temp > 24.0)
        {
            gpio_set_level(HOT_OUT, HIGH);          
            ESP_LOGE(TAG, "\nTOO HOT. Current: %.2f C | Avg: %.2f C | StdDev: %.2f C. Turn down resistence, activate actuators",
                     temp_samples[n_samples - 1], mean_temp, std_dev_temp);
        }

        else
        {
            gpio_set_level(IDEAL_OUT, HIGH);
            ESP_LOGI(TAG, "\nALL IDEAL. Current: %.2f C | Avg: %.2f C | StdDev: %.2f C. Turn down resistence, turn down actuators",
                     temp_samples[n_samples - 1], mean_temp, std_dev_temp);
        }
    }
}
