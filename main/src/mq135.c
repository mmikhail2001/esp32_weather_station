#include "mq135.h"

static esp_adc_cal_characteristics_t adc1_chars;
static adc_channel_t pin_analog;
static gpio_num_t pin_digital;

static const char *TAG = "MQ135";

void mq135_init(adc_channel_t apin, gpio_num_t gpin) {
  pin_analog = apin;
  pin_digital = gpin;

  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0,
                           &adc1_chars);
  ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
  ESP_ERROR_CHECK(adc1_config_channel_atten(pin_analog, ADC_ATTEN_DB_11));

  gpio_config_t io_conf;
  io_conf.pin_bit_mask = (1ULL << pin_digital);
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&io_conf));
}

void mq135_read_task(void *arg) {
  lcd_data_t lcd_data;

  while (1) {
    lcd_data.row = 3;
    lcd_data.col = 0;

    int no_threshold = gpio_get_level(pin_digital);
    uint32_t voltage = adc1_get_raw(pin_analog);

    sprintf(lcd_data.str, "gas %3d,%1d", voltage, no_threshold);
    xQueueSendToBack(lcd_string_queue, &lcd_data, 0);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
