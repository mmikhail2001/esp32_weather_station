#pragma once

#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_http_server.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/timers.h"

#include "display_events.h"

httpd_handle_t start_http_server();
extern EventGroupHandle_t net_event_group;