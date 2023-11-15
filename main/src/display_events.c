#include "display_events.h"

extern EventGroupHandle_t net_event_group = NULL;
static const char *TAG = "DISPLAY_EVENTS";


void display_info_task(void) {
    EventBits_t old_bits = 0;
    while (true) {
        EventBits_t bits = xEventGroupWaitBits(net_event_group, 
            DEFAULT_SET | STA_CONNECTED | WS_SENDING | AP_CONNECTED | HTTP_STARTED,
            // pdTRUE,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);
        EventBits_t edge_triggered_bits = bits ^ old_bits;
        old_bits = bits;

        if (edge_triggered_bits == 0) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }
        
        // ESP_LOGI(TAG, "%x\n", bits);

        int sta_state = 0;
        int ap_state = 0;
        
        if (edge_triggered_bits & WS_SENDING) {
            if (bits & WS_SENDING) {
                sta_state = 2;
            } else {
                sta_state = 1;
            }
        } else if (edge_triggered_bits & STA_CONNECTED) {
            if (bits & STA_CONNECTED) {
                sta_state = 1;
            } else {
                sta_state = 0;
            }
        }
        
        if (edge_triggered_bits & HTTP_STARTED) {
            if (bits & HTTP_STARTED) {
                ap_state = 2;
            } else {
                ap_state = 1;
            }
        } else if (edge_triggered_bits & AP_CONNECTED) {
            if (bits & AP_CONNECTED) {
                ap_state = 1;
            } else {
                ap_state = 0;
            }
        }

        // if (bits & STA_DISCONNECTED) {
        //     sta_state = 0;
        // } else if (bits & STA_CONNECTED) {
        //     sta_state = 1;
        // } else if (bits & WS_SENDING) {
        //     sta_state = 2;
        // }
        
        // if (bits & AP_DISCONNECTED) {
        //     ap_state = 0;
        // } else if (bits & AP_CONNECTED) {
        //     ap_state = 1;
        // } else if (bits & HTTP_STARTED) {
        //     ap_state = 2;
        // }
        
        
        lcd_data_t lcd_data;
        lcd_data.col = 17;
        lcd_data.row = 0;
        sprintf(lcd_data.str, "%d/%d", sta_state, ap_state);
        xQueueSendToBack(lcd_string_queue, &lcd_data, 0);
    }
}
