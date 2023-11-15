#pragma once


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

#include "lcd.h"
#include "display_events.h"

extern QueueHandle_t ws_send_queue;
extern EventGroupHandle_t net_event_group;

void ws_send_task(void);
void ws_init();
void ws_start();
void ws_stop();