idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES bt dht esp_adc esp_driver_gpio nvs_flash)
