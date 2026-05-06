#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "driver/gpio.h"
#include "dht.h"

#define WIFI_SSID "Brayan’s iPhone"
#define WIFI_PASS "hola66669"
#define WIFI_MAX_RETRY 5

#define DHT_GPIO 4 
#define DHT_TYPE DHT_TYPE_AM2301

#define SAMPLE_INTERVAL_MS 2000
#define SAMPLE_COUNT 10

#define THINGSPEAK_BASE_URL "http://api.thingspeak.com"
#define THINGSPEAK_API_KEY "QVPS5UB26NU18CGZ"

static const char *TAG = "THINGSPEAK";

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

float calcular_sensacion_termica(float temp_c, float humidity)
{
    float t = (temp_c * 9.0f / 5.0f) + 32.0f;
    float hi;

    hi = 0.5f * (t + 61.0f + ((t - 68.0f) * 1.2f) + (humidity * 0.094f));

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

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retrying WiFi connection");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "WiFi connection failed");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

static bool wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    bool connected = (bits & WIFI_CONNECTED_BIT) != 0;
    if (connected) {
        ESP_LOGI(TAG, "connected to WiFi SSID:%s", WIFI_SSID);
    } else {
        ESP_LOGE(TAG, "failed to connect to WiFi SSID:%s", WIFI_SSID);
    }

    return connected;
}

static esp_err_t thingspeak_send(esp_http_client_handle_t client, float temperature, float humidity, float heat_index)
{
    char url[256];
    snprintf(url, sizeof(url), "%s/update?api_key=%s&field1=%.1f&field2=%.0f&field3=%.0f",
             THINGSPEAK_BASE_URL, THINGSPEAK_API_KEY, temperature, humidity, heat_index);

    esp_http_client_set_url(client, url);
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        return err;
    }

    int status_code = esp_http_client_get_status_code(client);
    if (status_code != 200) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

static esp_err_t thingspeak_read_coworker(float *temp, float *hum, float *hi)
{
    esp_http_client_config_t config = {
        .url = "http://api.thingspeak.com/channels/2941224/feeds/last.json?api_key=Z1OY8Z0WQ3RIT2XC",
        .method = HTTP_METHOD_GET,
        .timeout_ms = 5000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    esp_err_t err = esp_http_client_open(client, 0);
    if (err == ESP_OK) {
        esp_http_client_fetch_headers(client);
        char buffer[512] = {0};
        int read_len = esp_http_client_read(client, buffer, sizeof(buffer) - 1);
        if (read_len > 0) {
            buffer[read_len] = '\0';
            char *p1 = strstr(buffer, "\"field1\":\"");
            char *p2 = strstr(buffer, "\"field2\":\"");
            char *p3 = strstr(buffer, "\"field3\":\"");
            if (p1) sscanf(p1, "\"field1\":\"%f\"", temp);
            if (p2) sscanf(p2, "\"field2\":\"%f\"", hum);
            if (p3) sscanf(p3, "\"field3\":\"%f\"", hi);
        }
        esp_http_client_close(client);
    }
    esp_http_client_cleanup(client);
    return err;
}

static void thingspeak_task(void *pvParameters)
{
    (void)pvParameters;

    esp_http_client_config_t config = {
        .url = THINGSPEAK_BASE_URL,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");

    while (true) {
        float temp_sum = 0.0f;
        float hum_sum = 0.0f;
        float hi_sum = 0.0f;
        int samples = 0;

        for (int i = 0; i < SAMPLE_COUNT; i++) {
            float humidity = 0.0f;
            float temperature = 0.0f;
            if (dht_read_float_data(DHT_TYPE, DHT_GPIO, &humidity, &temperature) == ESP_OK) {
                float heat_index = calcular_sensacion_termica(temperature, humidity);
                temp_sum += temperature;
                hum_sum += humidity;
                hi_sum += heat_index;
                samples++;
            } else {
                ESP_LOGW(TAG, "DHT read failed");
            }
            vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
        }

        if (samples > 0) {
            float temperature = temp_sum / samples;
            float humidity = hum_sum / samples;
            float heat_index = hi_sum / samples;

            float j_temp = 0.0f, j_hum = 0.0f, j_hi = 0.0f;
            thingspeak_read_coworker(&j_temp, &j_hum, &j_hi);

            ESP_LOGI(TAG, "data: %.1f,%.0f,%.0f | %.1f,%.0f,%.0f", temperature, humidity, heat_index, j_temp, j_hum, j_hi);

            esp_err_t err = thingspeak_send(client, temperature, humidity, heat_index);
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "ThingSpeak update ok");
            } else {
                ESP_LOGE(TAG, "ThingSpeak update failed: %s", esp_err_to_name(err));
            }

        } else {
            ESP_LOGW(TAG, "no valid samples");
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

    if (!wifi_init_sta()) {
        return;
    }

    xTaskCreate(thingspeak_task, "thingspeak_task", 8192, NULL, 5, NULL);
}
