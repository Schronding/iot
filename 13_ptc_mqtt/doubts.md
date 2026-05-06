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

## ThingSpeak & C Functions

**Q: This timeout means that it is the time it is going to wait for its response before it stops trying?**  
A: Exactly. `timeout_ms = 5000` tells the ESP-IDF HTTP client to wait a maximum of 5,000 milliseconds (5 seconds) for the server to reply. If the server does not respond in that timeframe, the connection attempt is aborted so the code does not freeze indefinitely.

**Q: What does the strstr function do? It seems that it separates it in three fields, this probably acts as a fstring in python, but I can't quite decipher its syntaxis.**  
A: `strstr(haystack, needle)` is a standard C library function that searches for the first occurrence of a substring (`needle`) inside a main string (`haystack`). It does not format or split the string like Python's f-strings or `.split()`. Instead, it simply returns a memory pointer to the exact point where that substring begins. In the code, it's used to quickly locate the starting position of `"field1":` within the raw JSON text response from ThingSpeak.

**Q: What does the function sscanf do?**  
A: `sscanf` (String Scan Formatted) is used to read data from a character string and extract values based on a specified format (the opposite of `sprintf`). By using `sscanf(p1, "\"field1\":\"%f\"", temp);`, the code tells the processor: "Look at this piece of text, expect the literal string `"field1":"`, then grab the floating-point number (`%f`) that follows, and store that directly into the variable `temp`." 

**Q: As the for loop forces this if statement to come after I am guaranteed to only request the data from Jean's thingspeak every 20 seconds... what I don't know is if this data is the historic first one or if it is the last taken?**  
A: It is the most recent (last taken) data. You are querying the URL pathway `/feeds/last.json`. The ThingSpeak API is specifically designed so that `last.json` always returns the very latest single entry uploaded to that specific channel. It will never return the oldest historical data.

**Q: Why I don't have a "catch" statement to see if the reading of Jean's data is correct? How can I know it is? For now I see the same values on his side over and over again; 28.4,38,28.**  
A: The C language does not have `try/catch` exception handling. Instead, error handling is done by checking return values. The function `thingspeak_read_coworker` returns an `esp_err_t` code, but in your main loop it was called simply as `thingspeak_read_coworker(...)` without checking its response. To "catch" errors, you would write `esp_err_t err = thingspeak_read_coworker(...); if (err != ESP_OK) { /* handle error */ }`. The reason you are seeing exactly the same values repeating (`28.4,38,28`) is highly likely because your coworker's device is currently turned off or not transmitting. Since you are querying `last.json`, ThingSpeak just keeps handing you his last known valid data point over and over again.

