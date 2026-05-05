idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_event esp_wifi esp_netif mbedtls nvs_flash esp_http_client dht)