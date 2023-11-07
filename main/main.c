#include "main.h"

static const char *TAG = "main";


/*
    TODO:
    - lcd_string_queue передавать в качестве параметра в задачи или как глобальную через extern?
    - превышение порога газа должно фиксироваться
    - mqtt, ws
    - питание
*/


extern QueueHandle_t lcd_string_queue;
extern QueueHandle_t ws_send_queue;

void app_main(void)
{ 
    lcd_string_queue = xQueueCreate(10, sizeof(lcd_data_t));
    ws_send_queue = xQueueCreate(10, sizeof(lcd_data_t));



    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    wifi_init_sta();

    ws_init();

    // i2c
    ret = i2c_init_master(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
    ESP_ERROR_CHECK(ret);

    ret = i2c_init_master(I2C_NUM_1, GPIO_NUM_33, GPIO_NUM_32);
    ESP_ERROR_CHECK(ret);

    // bmx280
    bmx280_t* bmx280 = bmx280_create(I2C_NUM_1);

    if (!bmx280) { 
        ESP_LOGE("test", "Could not create bmx280 driver.");
        return;
    }

    ESP_ERROR_CHECK(bmx280_init(bmx280));

    bmx280_config_t bmx_cfg = BMX280_DEFAULT_CONFIG;
    ESP_ERROR_CHECK(bmx280_configure(bmx280, &bmx_cfg));

    mq135_init(ADC_CHANNEL_0, GPIO_NUM_27);



    // lcd
    // TODO: не возвращает ошибку???
    lcd_init();
    lcd_clear();
    vTaskDelay(300 / portTICK_PERIOD_MS);

    // dht11
    DHT11_init(GPIO_NUM_23);
    vTaskDelay(300 / portTICK_PERIOD_MS);

    // async tasks
    xTaskCreate(lcd_process_queue_task, "lcd_process_queue_task", 2048, NULL, 2, NULL);
    xTaskCreate(dht11_read_task, "dht11_read_task", 2048, NULL, 2, NULL);
    xTaskCreate(bmx280_read_task, "bmx280_read_task", 2048, bmx280, 2, NULL);
    xTaskCreate(mq135_read_task, "mq135_read_task", 2048, NULL, 2, NULL);
    xTaskCreate(ws_send_task, "ws_send_task", 4096, NULL, 2, NULL);
    // configMAX_PRIORITIES - максимальный приоритет

    while(1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

}
