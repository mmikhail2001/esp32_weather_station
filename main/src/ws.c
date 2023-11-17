#include "ws.h"

static const char *REMOTE_WS_SERVER_URI = "ws://212.233.91.39";
static const int REMOTE_WS_SERVER_PORT = 80;

static const char *TAG = "WEBSOCKET";
static esp_websocket_client_handle_t client;
extern QueueHandle_t ws_send_queue = NULL;

void collect_statistics() {
  // Собираем статистику о памяти
  uint32_t free_heap_size = esp_get_free_heap_size();
  uint32_t min_free_heap_size = esp_get_minimum_free_heap_size();

  // Собираем MAC-адрес устройства
  uint8_t mac_addr[6];
  esp_err_t mac_err = esp_efuse_mac_get_default(mac_addr);

  // Получаем информацию о чипе ESP32
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  // Получаем информацию о версии IDF
  const char *idf_version = esp_get_idf_version();

  // Получаем информацию о текущем ядре CPU
  // int core_id = esp_cpu_get_core_id();

  // Получаем указатель на вершину стека текущего ядра
  void *stack_pointer = esp_cpu_get_sp();

  // Получаем текущее значение счетчика циклов CPU
  // esp_cpu_cycle_count_t cycle_count = esp_cpu_get_cycle_count();

  // Получаем описание текущего приложения
  // const esp_app_desc_t *app_desc = esp_app_get_description();

  // Получаем хеш ELF-файла текущего приложения
  // char elf_sha256[65];  // SHA-256 хеш представлен в 64 символах плюс
  // завершающий нуль int sha_err = esp_app_get_elf_sha256(elf_sha256,
  // sizeof(elf_sha256));

  // Выводим собранную статистику
  ESP_LOGI(TAG, "Free Heap Size: %u bytes\n", free_heap_size);
  ESP_LOGI(TAG, "Minimum Free Heap Size: %u bytes\n", min_free_heap_size);

  if (mac_err == ESP_OK) {
    ESP_LOGI(TAG, "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr[0],
             mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  } else {
    ESP_LOGI(TAG, "Failed to retrieve MAC Address\n");
  }

  // printf("Chip Model: %s, Cores: %d, Revision: %d\n", chip_info.model ==
  // CHIP_ESP32 ? "ESP32" : "Unknown", chip_info.cores, chip_info.revision);
  // printf("IDF Version: %s\n", idf_version);
  // printf("Current Core ID: %d\n", core_id);
  // printf("Stack Pointer: %p\n", stack_pointer);
  // printf("Cycle Count: %llu\n", cycle_count);

  // if (app_desc) {
  //     printf("Application Name: %s\n", app_desc->project_name);
  //     printf("Application Version: %s\n", app_desc->version);
  //     printf("Application ID: %s\n", app_desc->id);
  // } else {
  //     printf("Failed to retrieve application description\n");
  // }

  // if (sha_err == ESP_OK) {
  //     printf("ELF SHA-256: %s\n", elf_sha256);
  // } else {
  //     printf("Failed to calculate ELF SHA-256\n");
  // }
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
    ESP_LOGI(TAG, "event WEBSOCKET_EVENT_DATA");
    ESP_LOGW(TAG, "Received from websocket=%.*s\n", data->data_len,
             (char *)data->data_ptr);
    if (strstr(data->data_ptr, "restart") != NULL) {
      ESP_LOGW(TAG, "CMD=%.*s\n", data->data_len, (char *)data->data_ptr);
      esp_restart();
    } else if (strstr(data->data_ptr, "stat") != NULL) {
      ESP_LOGW(TAG, "CMD=%.*s\n", data->data_len, (char *)data->data_ptr);
      collect_statistics();
    }
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

void ws_send_task(void) {
  lcd_data_t lcd_data;
  char data[20];
  while (true) {
    if (xQueueReceive(ws_send_queue, &data, portMAX_DELAY)) {
      ESP_LOGI(TAG, "to ws: %s, bool = %d", data, strcmp(data, "tmp"));
      if (strstr(data, "tmp") != NULL || strstr(data, "hum") != NULL ||
          strstr(data, "prs") != NULL || strstr(data, "gas") != NULL) {
        if (esp_websocket_client_is_connected(client)) {
          data[3] = ' ';
          esp_websocket_client_send(client, data, strlen(data), portMAX_DELAY);
        }
      }
    }
  }
  esp_websocket_client_stop(client);
  esp_websocket_client_destroy(client);
}
