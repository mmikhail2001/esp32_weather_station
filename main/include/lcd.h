#pragma once 

#include "esp_log.h"
#include "i2c.h"

typedef struct {
    uint8_t row;
    uint8_t col;
    char str[20];
} lcd_data_t;

// TODO: уже есть в файле ws.h
extern QueueHandle_t ws_send_queue;
extern QueueHandle_t lcd_string_queue;

void lcd_process_queue_task(void *arg);
void lcd_init(i2c_port_t);
void lcd_clear(void);
