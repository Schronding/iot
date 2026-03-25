#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define INFRARED_IN 32 // Pin D32 para la señal del receptor IR
#define LED_GREEN 25
#define LED_YELLOW 26
#define LED_RED 27

static void update_occupancy_leds(int count)
{
    if (count <= 77) {
        gpio_set_level(LED_GREEN, 1);
        gpio_set_level(LED_YELLOW, 0);
        gpio_set_level(LED_RED, 0);
    } else if (count <= 155) {
        gpio_set_level(LED_GREEN, 0);
        gpio_set_level(LED_YELLOW, 1);
        gpio_set_level(LED_RED, 0);
    } else {
        gpio_set_level(LED_GREEN, 0);
        gpio_set_level(LED_YELLOW, 0);
        gpio_set_level(LED_RED, 1);
    }
}

static const char *TAG = "IR_COUNTER";

void app_main(void)
{
    // Configuración del pin del receptor infrarrojo
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << INFRARED_IN),
        /* What does it mean 1ULL? and bitmask? */
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        /* why do I need to disable the pulldown if I am just using the pull up? */
        .intr_type = GPIO_INTR_DISABLE // Para este ejemplo usaremos polling (lectura en bucle) con control de estado
    };
    gpio_config(&io_conf);
    /* It seems that io_conf is like a pointer in which I send to `gpio_config` and it pulls all the 
    information I have stored during the constructor into the object of the variable itself... */

    int passenger_count = 0;

    // Configuracion de LEDs de estado de ocupacion.
    gpio_reset_pin(LED_GREEN);
    gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED_YELLOW);
    gpio_set_direction(LED_YELLOW, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED_RED);
    gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);

    update_occupancy_leds(passenger_count);

    // Calibracion automatica: asume que al arrancar el haz esta libre.
    // Toma varias lecturas para decidir el estado base real del circuito.
    int ones = 0;
    int zeros = 0;
    for (int i = 0; i < 40; i++) {
        int s = gpio_get_level(INFRARED_IN);
        if (s == 1) {
            ones++;
        } else {
            zeros++;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    int beam_free_level = (ones > zeros) ? 1 : 0;
    ESP_LOGI(TAG, "Calibration complete. Free-beam level: %d", beam_free_level);

    // Latch para forzar: bloquear -> contar 1 vez -> liberar -> permitir nuevo conteo.
    bool waiting_for_release = false;

    ESP_LOGI(TAG, "System started. Waiting for passengers.");
    ESP_LOGI(TAG, "Current count: %d", passenger_count);

    int last_logged_state = gpio_get_level(INFRARED_IN);

    while (1) {
        // Leemos el estado actual del pin.
        int current_state = gpio_get_level(INFRARED_IN);
        bool beam_blocked = (current_state != beam_free_level);

        // Diagnostico: muestra cuando el pin cambia de estado.
        if (current_state != last_logged_state) {
            ESP_LOGI(TAG, "GPIO%d level change: %d -> %d", INFRARED_IN, last_logged_state, current_state);
            last_logged_state = current_state;
        }

        // Cuenta exactamente una vez cuando el haz pasa de libre a bloqueado.
        if (!waiting_for_release && beam_blocked) {
            passenger_count++;
            update_occupancy_leds(passenger_count);
            ESP_LOGI(TAG, "Passenger detected. Count: %d", passenger_count);
            
            if (passenger_count > 77 && passenger_count < 155) {
                ESP_LOGW(TAG, "More than half the capacity filled. YELLOW LED active.");
            }

            if (passenger_count > 155) {
                ESP_LOGE(TAG, "Capacity exceeded (>%d). RED LED active.", 155);
            }

            waiting_for_release = true;
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        // Solo permitimos otro conteo cuando el haz vuelva a estar libre.
        if (waiting_for_release && !beam_blocked) {
            waiting_for_release = false;
            vTaskDelay(pdMS_TO_TICKS(50));
        }

        /* The writing of the program seems to be very slow... I wonder if it has to do with the amount of 
        comments I am writing, as the extension itself seems to have a lot of troubles still. */

        // Pequeño retardo para no saturar la CPU.
        vTaskDelay(pdMS_TO_TICKS(10));
    }

        /*
        GENERAL WORKFLOW FOR A "TRANSPARENT + BLACK" IR PAIR (BREAK-BEAM STYLE)

        1) Transparent component (usually IR LED emitter):
             - Purpose: emit infrared light continuously or in pulses.
             - Typical polarity: long leg = anode (+), short leg = cathode (-).
             - Correct basic wiring: 3.3V (or 5V, depending on LED spec) -> resistor -> anode,
                 cathode -> GND.
             - Important: ALWAYS use a current-limiting resistor with the emitter LED.

        2) Black component (usually IR phototransistor receiver):
             - Purpose: change current when IR light arrives.
             - Typical polarity for phototransistor (common NPN style):
                 collector -> signal node, emitter -> GND.
             - Signal node then needs a pull-up resistor to 3.3V.
                 Example: 3.3V --[10k]--+-- GPIO input
                                                             |
                                                    collector
                                                    emitter -> GND
             - With IR light present, the transistor conducts and pulls the signal low.
                 Without IR light, pull-up keeps signal high.

        IS YOUR CURRENT CONFIGURATION CORRECT?
        - For the transparent LED: long to + and short to - is usually correct,
            but only if there is a series resistor.
        - For the black receiver: "long to negative and short to positive" is usually NOT
            the typical phototransistor connection, but package pinouts vary by model.
        - Final confirmation must be done with the exact part number datasheet.

        HOW TO SEND THE BLACK SENSOR SIGNAL TO ESP32?
        - Use one wire from the receiver output node (collector + pull-up node) to ESP32 GPIO.
        - Use common ground between sensor circuit and ESP32 ground.
        - Keep logic at 3.3V for ESP32 safety.

        WHAT IS PULL-UP / PULL-DOWN?
        - Pull-up: resistor from signal to VCC. Default state reads HIGH when nothing drives it.
        - Pull-down: resistor from signal to GND. Default state reads LOW when nothing drives it.
        - They prevent a floating (random/noisy) input.

        ESP32 NOTE FOR YOUR CHOSEN PIN (GPIO34):
        - GPIO34 is input-only and does NOT have internal pull-up/pull-down.
        - So if you use GPIO34, add an EXTERNAL pull resistor (commonly 10k).

        DO YOU NEED RESISTORS?
        - Transparent IR LED emitter: YES, mandatory current-limiting resistor.
        - Black receiver:
            * If bare phototransistor/photodiode: YES, usually a pull-up or load resistor is needed.
            * If breakout module with onboard resistor/comparator: maybe already included.

        BEFORE CODING CHECKLIST
        - Confirm part numbers and pinout from datasheets.
        - Confirm emitter resistor value from LED current target.
        - Confirm receiver output topology (phototransistor, photodiode, module digital output).
        - Confirm ESP32 pin voltage is 3.3V max.
        */

        /*
        HOW A NORMAL PULL-UP WORKS WITH INFRARED LEDS (FOR BEAM BREAK)

        Imagine the transparent IR LED is a flashlight that is always ON.
        The black IR receiver (phototransistor) is a light-sensitive switch.

        WIRING (PULL-UP CONFIGURATION):
        1. ESP32 Pin (GPIO 32) is connected to 3.3V through a Pull-Up resistor (can be the internal one).
        2. The same ESP32 Pin is also connected to the collector (often the short leg) of the black receiver.
        3. The emitter (often the long leg) of the black receiver goes to Ground (GND).

        HOW IT BEHAVES:
        - STATE A (IDLE / PATH CLEAR): 
          The transparent LED shines IR light onto the black receiver.
          The light "presses the button" on the receiver, making it conduct electricity.
          Current flows from the ESP32 Pin straight to GND through the receiver.
          RESULT: The ESP32 reads LOW (0V).

        - STATE B (PERSON CROSSES / BEAM BROKEN):
          A person blocks the invisible light.
          The black receiver stops receiving light and stops conducting (switch opens).
          Because the path to GND is blocked, the Pull-Up resistor "pulls" the voltage on the pin up to 3.3V.
          RESULT: The ESP32 reads HIGH (3.3V).

        WHY IS THIS USEFUL FOR COUNTING PASSENGERS?
        When the path is clear, the signal is LOW.
        When a passenger passes and breaks the beam, the signal suddenly jumps to HIGH.
        You can program the ESP32 to trigger an interrupt exactly at this LOW-to-HIGH jump (Rising Edge).
        Once the person finishes passing, it goes back to LOW. 
        You count +1 only on the jump from LOW to HIGH, meaning one person has arrived.
        */

        /*
        BREADBOARD WIRING STEP-BY-STEP (FOR PULL-UP RECEIVER)

        PART 1: THE TRANSPARENT LED (EMITTER)
        - Long leg (+): Connect to a 330 ohm resistor. The other end of this resistor goes to ESP32 3.3V.
        - Short leg (-): Connect straight to the breadboard's GND (Ground) rail.
        (This makes it shine constantly.)

        PART 2: THE BLACK SENSOR (RECEIVER) 
        *Note: In most black IR phototransistors, the LONG leg is the Emitter(-) and the SHORT leg is the Collector(+)*
        
        If we assume the standard standard pinout (Short = Collector, Long = Emitter):

        1. Plug the black sensor into the breadboard so the legs are in two separate rows.
        
        2. The LONG leg (Emitter): 
           - Connect this row straight to the breadboard's GND rail.

        3. The SHORT leg (Collector) - THIS IS YOUR SIGNAL POINT:
           - You need to make a "Junction" here on this row.
           - Connect ONE wire from this row to the ESP32 Pin (e.g., GPIO 32).
           - Connect ONE resistor (e.g. 10k ohm) from this row to the ESP32 3.3V rail.
             (If you are using the ESP32's *internal* pull-up, you can skip this physical resistor.
              But using a physical 10k pull-up resistor is safest and most reliable).

        WHAT HAPPENS AT THAT SHORT-LEG JUNCTION?
        - The ESP32 pin is watching this row.
        - The 10k resistor is constantly trying to "pull" this row up to 3.3V.
        - When IR light hits the black sensor, the sensor "opens a trapdoor" inside itself, letting all that 3.3V fall down the long leg into GND. So the ESP32 sees 0V (LOW).
        - When something blocks the light, the trapdoor slams shut. The 3.3V can't fall to GND anymore, so it builds up at the junction. The ESP32 sees 3.3V (HIGH).
        */
}

/*#include <stdio.h>

#include "freertos/FreeRTOS.h"

#include "driver/gpio.h"

#include "esp_log.h"

#define BUTTON_ISR 27

  

const char *TAG = "Interrupt example";

uint8_t counter = 0;

  

esp_err_t init_irs();

void isr_handler(void*args);

  

void app_main(){

    init_irs();

    while(true){

        ESP_LOGI(TAG, "Counter value: %d", counter);

        vTaskDelay(pdMS_TO_TICKS(100));

    }

  

}

esp_err_t init_irs(){

    gpio_config_t GPIO_config;

    GPIO_config.pin_bit_mask = (1ULL << BUTTON_ISR);

    GPIO_config.mode = GPIO_MODE_INPUT;

    GPIO_config.pull_up_en = GPIO_PULLUP_DISABLE;

    GPIO_config.pull_down_en = GPIO_PULLDOWN_DISABLE;

    GPIO_config.intr_type = GPIO_INTR_NEGEDGE;

  

    gpio_config(&GPIO_config);

  

    gpio_install_isr_service(0);

    gpio_isr_handler_add(BUTTON_ISR, isr_handler, NULL);

    return ESP_OK;

}

  

void isr_handler(void*args){

    counter++;

}*/