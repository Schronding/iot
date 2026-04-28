idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES dht esp_driver_uart esp_driver_gpio esp_adc freertos log)
