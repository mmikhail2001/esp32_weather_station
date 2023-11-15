#pragma once

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_wifi.h"


#include "i2c.h"
#include "lcd.h"
#include "dht11.h"
#include "wifi_ap.h"
#include "wifi_sta.h"
#include "bmx280.h"
#include "mq135.h"
#include "ws.h"
#include "http_server.h"
#include "display_events.h"

extern EventGroupHandle_t net_event_group;

