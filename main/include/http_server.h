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

inline static const char index_html[] = {
    0x3C, 0x68, 0x74, 0x6D, 0x6C, 0x3E, 0x0D, 0x0A, 0x3C, 0x68, 0x65, 0x61, 0x64, 0x3E, 0x0D,
    0x0A, 0x20, 0x20, 0x20, 0x20, 0x3C, 0x74, 0x69, 0x74, 0x6C, 0x65, 0x3E, 0x45, 0x53, 0x50,
    0x33, 0x32, 0x3C, 0x2F, 0x74, 0x69, 0x74, 0x6C, 0x65, 0x3E, 0x0D, 0x0A, 0x3C, 0x2F, 0x68,
    0x65, 0x61, 0x64, 0x3E, 0x0D, 0x0A, 0x3C, 0x62, 0x6F, 0x64, 0x79, 0x3E, 0x0D, 0x0A, 0x20,
    0x20, 0x20, 0x20, 0x3C, 0x66, 0x6F, 0x72, 0x6D, 0x20, 0x6F, 0x6E, 0x73, 0x75, 0x62, 0x6D,
    0x69, 0x74, 0x3D, 0x22, 0x73, 0x75, 0x62, 0x6D, 0x69, 0x74, 0x46, 0x6F, 0x72, 0x6D, 0x28,
    0x65, 0x76, 0x65, 0x6E, 0x74, 0x29, 0x22, 0x3E, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x4E, 0x65, 0x77, 0x20, 0x53, 0x53, 0x49, 0x44, 0x3A, 0x20, 0x3C, 0x69,
    0x6E, 0x70, 0x75, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3D, 0x22, 0x74, 0x65, 0x78, 0x74,
    0x22, 0x20, 0x6E, 0x61, 0x6D, 0x65, 0x3D, 0x22, 0x73, 0x73, 0x69, 0x64, 0x22, 0x3E, 0x3C,
    0x62, 0x72, 0x3E, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x50, 0x61,
    0x73, 0x73, 0x77, 0x6F, 0x72, 0x64, 0x3A, 0x20, 0x3C, 0x69, 0x6E, 0x70, 0x75, 0x74, 0x20,
    0x74, 0x79, 0x70, 0x65, 0x3D, 0x22, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64, 0x22,
    0x20, 0x6E, 0x61, 0x6D, 0x65, 0x3D, 0x22, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64,
    0x22, 0x3E, 0x3C, 0x62, 0x72, 0x3E, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x41, 0x63, 0x63, 0x65, 0x73, 0x73, 0x20, 0x6B, 0x65, 0x79, 0x3A, 0x20, 0x3C, 0x69,
    0x6E, 0x70, 0x75, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3D, 0x22, 0x70, 0x61, 0x73, 0x73,
    0x77, 0x6F, 0x72, 0x64, 0x22, 0x20, 0x6E, 0x61, 0x6D, 0x65, 0x3D, 0x22, 0x61, 0x63, 0x63,
    0x65, 0x73, 0x73, 0x5F, 0x6B, 0x65, 0x79, 0x22, 0x3E, 0x3C, 0x62, 0x72, 0x3E, 0x0D, 0x0A,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3C, 0x69, 0x6E, 0x70, 0x75, 0x74, 0x20,
    0x74, 0x79, 0x70, 0x65, 0x3D, 0x22, 0x73, 0x75, 0x62, 0x6D, 0x69, 0x74, 0x22, 0x20, 0x76,
    0x61, 0x6C, 0x75, 0x65, 0x3D, 0x22, 0x53, 0x75, 0x62, 0x6D, 0x69, 0x74, 0x22, 0x3E, 0x0D,
    0x0A, 0x20, 0x20, 0x20, 0x20, 0x3C, 0x2F, 0x66, 0x6F, 0x72, 0x6D, 0x3E, 0x0D, 0x0A, 0x20,
    0x20, 0x20, 0x20, 0x3C, 0x64, 0x69, 0x76, 0x20, 0x69, 0x64, 0x3D, 0x22, 0x6D, 0x65, 0x73,
    0x73, 0x61, 0x67, 0x65, 0x22, 0x3E, 0x3C, 0x2F, 0x64, 0x69, 0x76, 0x3E, 0x0D, 0x0A, 0x20,
    0x20, 0x20, 0x20, 0x3C, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x3E, 0x0D, 0x0A, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20,
    0x73, 0x75, 0x62, 0x6D, 0x69, 0x74, 0x46, 0x6F, 0x72, 0x6D, 0x28, 0x65, 0x76, 0x65, 0x6E,
    0x74, 0x29, 0x20, 0x7B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x2E, 0x70, 0x72, 0x65, 0x76, 0x65, 0x6E,
    0x74, 0x44, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74, 0x28, 0x29, 0x3B, 0x0D, 0x0A, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x63, 0x6F, 0x6E, 0x73, 0x74,
    0x20, 0x73, 0x73, 0x69, 0x64, 0x20, 0x3D, 0x20, 0x64, 0x6F, 0x63, 0x75, 0x6D, 0x65, 0x6E,
    0x74, 0x2E, 0x67, 0x65, 0x74, 0x45, 0x6C, 0x65, 0x6D, 0x65, 0x6E, 0x74, 0x73, 0x42, 0x79,
    0x4E, 0x61, 0x6D, 0x65, 0x28, 0x27, 0x73, 0x73, 0x69, 0x64, 0x27, 0x29, 0x5B, 0x30, 0x5D,
    0x2E, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x63, 0x6F, 0x6E, 0x73, 0x74, 0x20, 0x70, 0x61, 0x73,
    0x73, 0x77, 0x6F, 0x72, 0x64, 0x20, 0x3D, 0x20, 0x64, 0x6F, 0x63, 0x75, 0x6D, 0x65, 0x6E,
    0x74, 0x2E, 0x67, 0x65, 0x74, 0x45, 0x6C, 0x65, 0x6D, 0x65, 0x6E, 0x74, 0x73, 0x42, 0x79,
    0x4E, 0x61, 0x6D, 0x65, 0x28, 0x27, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64, 0x27,
    0x29, 0x5B, 0x30, 0x5D, 0x2E, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3B, 0x0D, 0x0A, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x63, 0x6F, 0x6E, 0x73, 0x74,
    0x20, 0x61, 0x63, 0x63, 0x65, 0x73, 0x73, 0x5F, 0x6B, 0x65, 0x79, 0x20, 0x3D, 0x20, 0x64,
    0x6F, 0x63, 0x75, 0x6D, 0x65, 0x6E, 0x74, 0x2E, 0x67, 0x65, 0x74, 0x45, 0x6C, 0x65, 0x6D,
    0x65, 0x6E, 0x74, 0x73, 0x42, 0x79, 0x4E, 0x61, 0x6D, 0x65, 0x28, 0x27, 0x61, 0x63, 0x63,
    0x65, 0x73, 0x73, 0x5F, 0x6B, 0x65, 0x79, 0x27, 0x29, 0x5B, 0x30, 0x5D, 0x2E, 0x76, 0x61,
    0x6C, 0x75, 0x65, 0x3B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x63, 0x6F, 0x6E, 0x73, 0x74, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67,
    0x65, 0x45, 0x6C, 0x65, 0x6D, 0x65, 0x6E, 0x74, 0x20, 0x3D, 0x20, 0x64, 0x6F, 0x63, 0x75,
    0x6D, 0x65, 0x6E, 0x74, 0x2E, 0x67, 0x65, 0x74, 0x45, 0x6C, 0x65, 0x6D, 0x65, 0x6E, 0x74,
    0x42, 0x79, 0x49, 0x64, 0x28, 0x27, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x27, 0x29,
    0x3B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x66, 0x65, 0x74, 0x63, 0x68, 0x28, 0x27, 0x2F, 0x6C, 0x6F, 0x67, 0x69, 0x6E, 0x3F, 0x73,
    0x73, 0x69, 0x64, 0x3D, 0x27, 0x20, 0x2B, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x55,
    0x52, 0x49, 0x43, 0x6F, 0x6D, 0x70, 0x6F, 0x6E, 0x65, 0x6E, 0x74, 0x28, 0x73, 0x73, 0x69,
    0x64, 0x29, 0x20, 0x2B, 0x20, 0x27, 0x26, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64,
    0x3D, 0x27, 0x20, 0x2B, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x55, 0x52, 0x49, 0x43,
    0x6F, 0x6D, 0x70, 0x6F, 0x6E, 0x65, 0x6E, 0x74, 0x28, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6F,
    0x72, 0x64, 0x29, 0x20, 0x2B, 0x20, 0x27, 0x26, 0x61, 0x63, 0x63, 0x65, 0x73, 0x73, 0x5F,
    0x6B, 0x65, 0x79, 0x3D, 0x27, 0x20, 0x2B, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x55,
    0x52, 0x49, 0x43, 0x6F, 0x6D, 0x70, 0x6F, 0x6E, 0x65, 0x6E, 0x74, 0x28, 0x61, 0x63, 0x63,
    0x65, 0x73, 0x73, 0x5F, 0x6B, 0x65, 0x79, 0x29, 0x29, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2E, 0x74, 0x68,
    0x65, 0x6E, 0x28, 0x72, 0x65, 0x73, 0x70, 0x6F, 0x6E, 0x73, 0x65, 0x20, 0x3D, 0x3E, 0x20,
    0x7B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x69, 0x66, 0x20, 0x28, 0x72, 0x65, 0x73,
    0x70, 0x6F, 0x6E, 0x73, 0x65, 0x2E, 0x73, 0x74, 0x61, 0x74, 0x75, 0x73, 0x20, 0x3D, 0x3D,
    0x3D, 0x20, 0x32, 0x30, 0x30, 0x29, 0x20, 0x7B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x45, 0x6C, 0x65, 0x6D,
    0x65, 0x6E, 0x74, 0x2E, 0x69, 0x6E, 0x6E, 0x65, 0x72, 0x48, 0x54, 0x4D, 0x4C, 0x20, 0x3D,
    0x20, 0x27, 0x44, 0x61, 0x74, 0x61, 0x20, 0x61, 0x70, 0x70, 0x6C, 0x69, 0x65, 0x64, 0x20,
    0x73, 0x75, 0x63, 0x63, 0x65, 0x73, 0x73, 0x66, 0x75, 0x6C, 0x6C, 0x79, 0x2E, 0x27, 0x3B,
    0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x6D, 0x65, 0x73, 0x73,
    0x61, 0x67, 0x65, 0x45, 0x6C, 0x65, 0x6D, 0x65, 0x6E, 0x74, 0x2E, 0x73, 0x74, 0x79, 0x6C,
    0x65, 0x2E, 0x62, 0x61, 0x63, 0x6B, 0x67, 0x72, 0x6F, 0x75, 0x6E, 0x64, 0x43, 0x6F, 0x6C,
    0x6F, 0x72, 0x20, 0x3D, 0x20, 0x27, 0x67, 0x72, 0x65, 0x65, 0x6E, 0x27, 0x3B, 0x0D, 0x0A,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x73, 0x65, 0x74, 0x54, 0x69, 0x6D,
    0x65, 0x6F, 0x75, 0x74, 0x28, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x28,
    0x29, 0x20, 0x7B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x45, 0x6C, 0x65, 0x6D, 0x65,
    0x6E, 0x74, 0x2E, 0x69, 0x6E, 0x6E, 0x65, 0x72, 0x48, 0x54, 0x4D, 0x4C, 0x20, 0x3D, 0x20,
    0x27, 0x27, 0x3B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x7D,
    0x2C, 0x20, 0x33, 0x30, 0x30, 0x30, 0x29, 0x3B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x7D, 0x20, 0x65, 0x6C, 0x73, 0x65, 0x20, 0x69, 0x66, 0x20, 0x28, 0x72, 0x65, 0x73, 0x70,
    0x6F, 0x6E, 0x73, 0x65, 0x2E, 0x73, 0x74, 0x61, 0x74, 0x75, 0x73, 0x20, 0x3D, 0x3D, 0x3D,
    0x20, 0x34, 0x30, 0x33, 0x29, 0x20, 0x7B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x45, 0x6C, 0x65, 0x6D, 0x65,
    0x6E, 0x74, 0x2E, 0x69, 0x6E, 0x6E, 0x65, 0x72, 0x48, 0x54, 0x4D, 0x4C, 0x20, 0x3D, 0x20,
    0x27, 0x54, 0x68, 0x65, 0x20, 0x6B, 0x65, 0x79, 0x20, 0x69, 0x73, 0x20, 0x69, 0x6E, 0x63,
    0x6F, 0x72, 0x72, 0x65, 0x63, 0x74, 0x27, 0x3B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x45, 0x6C, 0x65, 0x6D,
    0x65, 0x6E, 0x74, 0x2E, 0x73, 0x74, 0x79, 0x6C, 0x65, 0x2E, 0x62, 0x61, 0x63, 0x6B, 0x67,
    0x72, 0x6F, 0x75, 0x6E, 0x64, 0x43, 0x6F, 0x6C, 0x6F, 0x72, 0x20, 0x3D, 0x20, 0x27, 0x72,
    0x65, 0x64, 0x27, 0x3B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x73, 0x65, 0x74, 0x54, 0x69, 0x6D, 0x65, 0x6F, 0x75, 0x74, 0x28, 0x66, 0x75, 0x6E, 0x63,
    0x74, 0x69, 0x6F, 0x6E, 0x20, 0x28, 0x29, 0x20, 0x7B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67,
    0x65, 0x45, 0x6C, 0x65, 0x6D, 0x65, 0x6E, 0x74, 0x2E, 0x69, 0x6E, 0x6E, 0x65, 0x72, 0x48,
    0x54, 0x4D, 0x4C, 0x20, 0x3D, 0x20, 0x27, 0x27, 0x3B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x7D, 0x2C, 0x20, 0x33, 0x30, 0x30, 0x30, 0x29, 0x3B, 0x0D,
    0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x7D, 0x20, 0x65, 0x6C, 0x73, 0x65, 0x20, 0x7B, 0x0D,
    0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x64, 0x6F, 0x63, 0x75, 0x6D,
    0x65, 0x6E, 0x74, 0x2E, 0x67, 0x65, 0x74, 0x45, 0x6C, 0x65, 0x6D, 0x65, 0x6E, 0x74, 0x42,
    0x79, 0x49, 0x64, 0x28, 0x27, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x27, 0x29, 0x2E,
    0x69, 0x6E, 0x6E, 0x65, 0x72, 0x48, 0x54, 0x4D, 0x4C, 0x20, 0x3D, 0x20, 0x27, 0x44, 0x61,
    0x74, 0x61, 0x20, 0x6E, 0x6F, 0x74, 0x20, 0x61, 0x70, 0x70, 0x6C, 0x69, 0x65, 0x64, 0x27,
    0x3B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x6D, 0x65, 0x73,
    0x73, 0x61, 0x67, 0x65, 0x45, 0x6C, 0x65, 0x6D, 0x65, 0x6E, 0x74, 0x2E, 0x73, 0x74, 0x79,
    0x6C, 0x65, 0x2E, 0x62, 0x61, 0x63, 0x6B, 0x67, 0x72, 0x6F, 0x75, 0x6E, 0x64, 0x43, 0x6F,
    0x6C, 0x6F, 0x72, 0x20, 0x3D, 0x20, 0x27, 0x72, 0x65, 0x64, 0x27, 0x3B, 0x0D, 0x0A, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x73, 0x65, 0x74, 0x54, 0x69, 0x6D, 0x65,
    0x6F, 0x75, 0x74, 0x28, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x28, 0x29,
    0x20, 0x7B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x45, 0x6C, 0x65, 0x6D, 0x65, 0x6E,
    0x74, 0x2E, 0x69, 0x6E, 0x6E, 0x65, 0x72, 0x48, 0x54, 0x4D, 0x4C, 0x20, 0x3D, 0x20, 0x27,
    0x27, 0x3B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x7D, 0x2C,
    0x20, 0x33, 0x30, 0x30, 0x30, 0x29, 0x3B, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x7D,
    0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x7D, 0x29, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x7D, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x3C, 0x2F, 0x73, 0x63, 0x72,
    0x69, 0x70, 0x74, 0x3E, 0x0D, 0x0A, 0x3C, 0x2F, 0x62, 0x6F, 0x64, 0x79, 0x3E, 0x0D, 0x0A,
    0x3C, 0x2F, 0x68, 0x74, 0x6D, 0x6C, 0x3E,
}; 
