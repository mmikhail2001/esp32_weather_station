#include "ws.h"

static const char *remote_ws_uri = "ws://212.233.91.39";
static const int remote_ws_port = 80;

static const char *TAG = "WEBSOCKET";
static esp_websocket_client_handle_t client;
extern QueueHandle_t ws_send_queue = NULL;

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "event WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "event WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        // TODO:
        ESP_LOGW(TAG, "Received=%.*s\n", data->data_len, (char *)data->data_ptr);
        break;
    case WEBSOCKET_EVENT_CLOSED:
        ESP_LOGI(TAG, "event WEBSOCKET_EVENT_CLOSED");
        break;
    }
}

void ws_init() {
    esp_websocket_client_config_t websocket_cfg = {};
    websocket_cfg.uri = remote_ws_uri;
    websocket_cfg.port = remote_ws_port;
    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
}

void ws_start() {
    ESP_LOGI(TAG, "connecting to ws server... uri: %s, port: %d", remote_ws_uri, remote_ws_port);
    esp_websocket_client_start(client);
    ESP_LOGI(TAG, "client and server connection started");
}

void ws_stop() {
    esp_websocket_client_stop(client);
    ESP_LOGI(TAG, "client and server connection stopped");
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
    esp_websocket_client_destroy(client);
}

