#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"

#include "ws.h"

#define REMOTE_WS_URI "ws://212.233.91.39"
#define REMOTE_WS_PORT 80

static const char *TAG = "WEBSOCKET";
static esp_websocket_client_handle_t client;

extern QueueHandle_t ws_send_queue = NULL;

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGW(TAG, "Received=%.*s\n", data->data_len, (char *)data->data_ptr);
        break;
    }
}

void ws_init() {
    esp_websocket_client_config_t websocket_cfg = {};
    websocket_cfg.uri = REMOTE_WS_URI;
    websocket_cfg.port = REMOTE_WS_PORT;
    ESP_LOGI(TAG, "Connecting to %s ...", websocket_cfg.uri);

    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
}

void ws_send_task(void)
{
    lcd_data_t lcd_data;
    while (true)
    {
        if (xQueueReceive(ws_send_queue, &lcd_data, portMAX_DELAY))
        {
            if (esp_websocket_client_is_connected(client)) {
                esp_websocket_client_send(client, lcd_data.str, strlen(lcd_data.str), portMAX_DELAY);
            }
        }
    }

    esp_websocket_client_stop(client);
    ESP_LOGI(TAG, "Websocket Stopped");
    esp_websocket_client_destroy(client);
}

