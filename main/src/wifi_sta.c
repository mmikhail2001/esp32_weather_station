#include "wifi_sta.h"

#define TEST_WIFI_SSID "test-wifi"
#define TEST_WIFI_PASS "test-pass"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static const char *TAG = "wifi station";

static int retry_count = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT) {
    switch (event_id) {
    case WIFI_EVENT_STA_START:
      ESP_LOGI(TAG, "event WIFI_EVENT_STA_START");
      esp_wifi_connect();
      break;
    case WIFI_EVENT_STA_DISCONNECTED:
      ESP_LOGI(TAG, "event WIFI_EVENT_STA_DISCONNECTED");
      xEventGroupSetBits(net_event_group, WIFI_STA_NOT_CONNECTED);
      esp_wifi_connect();
      retry_count++;
      ESP_LOGI(TAG, "retry to connect to the AP, attempts: %d", retry_count);
      break;
    }
  } else if (event_base == IP_EVENT) {
    switch (event_id) {
    case IP_EVENT_STA_GOT_IP:
      xEventGroupSetBits(net_event_group, WIFI_STA_CONNECTED);
      ESP_LOGI(TAG, "event IP_EVENT_STA_GOT_IP");
      ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
      ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
      retry_count = 0;
      break;
    }
  }
}

void wifi_init_sta(void) {
  nvs_handle_t nvs_handle;
  ESP_ERROR_CHECK(nvs_open("app.storage", NVS_READWRITE, &nvs_handle));

  char ssid[32];
  char password[64];
  size_t ssid_len = sizeof(ssid);
  size_t password_len = sizeof(password);

  esp_err_t ssid_err =
      nvs_get_str(nvs_handle, "app.wifi.ssid", ssid, &ssid_len);
  esp_err_t pass_err =
      nvs_get_str(nvs_handle, "app.wifi.pass", password, &password_len);

  if (ssid_err != ESP_OK || pass_err != ESP_OK) {
    ESP_LOGI(TAG, "ssid and pass are not in nvs");

    strcpy(ssid, TEST_WIFI_SSID);
    strcpy(password, TEST_WIFI_PASS);

    // TODO: нужно ли записывать в NVS тестовые креды???  
    nvs_set_str(nvs_handle, "app.wifi.ssid", ssid);
    nvs_set_str(nvs_handle, "app.wifi.pass", password);
    nvs_commit(nvs_handle);
  } else {
    ESP_LOGI(TAG, "ssid and pass are in nvs: %s, %s", ssid, password);
  }

  nvs_close(nvs_handle);

  s_wifi_event_group = xEventGroupCreate();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

  wifi_config_t wifi_config = {
      .sta =
          {
              /* Authmode threshold resets to WPA2 as default if password
               * matches WPA2 standards (pasword len => 8). If you want to
               * connect the device to deprecated WEP/WPA networks, Please set
               * the threshold value to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set
               * the password with length and format matching to
               * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
               */
              .threshold.authmode = WIFI_AUTH_WPA2_PSK,
              .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
          },
  };

  strcpy((char *)wifi_config.sta.ssid, ssid);
  strcpy((char *)wifi_config.sta.password, password);

  lcd_data_t lcd_data;
  lcd_data.col = 10;
  lcd_data.row = 1;
  sprintf(lcd_data.str, "%.10s", wifi_config.sta.ssid);
  xQueueSendToBack(lcd_string_queue, &lcd_data, 0);
  vTaskDelay(100 / portTICK_PERIOD_MS);

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  // without this: Brownout detector was triggered
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "WIFI STA started, waiting events");
}
