#pragma once 

#include <stdio.h>
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lcd.h"

extern QueueHandle_t lcd_string_queue;

void mq135_init(adc_channel_t apin, gpio_num_t gpin);
void mq135_read_task(void *arg);
