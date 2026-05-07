#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "freertos/projdefs.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "driver/gpio.h"
#include "esp_err.h"

#include "dht.h"
#include "connect_wifi.h"

#define DHTPIN GPIO_NUM_26

#define MQTT_PUB_TEMP_DHT "esp32/dht/temperature"
#define MQTT_PUB_HUM_DHT "esp32/dht/humidity"

static const char *TAG = "MQTT_EXAMPLE";

uint32_t MQTT_CONNECTED = 0;
esp_mqtt_client_handle_t client = NULL;
gpio_num_t dht_gpio = DHTPIN; /* Digital pin connected to the DHT sensor */
dht_sensor_type_t sensor_type = DHT_TYPE_AM2301; /* Using DHT11 -> DHT_TYPE_DHT11 */

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id){
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            MQTT_CONNECTED = 1;
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            MQTT_CONNECTED = 0;
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;

        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

static void mqtt_app_start(void){
    ESP_LOGI(TAG, "Starting MQTT");

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://172.20.10.4:1883",
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void pub_task(void *params){
    float hum = 0;
    float temp = 0;
    
    while (true){
        if (dht_read_float_data(sensor_type, dht_gpio, &hum, &temp) == ESP_OK){
            char temperature[12];
            sprintf(temperature, "%.2f C", temp);
            
            char humidity[10];
            sprintf(humidity, "%.2f %%", hum);
            
            if (MQTT_CONNECTED){
                ESP_LOGI(TAG, "Publishing...");
                esp_mqtt_client_publish(client, MQTT_PUB_TEMP_DHT, temperature, 0, 0, 2);
                esp_mqtt_client_publish(client, MQTT_PUB_HUM_DHT, humidity, 0, 0, 2);
                
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
        }
        else
            ESP_LOGE(TAG, "Could not read data from sensor");

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void){
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    
    connect_wifi();
    
    if (wifi_connect_status){
        mqtt_app_start();
    
        xTaskCreate(pub_task, "pub_task", 1024 * 5, NULL, 5, NULL);
    }
}
