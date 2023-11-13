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

void wifi_init_sta(void);
