#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* 
Recommended workflow:

In VS Code, use File -> Open Folder...
Select practicaprevia
Run ESP-IDF: Select Port
Run ESP-IDF: Build
Run ESP-IDF: Flash
Alternative (if you must keep /iot open):

Configure the extension so project directory is explicitly practicaprevia.
Keep root settings.json pointing to:
idf.buildPath = ${workspaceFolder}/practicaprevia/build
Make sure commands are launched for that subproject, not the monorepo root.
Practical answer to your question:

For class/lab work and fewer errors: use local folder practicaprevia directly.
You do not need to create/open the project through the extension first; opening the correct folder is enough.

*/

#define BUTTON 33
#define LED 27
#define HIGH 1
#define LOW 0

void configure_pin(){
    /* 1. Reiniciamos el estado del pin
        2 Configuramos como entrada, salida, entrada salida
        3 Ponemos un estado*/
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_DEF_OUTPUT);
    gpio_set_level(LED, LOW);

    gpio_reset_pin(BUTTON);
    gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON, GPIO_PULLUP_ONLY);

}


void app_main(void)
{
    configure_pin();
    while(true){
        if (gpio_get_level(BUTTON))
            gpio_set_level(LED, HIGH);
        else
            gpio_set_level(LED, LOW);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
