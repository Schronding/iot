#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "dht.h" 
#include "esp_adc/adc_oneshot.h"


#define SENSOR_TYPE DHT_TYPE_AM2301 
#define ADC1_CHAN0 ADC_CHANNEL_0
#define IN_DHT22 32      
#define IN_LM35 36 /* Here I am assuming that according to 
my photo of esp32 pinout I have to point both, the ADC (to
tell the ESP32 to use the pin that way) and also from which
GPIO I will be sending the signal (in the photo it says that
ADC1_0 is also the GPIO36)*/    
#define ADC_ATTEN ADC_ATTEN_DB_12 /* Twelve bits seems way too high, but as the 
other options seem to have very little voltage (such as 1.1, 1.5, 2.2) then 
I assume I am practically obligated to use this*/    
#define LED_GPIO 2
#define BASE_STACK_SIZE 1024   
#define TEMP_CAL_GAIN 1.0f
#define TEMP_CAL_OFFSET_C 9.0f     

#define UART_PORT_NUM      UART_NUM_1
#define UART_BAUD_RATE     115200
#define UART_TXD           1
#define UART_RXD           3
/* In my ESP32 I never have connected pin 1 or 3... in fact, I don't 
even see them, as on the right side of it it says 2 and 4*/
#define UART_RTS           UART_PIN_NO_CHANGE
#define UART_CTS           UART_PIN_NO_CHANGE
#define BUF_SIZE           1024

const char *TAG = "LM35 Temperature";
adc_oneshot_unit_handle_t adc1_handle; /* I wonder why do I need the adc1_handle
variable. */

static int adc_raw;
static float voltage;
float temp_samples; /* As I just want the last value I wonder if 
I could completely ditch the idea of using an array and simply have a single
variable so it reads, updates, and logs directly*/

float dht22_temperature;
float dht22_humidity;  
float lm35_temperature; 


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
    *temperature_out = (voltage * 100.0f) * TEMP_CAL_GAIN + TEMP_CAL_OFFSET_C;
    /* As I am sending 3.3 V this formula should work, as I assume the LM35 
    doesn't care much about how much voltage you're sending as long as it is neither
    too much or too little. */

    return ESP_OK;
	
}

static const char *TAG = "DHT_XX_TEST";

/* It seems that indeed every "task" is in reality a whole function*/
void dht_temp_and_hum_task(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        esp_err_t res = dht_read_float_data(SENSOR_TYPE, IN_DHT22, &humidity, &temperature);

        if (res == ESP_OK)
        {
            ESP_LOGI(TAG, "%.1f,%.1f", temperature, humidity);
            int len = sprintf(uart_msg, "%.1f,%.1f\r\n", temperature, humidity);
            uart_write_bytes(UART_PORT_NUM, uart_msg, len);
        }
        else
        {
            ESP_LOGE(TAG, "Sensor read error (Code: %d)", res);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void led_signal_task(void *pvParameters)
{
    (void)pvParameters;
    int level = 0;

    while (1)
    {
        level = !level;
        gpio_set_level(LED_GPIO, level);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void lm35_temp_task(void *pvParameters){
    (void)pvParameters;
    float temperature;
    char uart_msg[64];

    config_ADC(); 
    while(1){
        get_ADC_value(&lm35_temperature); /* I assume I will not put a delay here
        As I don't care that I get all the values correctly, just that I get 
        one... yet what scares me is that by trying to read too quickly I will
        always get incorrect reads */
        vtaskDelay(pdm_TO_TICKS(200));
    }
}

void print_signals_task(*pvParameters){
    char uart_msg[64];
    ESP_LOGI(TAG, "%.1f,%.1f,%.1f", lm35_temperature, dht22_temperature, dht22_humidity);
    int len = sprintf(uart_msg, "%.1f,%.1f\r\n", temperature);
    /* It is so interesting that it seems the serial message doesn't gets
    displayed on the terminal by the default, or that is what I imagine as 
    the double "printing" funcion (ESP_LOGI and uart_write_bytes) are being
    used simultaneously. */
    uart_write_bytes(UART_PORT_NUM, uart_msg, len);

}

void app_main(void)
{
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

    /* I wonder what IRAM means */

    #if CONFIG_UART_ISR_IN_IRAM
        intr_alloc_flags = ESP_INTR_FLAG_IRAM;
    #endif

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TXD, UART_RXD, UART_RTS, UART_CTS));

    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    xTaskCreate(led_signal_task, "led_signal", BASE_STACK_SIZE * 2, NULL, 2, NULL);
    xTaskCreate(lm35_temp_task, "lm35_temp", BASE_STACK_SIZE * 3, NULL, 3, NULL);
    xTaskCreate(dht_temp_and_hum_task, "dht_temp_and_hum", BASE_STACK_SIZE * 3, NULL, 4, NULL);
    xTaskCreate(print_signals_task, "serial_print", BASE_STACK_SIZE * 2, NULL, 5, NULL);
    /* I messed up a bit the logic of the tasks... What the sensors do is simply 
    take measurements, it is the task of `print_signals` the one that actually
    sends them via UART. I imagine I can make use of global variables in order
    to have a notion of what the last values of the sensors were */
}

/* xTaskCreate arguments:
1. `pvTaskCode` Pointer to the function that has to either erase himself or to do an infinite
loop
2. `pcName` Descriptive name for easy debugging
3. `useStackDepth` The size of the stack to assign in RAM memory (in bytes)
4. `pvParameters` Pointer to the data that you want to share with the function
5. `uxPriority` Priority in an integer number.
6. `pxCreatedTask` The handle that you use to control the task in another part 
of the code. 

The priority goes from 0 to 24 in which the larger the number, the more important
it is. If one task of higher priority comes in the middle of the execution of 
one of lower priority it interrupts it. If the tasks have the same priority it 
gives a little bit time of execution to each. 
*/