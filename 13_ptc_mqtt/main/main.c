#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "mqtt_client.h"

#include "driver/gpio.h"
#include "dht.h"
#include "connect_wifi.h"

#define DHT_GPIO GPIO_NUM_4
#define DHT_TYPE DHT_TYPE_AM2301

#define SAMPLE_INTERVAL_MS 2000
#define SAMPLE_COUNT 10

#define MQTT_BROKER_URI "mqtt://192.168.1.216:1883"
#define MQTT_TOPIC_TEMP "enesj/vi/a201/temp_brayan"
#define MQTT_TOPIC_HUM "enesj/vi/a201/hum_brayan"
#define MQTT_TOPIC_ST "enesj/vi/a201/st_brayan"

static const char *TAG = "MQTT_PUBLISH";

static esp_mqtt_client_handle_t client = NULL;
static bool mqtt_connected = false;

static float calcular_sensacion_termica(float temp_c, float humidity)
{
    float t = (temp_c * 9.0f / 5.0f) + 32.0f;
    float hi = 0.5f * (t + 61.0f + ((t - 68.0f) * 1.2f) + (humidity * 0.094f));

    if (hi > 80.0f) {
        const float c1 = -42.379f;
        const float c2 = 2.04901523f;
        const float c3 = 10.14333127f;
        const float c4 = -0.22475541f;
        const float c5 = -0.00683783f;
        const float c6 = -0.05481717f;
        const float c7 = 0.00122874f;
        const float c8 = 0.00085282f;
        const float c9 = -0.00000199f;

        hi = c1 + (c2 * t) + (c3 * humidity) + (c4 * t * humidity) +
             (c5 * t * t) + (c6 * humidity * humidity) +
             (c7 * t * t * humidity) + (c8 * t * humidity * humidity) +
             (c9 * t * t * humidity * humidity);
    }

    return (hi - 32.0f) * 5.0f / 9.0f;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    (void)handler_args;
    (void)base;
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqtt_connected = true;
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            mqtt_connected = false;
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGD(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

static void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "Starting MQTT");

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

static void publish_task(void *params)
{
    (void)params;

    while (true) {
        float temp_sum = 0.0f;
        float hum_sum = 0.0f;
        float st_sum = 0.0f;
        int samples = 0;

        for (int i = 0; i < SAMPLE_COUNT; i++) {
            float humidity = 0.0f;
            float temperature = 0.0f;
            if (dht_read_float_data(DHT_TYPE, DHT_GPIO, &humidity, &temperature) == ESP_OK) {
                float st = calcular_sensacion_termica(temperature, humidity);
                temp_sum += temperature;
                hum_sum += humidity;
                st_sum += st;
                samples++;
            } else {
                ESP_LOGW(TAG, "DHT read failed");
            }
            vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
        }

        if (samples == 0) {
            ESP_LOGW(TAG, "No valid samples to publish");
            continue;
        }

        float temperature = temp_sum / samples;
        float humidity = hum_sum / samples;
        float sensacion_termica = st_sum / samples;

        if (mqtt_connected && client != NULL) {
            char temp_payload[16];
            char hum_payload[16];
            char st_payload[16];

            snprintf(temp_payload, sizeof(temp_payload), "%.2f", temperature);
            snprintf(hum_payload, sizeof(hum_payload), "%.2f", humidity);
            snprintf(st_payload, sizeof(st_payload), "%.2f", sensacion_termica);

            ESP_LOGI(TAG, "Publishing: temp=%s hum=%s st=%s", temp_payload, hum_payload, st_payload);

            esp_mqtt_client_publish(client, MQTT_TOPIC_TEMP, temp_payload, 0, 1, 0);
            esp_mqtt_client_publish(client, MQTT_TOPIC_HUM, hum_payload, 0, 1, 0);
            esp_mqtt_client_publish(client, MQTT_TOPIC_ST, st_payload, 0, 1, 0);
        } else {
            ESP_LOGW(TAG, "MQTT not connected, skipping publish");
        }
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    connect_wifi();

    if (wifi_connect_status) {
        mqtt_app_start();
        xTaskCreate(publish_task, "publish_task", 4096, NULL, 5, NULL);
    }
}
