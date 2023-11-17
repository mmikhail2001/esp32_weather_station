#include "i2c.h"

#define I2C_FREQUENCY 400000

static const char *TAG = "i2c";

esp_err_t i2c_init_master(i2c_port_t port, gpio_num_t pin_sda,
                          gpio_num_t pin_scl) {
  esp_err_t ret;
  i2c_config_t conf = {.mode = I2C_MODE_MASTER,
                       .sda_io_num = pin_sda,
                       .sda_pullup_en = GPIO_PULLUP_ENABLE,
                       .scl_io_num = pin_scl,
                       .scl_pullup_en = GPIO_PULLUP_ENABLE,
                       .master.clk_speed = I2C_FREQUENCY};
  ret = i2c_param_config(port, &conf);
  if (ret != ESP_OK) {
    return ret;
  }
  ret = i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0);
  return ret;
}
