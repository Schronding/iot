# ESP32 Code Doubts & Research Log

## User Doubts & Explanations

**Q: Do I need to point the ADC and the GPIO?**
A: No, configuring the ADC channel automatically configures the corresponding GPIO pin for analog input. You don't need a separate GPIO configuration.

**Q: Is 12dB attenuation too high?**
A: The ESP32 ADC requires `ADC_ATTEN_DB_12` to measure voltages up to ~3.3V. Lower attenuations max out at much lower voltages.

**Q: Why don't I see pins 1 and 3 on my ESP32?**
A: Pins 1 and 3 are usually dedicated to the default UART0 (TX/RX) used for debugging via the USB connection, so they are not always broken out or labeled as general I/O pins.

**Q: Why do I need the `adc1_handle` variable?**
A: The newer EPS-IDF API uses handles (pointers to internal driver state objects) to manage peripheral instances, which allows thread-safe and object-oriented driver management.

**Q: Could I completely ditch the array and have a single variable?**
A: Yes. If you only care about the latest reading, a single float variable is sufficient.

**Q: Will the LM35 formula work sending 3.3V?**
A: Yes, as long as the LM35 is supplied with a voltage within its operating range. While typically rated for 4V to 30V, many LM35s operate adequately on 3.3V.

**Q: Should I put a delay in the ADC read loop?**
A: Yes, reading without any delay will starve the watchdog timer and other tasks of lower priority. A delay is essential in FreeRTOS loops.

**Q: What does IRAM mean?**
A: IRAM stands for Instruction RAM, a fast internal memory. Executing code from IRAM prevents latency and crashes when flash memory is temporarily inaccessible (such as during flash write operations).

---

## User Research Summary

*   **Task execution**: FreeRTOS tasks execute as infinite loops inside standard functions.
*   **Logging vs UART**: Using `ESP_LOGI` prints to the standard monitor, while `uart_write_bytes` can be used simultaneously to send data payloads over a specific UART port.
*   **Task Logic Separation**: Sensor tasks only take measurements and update global variables, whereas a dedicated print task handles sending those values.
*   **xTaskCreate Parameters**: Tasks are created using explicit arguments: function pointer, descriptive name, stack size (in bytes), parameter pointer, priority (higher number = higher priority), and a task handle. Priority dictates CPU time allocation.
