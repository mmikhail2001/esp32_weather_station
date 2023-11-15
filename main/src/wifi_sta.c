// #include "lwip/err.h"
// #include "lwip/sys.h"
#include "wifi_sta.h"

// #define WIFI_SSID      "MGTS_GPON_5F1C"
#define WIFI_SSID      "HONOR 20S"
// #define WIFI_PASS      "5AYJ69HK"
#define WIFI_PASS      "88888888"
#define DEFAULT_FORCE      1
#define MAXIMUM_RETRY  5

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    ESP_LOGI(TAG, "event WIFI.......");
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "event WIFI_EVENT_STA_START");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "event WIFI_EVENT_STA_DISCONNECTED");
        xEventGroupClearBits(net_event_group, STA_CONNECTED);
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP, attempts: %d/%d", s_retry_num, MAXIMUM_RETRY);
        } else {
            ESP_LOGI(TAG, "failed attempts to connect to the AP");
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(net_event_group, STA_CONNECTED);
        ESP_LOGI(TAG, "event IP_EVENT_STA_GOT_IP");
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("app.storage", NVS_READWRITE, &nvs_handle));

    char ssid[32];
    char password[64];
    size_t ssid_len = sizeof(ssid);
    size_t password_len = sizeof(password);

    esp_err_t ssid_err = nvs_get_str(nvs_handle, "app.wifi.ssid", ssid, &ssid_len);
    esp_err_t pass_err = nvs_get_str(nvs_handle, "app.wifi.pass", password, &password_len);

    if (ssid_err != ESP_OK || pass_err != ESP_OK) {
        ESP_LOGI(TAG, "ssid and pass are not in nvs");


        strcpy(ssid, WIFI_SSID);
        strcpy(password, WIFI_PASS);

        nvs_set_str(nvs_handle, "app.wifi.ssid", ssid);
        nvs_set_str(nvs_handle, "app.wifi.pass", password);
        nvs_commit(nvs_handle);
    } else {
        ESP_LOGI(TAG, "ssid and pass are in nvs: %s, %s", ssid, password);
    }

    nvs_close(nvs_handle);


    s_wifi_event_group = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
	     * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    if (!DEFAULT_FORCE) {
    //     wifi_config.sta.ssid = WIFI_SSID;
    //     wifi_config.sta.password = WIFI_PASS;
    // } else {
        strcpy((char *)wifi_config.sta.ssid, ssid);
        strcpy((char *)wifi_config.sta.password, password);
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    // without this: Brownout detector was triggered
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "WIFI STA started, waiting events");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s",
                 WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s",
                 WIFI_SSID);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}
