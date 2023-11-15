#include "wifi_ap.h"

static const char *TAG = "WIFI_AP";

#define AP_SSID "ESP32-AP-WIFI"
#define AP_PASS "12345678"

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    // TODO: http_server событие исчезает
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        xEventGroupSetBits(net_event_group, AP_CONNECTED);
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        xEventGroupClearBits(net_event_group, AP_CONNECTED);
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    } 
    else if (event_id == WIFI_EVENT_AP_START) 
    {
        ESP_LOGI(TAG, "event WIFI_EVENT_AP_START");

        lcd_data_t lcd_data;
        lcd_data.col = 10;
        lcd_data.row = 2;
        sprintf(lcd_data.str, "p:%s", AP_PASS);
        xQueueSendToBack(lcd_string_queue, &lcd_data, 0);

        lcd_data.row = 3;
        sprintf(lcd_data.str, "ip    .4.1");
        xQueueSendToBack(lcd_string_queue, &lcd_data, 0);
    }
    else if (event_id == WIFI_EVENT_AP_STOP) 
    {
        ESP_LOGI(TAG, "event WIFI_EVENT_AP_STOP");

        lcd_data_t lcd_data;
        lcd_data.col = 10;
        lcd_data.row = 2;
        memset(lcd_data.str, ' ', 10);
        xQueueSendToBack(lcd_string_queue, &lcd_data, 0);
        lcd_data.row = 3;
        xQueueSendToBack(lcd_string_queue, &lcd_data, 0);
    }
}

void wifi_init_softap()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .password = AP_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    if (strlen(AP_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "WIFI AP started. SSID:%s password:%s\n\n", AP_SSID, AP_PASS);
}