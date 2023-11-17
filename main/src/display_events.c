#include "display_events.h"

extern EventGroupHandle_t net_event_group = NULL;
static const char *TAG = "DISPLAY_EVENTS";
static int STA_STATE = 0;
static int AP_STATE = 0;

void display_info_task(void) {
  EventBits_t old_bits = 0;
  while (true) {
    EventBits_t bits = xEventGroupWaitBits(
        net_event_group,
        WIFI_STA_CONNECTED | WIFI_STA_NOT_CONNECTED | WS_SERVER_CONNECTED |
            WS_SERVER_NOT_CONNECTED | WIFI_AP_STARTED | WIFI_AP_STOPPED |
            STA_DEVICE_CONNECTED | STA_DEVICE_NOT_CONNECTED,
        pdTRUE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_STA_CONNECTED) {
      STA_STATE = 1;
    } else if (bits & WIFI_STA_NOT_CONNECTED) {
      STA_STATE = 0;
    } else if (bits & WS_SERVER_CONNECTED) {
      STA_STATE = 2;
    } else if (bits & WS_SERVER_NOT_CONNECTED && STA_STATE == 2) {
      STA_STATE = 1;
    }

    if (bits & WIFI_AP_STARTED) {
      AP_STATE = 1;
    } else if (bits & WIFI_AP_STOPPED) {
      AP_STATE = 0;
    } else if (bits & STA_DEVICE_CONNECTED) {
      ++AP_STATE;
    } else if (bits & STA_DEVICE_NOT_CONNECTED && AP_STATE >= 2) {
      --AP_STATE;
    }

    lcd_data_t lcd_data = {
        .col = 17,
        .row = 0,
    };
    sprintf(lcd_data.str, "%d/%d", STA_STATE, AP_STATE);
    xQueueSendToBack(lcd_string_queue, &lcd_data, 0);
  }
}
