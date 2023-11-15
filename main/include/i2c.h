#pragma once 

#include "driver/i2c.h"
#include "esp_log.h"
#include <unistd.h>

esp_err_t i2c_init_master(i2c_port_t port, gpio_num_t pin_sda, gpio_num_t pin_scl);

