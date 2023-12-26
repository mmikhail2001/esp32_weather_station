#include "mq135.h"

#define VCC 5
#define RL 9.4 // 4.7 * 2
#define RO_CLEAN_AIR_FACTOR 1 // 9.83
// RL = 1060 Ом

static esp_adc_cal_characteristics_t adc1_chars;
static adc_channel_t pin_analog;
static gpio_num_t pin_digital;

static const char *TAG = "MQ135";

// Функция для вычисления напряжения на датчике
float read_sensor_voltage(adc1_channel_t channel, const esp_adc_cal_characteristics_t* adc_chars) {
    uint32_t adc_reading = adc1_get_raw(channel);
    ESP_LOGI(TAG, "adc_reading %d", adc_reading);
    uint32_t miles_voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    ESP_LOGI(TAG, "voltage %f", (float)miles_voltage / 1000.0);
    return (float)miles_voltage / 1000.0;  // Возвращаем напряжение в вольтах
}

float calculate_sensor_resistance(float sensor_voltage) {
    float rs = ((VCC - sensor_voltage) / sensor_voltage) * RL;
    return rs;
}

// Функция для вычисления PPM
float calculate_ppm(float rs, float ro) {
    // Примерные значения для A и B, полученные из калибровочной кривой (зависят от датчика)
    const float A = 6.87;
    const float B = -0.42;
    float ratio = rs / ro;
    ESP_LOGI(TAG, "ratio %f\n", ratio);
    float ppm = A * pow(ratio, B);
    ESP_LOGI(TAG, "ppm %f", ppm);
    return ppm;
}

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

    // Чтение напряжения с датчика
    float sensor_voltage = read_sensor_voltage(pin_analog, &adc1_chars);
    // Вычисление Rs
    float sensor_resistance = calculate_sensor_resistance(sensor_voltage);
    float ppm = calculate_ppm(sensor_resistance, RO_CLEAN_AIR_FACTOR);

    uint32_t voltage = adc1_get_raw(pin_analog);

    sprintf(lcd_data.str, "gas %5.2f", ppm);
    xQueueSendToBack(lcd_string_queue, &lcd_data, 0);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
