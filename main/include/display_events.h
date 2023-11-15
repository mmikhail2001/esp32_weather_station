#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "lcd.h"

extern EventGroupHandle_t net_event_group;

// #define STA_DISCONNECTED BIT0
#define DEFAULT_SET BIT0
#define STA_CONNECTED BIT1
#define WS_SENDING BIT2
// #define AP_DISCONNECTED BIT3
#define AP_CONNECTED BIT4
#define HTTP_STARTED BIT5

void display_info_task();