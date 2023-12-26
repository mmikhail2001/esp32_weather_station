#include "wifi_ap.h"

static const char *TAG = "WIFI_AP";

#define AP_SSID "ESP32-AP-WIFI"
#define AP_PASS "12345678"
#define MAX_STA_CONNECTIONS 4

typedef struct {
  uint64_t mac;
  uint8_t is_connected;
} sta_info_t;

static sta_info_t sta_list[MAX_STA_CONNECTIONS];

static uint64_t mac_to_uint64(const uint8_t *mac) {
  uint64_t mac_val = 0;
  mac_val |= ((uint64_t)mac[0]) << 40;
  mac_val |= ((uint64_t)mac[1]) << 32;
  mac_val |= ((uint64_t)mac[2]) << 24;
  mac_val |= ((uint64_t)mac[3]) << 16;
  mac_val |= ((uint64_t)mac[4]) << 8;
  mac_val |= ((uint64_t)mac[5]);
  return mac_val;
}

static int find_sta(const uint8_t *mac) {
  uint64_t mac_val = mac_to_uint64(mac);
  for (int i = 0; i < MAX_STA_CONNECTIONS; i++) {
    if (sta_list[i].mac == mac_val && sta_list[i].is_connected == 1) {
      return i;
    }
  }
  return -1;
}

static int append_to_sta_list(const uint8_t *mac) {
  uint64_t mac_val = mac_to_uint64(mac);
  if (find_sta(mac) != -1) {
    return 1;
  }
  for (int i = 0; i < MAX_STA_CONNECTIONS; ++i) {
    if (sta_list[i].is_connected == 0) {
      sta_list[i].mac = mac_val;
      sta_list[i].is_connected = 1;
      return 0;
    }
  }
  return 2;
}

static int delete_from_sta_list(const uint8_t *mac) {
  uint64_t mac_val = mac_to_uint64(mac);

  for (int i = 0; i < MAX_STA_CONNECTIONS; ++i) {
    if (sta_list[i].mac == mac_val && sta_list[i].is_connected == 1) {
      sta_list[i].is_connected = 0;
      return 0;
    }
  }
  return 1;
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  switch (event_id) {
  case WIFI_EVENT_AP_STACONNECTED: {
    wifi_event_ap_staconnected_t *event =
        (wifi_event_ap_staconnected_t *)event_data;
    if (find_sta(event->mac) != -1) {
      ESP_LOGI(TAG, "event repeated WIFI_EVENT_AP_STACONNECTED");
      break;
    }
    if (append_to_sta_list(event->mac) != 0) {
      ESP_LOGI(TAG, "append sta while sta_list is full or sta already exists");
      break;
    }
    ESP_LOGI(TAG, "event WIFI_EVENT_AP_STACONNECTED");
    xEventGroupSetBits(net_event_group, STA_DEVICE_CONNECTED);
    ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac),
             event->aid);
    break;
  }
  case WIFI_EVENT_AP_STADISCONNECTED: {
    wifi_event_ap_stadisconnected_t *event =
        (wifi_event_ap_stadisconnected_t *)event_data;
    if (delete_from_sta_list(event->mac) != 0) {
      ESP_LOGI(TAG, "delete sta while sta in sta_list not found");
      break;
    }
    ESP_LOGI(TAG, "event WIFI_EVENT_AP_STADISCONNECTED");
    xEventGroupSetBits(net_event_group, STA_DEVICE_NOT_CONNECTED);
    ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac),
             event->aid);
    break;
  }
  case WIFI_EVENT_AP_START: {
    ESP_LOGI(TAG, "event WIFI_EVENT_AP_START");
    xEventGroupSetBits(net_event_group, WIFI_AP_STARTED);

    lcd_data_t lcd_data;
    lcd_data.col = 10;
    lcd_data.row = 2;
    sprintf(lcd_data.str, "p:%s", AP_PASS);
    xQueueSendToBack(lcd_string_queue, &lcd_data, 0);

    lcd_data.row = 3;
    sprintf(lcd_data.str, "ip    .4.1");
    xQueueSendToBack(lcd_string_queue, &lcd_data, 0);
    break;
  }
  case WIFI_EVENT_AP_STOP: {
    ESP_LOGI(TAG, "event WIFI_EVENT_AP_STOP");
    xEventGroupSetBits(net_event_group, WIFI_AP_STOPPED);

    lcd_data_t lcd_data;
    lcd_data.col = 10;
    lcd_data.row = 2;
    memset(lcd_data.str, ' ', 10);
    xQueueSendToBack(lcd_string_queue, &lcd_data, 0);
    lcd_data.row = 3;
    xQueueSendToBack(lcd_string_queue, &lcd_data, 0);
    break;
  }
  }
}

void wifi_init_softap() {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

  wifi_config_t wifi_config = {
      .ap = {.ssid = AP_SSID,
             .password = AP_PASS,
             .max_connection = MAX_STA_CONNECTIONS,
             .authmode = WIFI_AUTH_WPA_WPA2_PSK},
  };
  if (strlen(AP_PASS) == 0) {
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI(TAG, "WIFI AP started. SSID:%s password:%s\n\n", AP_SSID, AP_PASS);
}
