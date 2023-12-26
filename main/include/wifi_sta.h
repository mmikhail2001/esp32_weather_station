#pragma once 

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "display_events.h"
#include "lcd.h"

extern EventGroupHandle_t net_event_group;
extern QueueHandle_t lcd_string_queue;

void wifi_init_sta(void);
