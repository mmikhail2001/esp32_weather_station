#include "main.h"

static const char *TAG = "main";

/*
    TODO:
    - lcd_string_queue передавать в качестве параметра в задачи или как
   глобальную через extern? (по хорошему нужен вариант 1)
    - превышение порога газа должно фиксироваться
    + ws
        - передача в заголовке device_id
    + питание
    - tg bot
    - команда на рестарт по ws
    - мигание или инфа на лед
        - не подключен к wifi
        - подключен к wifi
        - установлена связь с сервером по ws
    + wifi ap и http сервер
    - http сервер передача
        - показать текущий wifi ssid
        - при необходимости изменить ssid нужно ввести
            - пароль от устройства
            - новый ssid
            - новый password
    - изменить событийную систему на эту
        - sta_connected
        - sta_disconnected
        - ws_connected
        - ws_disconnected
        - ap_start
        - ap_end
        - new_device_connected
        - device_disconnected
    - при post запросе формы можно не перезагружать МК
*/

extern QueueHandle_t lcd_string_queue;
extern QueueHandle_t ws_send_queue;
static TimerHandle_t button_timer = NULL;
static QueueHandle_t button_queue = NULL;

static esp_netif_t *netif_wifi_sta;
static esp_netif_t *netif_wifi_ap;

static bool wifi_ap_active = false;

static httpd_handle_t server = NULL;

static void button_switch_wifi_ap_sta_task(void *arg) {
  uint32_t button_value;
  while (1) {
    if (xQueueReceive(button_queue, &button_value, portMAX_DELAY)) {
      ESP_LOGI(TAG, "message received, button level [%d]\n", button_value);
      if (!wifi_ap_active) {
        ws_stop();
        esp_wifi_stop();

        // TODO: не выводится
        esp_netif_ip_info_t ip_info;
        ESP_ERROR_CHECK(esp_netif_get_ip_info(netif_wifi_ap, &ip_info));
        ESP_LOGI(TAG, "AP IP Address: " IPSTR, IP2STR(&ip_info.ip));

        wifi_init_softap();
        server = start_http_server();
        wifi_ap_active = true;
        ESP_LOGI(TAG, "Wifi STA stopped, WiFi AP and HTTP server started");
      } else {
        esp_wifi_stop();
        httpd_stop(server);
        wifi_ap_active = false;
        wifi_init_sta();
        ws_start();
        ESP_LOGI(TAG, "Wifi STA started, WiFi AP and HTTP server stopped");
      }
    }
  }
}

static void buttonTimerHandle(TimerHandle_t timer) {
  gpio_intr_enable(GPIO_NUM_26);
}

static void IRAM_ATTR button_isr_handler(void *arg) {
  gpio_intr_disable(GPIO_NUM_26);
  uint32_t button_value = gpio_get_level(GPIO_NUM_26);
  xQueueSendFromISR(button_queue, &button_value, NULL);
  xTimerStartFromISR(button_timer, pdFALSE);
}

void app_main(void) {
  esp_err_t ret;
  // button isr

  gpio_set_direction(GPIO_NUM_26, GPIO_MODE_INPUT);
  gpio_pullup_en(GPIO_NUM_26);
  gpio_set_intr_type(GPIO_NUM_26, GPIO_INTR_POSEDGE);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(GPIO_NUM_26, button_isr_handler, NULL);

  button_timer = xTimerCreate("Button timer", 500 / portTICK_PERIOD_MS, pdFALSE,
                              NULL, buttonTimerHandle);

  // queues

  lcd_string_queue = xQueueCreate(10, sizeof(lcd_data_t));
  ws_send_queue = xQueueCreate(10, sizeof(lcd_data_t));
  button_queue = xQueueCreate(10, sizeof(uint32_t));

  // eventGroups

  net_event_group = xEventGroupCreate();

  // i2c
  ret = i2c_init_master(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
  ESP_ERROR_CHECK(ret);

  ret = i2c_init_master(I2C_NUM_1, GPIO_NUM_33, GPIO_NUM_32);
  ESP_ERROR_CHECK(ret);

  // lcd
  // TODO: не возвращает ошибку???
  lcd_init(I2C_NUM_0);
  lcd_clear();
  vTaskDelay(300 / portTICK_PERIOD_MS);

  // nvs

  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  netif_wifi_sta = esp_netif_create_default_wifi_sta();
  netif_wifi_ap = esp_netif_create_default_wifi_ap();

  ESP_LOGI(TAG, "netif_wifi_sta and netif_wifi_ap created");

  // wifi sta
  wifi_init_sta();

  ESP_LOGI(TAG, "Wifi STA init");

  // bmx280
  bmx280_t *bmx280 = bmx280_create(I2C_NUM_1);

  if (!bmx280) {
    ESP_LOGE(TAG, "Could not create bmx280 driver.");
    return;
  }

  ESP_ERROR_CHECK(bmx280_init(bmx280));

  bmx280_config_t bmx_cfg = BMX280_DEFAULT_CONFIG;
  ESP_ERROR_CHECK(bmx280_configure(bmx280, &bmx_cfg));

  // mq135
  mq135_init(ADC_CHANNEL_0, GPIO_NUM_27);

  // dht11
  DHT11_init(GPIO_NUM_23);
  vTaskDelay(300 / portTICK_PERIOD_MS);

  // websocket
  ws_init();
  ws_start();

  // async tasks
  xTaskCreate(lcd_process_queue_task, "lcd_process_queue_task", 2048, NULL, 2,
              NULL);
  xTaskCreate(dht11_read_task, "dht11_read_task", 2048, NULL, 2, NULL);
  xTaskCreate(bmx280_read_task, "bmx280_read_task", 2048, bmx280, 2, NULL);
  xTaskCreate(mq135_read_task, "mq135_read_task", 2048, NULL, 2, NULL);
  xTaskCreate(ws_send_task, "ws_send_task", 4096, NULL, 2, NULL);
  xTaskCreate(button_switch_wifi_ap_sta_task, "button_task", 4096, NULL, 10,
              NULL);
  xTaskCreate(display_info_task, "display_info_task", 4096, NULL, 10, NULL);

  // lcd info
  lcd_data_t lcd_data;
  lcd_data.col = 10;
  lcd_data.row = 0;
  sprintf(lcd_data.str, "sta/ap");
  xQueueSendToBack(lcd_string_queue, &lcd_data, 0);
  vTaskDelay(300 / portTICK_PERIOD_MS);

  while (1) {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}
