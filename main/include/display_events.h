#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "lcd.h"

extern EventGroupHandle_t net_event_group;

#define  WIFI_STA_CONNECTED BIT0
#define  WIFI_STA_NOT_CONNECTED BIT1

#define  WS_SERVER_CONNECTED BIT2
#define  WS_SERVER_NOT_CONNECTED BIT3

#define  WIFI_AP_STARTED BIT4
#define  WIFI_AP_STOPPED BIT5

#define  STA_DEVICE_CONNECTED BIT6
#define  STA_DEVICE_NOT_CONNECTED BIT7


void display_info_task();
