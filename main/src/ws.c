#include "ws.h"

static const char *REMOTE_WS_SERVER_URI = "ws://212.233.91.39";
static const int REMOTE_WS_SERVER_PORT = 80;

static const char *TAG = "WEBSOCKET";
static esp_websocket_client_handle_t client;
extern QueueHandle_t ws_send_sensors_queue = NULL;
extern QueueHandle_t ws_send_stats_queue = NULL;

typedef struct {
  uint8_t mac_addr[6];
  char model[32];
  int core_count;
  unsigned int silicon_revision_major;
  unsigned int silicon_revision_minor;
  int cpu_frequency_mhz;
  int free_heap_bytes;
  int minimum_free_heap_bytes;
  int flash_size_mb;
  char esp_idf_version[32];
  int64_t uptime_seconds;
  char elf_sha256_str[65];
} system_info_t;

void collect_statistics(system_info_t *info) {
  esp_err_t mac_err = esp_efuse_mac_get_default(info->mac_addr);
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  strncpy(info->model, (chip_info.model == CHIP_ESP32) ? "ESP32" : "Unknown",
          sizeof(info->model));
  info->core_count = chip_info.cores;
  info->silicon_revision_major = chip_info.full_revision / 100;
  info->silicon_revision_minor = chip_info.full_revision % 100;
  info->cpu_frequency_mhz = esp_clk_cpu_freq() / 1000000;
  info->free_heap_bytes = esp_get_free_heap_size();
  info->minimum_free_heap_bytes = esp_get_minimum_free_heap_size();
  size_t flash_size = spi_flash_get_chip_size();
  info->flash_size_mb = flash_size / (1024 * 1024);
  strncpy(info->esp_idf_version, esp_get_idf_version(),
          sizeof(info->esp_idf_version));
  info->uptime_seconds = esp_timer_get_time() / 1000000;

  const esp_app_desc_t *app_desc = esp_ota_get_app_description();

  if (app_desc != NULL) {
    sprintf(info->elf_sha256_str, "%s", app_desc->app_elf_sha256);
  }
}

char *system_info_to_json(const system_info_t *info) {
  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "mac_address", (const char *)info->mac_addr);
  cJSON_AddStringToObject(root, "model", info->model);
  cJSON_AddNumberToObject(root, "core_count", info->core_count);
  cJSON_AddNumberToObject(root, "silicon_revision_major",
                          info->silicon_revision_major);
  cJSON_AddNumberToObject(root, "silicon_revision_minor",
                          info->silicon_revision_minor);
  cJSON_AddNumberToObject(root, "cpu_frequency_mhz", info->cpu_frequency_mhz);
  cJSON_AddNumberToObject(root, "free_heap_bytes", info->free_heap_bytes);
  cJSON_AddNumberToObject(root, "minimum_free_heap_bytes",
                          info->minimum_free_heap_bytes);
  cJSON_AddNumberToObject(root, "flash_size_mb", info->flash_size_mb);
  cJSON_AddStringToObject(root, "esp_idf_version", info->esp_idf_version);
  cJSON_AddNumberToObject(root, "uptime_seconds", info->uptime_seconds);
  cJSON_AddStringToObject(root, "elf_sha256", info->elf_sha256_str);

  char *json_str = cJSON_PrintUnformatted(root);
  cJSON_Delete(root);
  return json_str;
}

static void websocket_event_handler(void *handler_args, esp_event_base_t base,
                                    int32_t event_id, void *event_data) {
  esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
  switch (event_id) {
  case WEBSOCKET_EVENT_DISCONNECTED:
  case WEBSOCKET_EVENT_CLOSED: {
    ESP_LOGI(TAG,
             "event WEBSOCKET_EVENT_DISCONNECTED or WEBSOCKET_EVENT_CLOSED");
    xEventGroupSetBits(net_event_group, WS_SERVER_NOT_CONNECTED);
    break;
  }
  case WEBSOCKET_EVENT_CONNECTED: {
    xEventGroupSetBits(net_event_group, WS_SERVER_CONNECTED);
    ESP_LOGI(TAG, "event WEBSOCKET_EVENT_CONNECTED");
    break;
  }
  case WEBSOCKET_EVENT_DATA: {

    static char buf[20];
    memset(buf, 0, sizeof(buf));

    strncpy(buf, (char *)data->data_ptr, sizeof(buf) - 1);
    if (data->data_len >= 20) {
      buf[sizeof(buf) - 1] = '\0';
    } else {
      buf[data->data_len] = '\0';
    }

    ESP_LOGI(TAG, "event WEBSOCKET_EVENT_DATA");
    ESP_LOGW(TAG, "Received from websocket=%.*s\n", data->data_len,
             (char *)data->data_ptr);
    if (strstr(buf, "restart") != NULL) {
      ESP_LOGW(TAG, "CMD=%.*s\n", data->data_len, (char *)data->data_ptr);
      esp_restart();
    } else if (strstr(buf, "stat") != NULL) {
      ESP_LOGW(TAG, "CMD=%.*s\n", data->data_len, (char *)data->data_ptr);
      system_info_t info;
      collect_statistics(&info);
      char *json_str = system_info_to_json(&info);

      char json_static[512];

      if (json_str != NULL) {
        strncpy(json_static, json_str, sizeof(json_static));
        free(json_str);
      } else {
        strcpy(json_static, "error");
      }
      json_static[strlen(json_static)] = '\0';
      xQueueSendToBack(ws_send_stats_queue, &json_static, 0);
    }
    memset(data, 0, sizeof(*data));
    break;
  }
  }
}

void ws_init() {
  esp_websocket_client_config_t websocket_cfg = {};
  websocket_cfg.uri = REMOTE_WS_SERVER_URI;
  websocket_cfg.port = REMOTE_WS_SERVER_PORT;
  client = esp_websocket_client_init(&websocket_cfg);
  esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY,
                                websocket_event_handler, (void *)client);
}

void ws_start() {
  ESP_LOGI(TAG, "connecting to ws server... uri: %s, port: %d",
           REMOTE_WS_SERVER_URI, REMOTE_WS_SERVER_PORT);
  esp_websocket_client_start(client);
  ESP_LOGI(TAG, "client and server connection started");
}

void ws_stop() {
  esp_websocket_client_stop(client);
  ESP_LOGI(TAG, "client and server connection stopped");
}

void ws_send_sensors_data_task(void) {
  char data_sensors[20];
  while (true) {
    if (xQueueReceive(ws_send_sensors_queue, &data_sensors, portMAX_DELAY)) {
      if (esp_websocket_client_is_connected(client)) {
        esp_websocket_client_send(client, data_sensors, strlen(data_sensors),
                                  portMAX_DELAY);
      }
    }
  }
  esp_websocket_client_stop(client);
  esp_websocket_client_destroy(client);
}

void ws_send_stats_task(void) {
  char data_stats[512];
  while (true) {
    if (xQueueReceive(ws_send_stats_queue, &data_stats, portMAX_DELAY)) {
      if (esp_websocket_client_is_connected(client)) {
        esp_websocket_client_send(client, data_stats, strlen(data_stats),
                                  portMAX_DELAY);
      }
    }
  }
  esp_websocket_client_stop(client);
  esp_websocket_client_destroy(client);
}
