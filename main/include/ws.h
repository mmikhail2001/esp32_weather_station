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


#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_efuse.h"
#include "esp_chip_info.h"
#include "esp_cpu.h"
#include "esp_clk.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include <esp_partition.h>
#include "esp_ota_ops.h"

#include "cJSON.h"

#include "lcd.h"
#include "display_events.h"

extern QueueHandle_t ws_send_sensors_queue;
extern QueueHandle_t ws_send_stats_queue;
extern EventGroupHandle_t net_event_group;

void ws_send_sensors_data_task(void);
void ws_send_stats_task(void);
void ws_init();
void ws_start();
void ws_stop();
