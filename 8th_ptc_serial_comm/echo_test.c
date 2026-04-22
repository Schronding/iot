/*
 * Questions answered:
 * - The message does not depend on the LED blink; UART transmission is separate.
 * - TX0 is the default serial output of UART0, so printing to the console sends the text through that pin.
 * - If you only want to send one message, it is enough to print it once over the UART console.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    while (1) {
        const char *message = "Greetings from the virtual world!";
        printf("%s\n", message);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}