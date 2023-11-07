#pragma once

#include "lcd.h"

extern QueueHandle_t ws_send_queue;

void ws_send_task(void);
void ws_init();