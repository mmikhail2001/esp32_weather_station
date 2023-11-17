#include "http_server.h"

#define ACCESS_KEY "bmstu"

static char ssid[64];
static char password[64];
static httpd_handle_t server = NULL;
static const char *TAG = "HTTP_SERVER";

static esp_err_t root_handler(httpd_req_t *req)
{
	if (req->method == HTTP_GET)
	{
		httpd_resp_sendstr(req, index_html);
		ESP_LOGI(TAG, "GET / HTTP successful");
		return ESP_OK;
	}
	ESP_LOGE(TAG, "GET / HTTP failed");
	return httpd_resp_send_404(req);
}

static esp_err_t login_handler(httpd_req_t *req)
{
	char buf[2048];
	memset(buf, 0, sizeof(buf));
	if (req->method == HTTP_GET)
	{
		size_t buf_len = httpd_req_get_url_query_len(req) + 1;
		if (buf_len > 1)
		{
			char *buf = malloc(buf_len);
			if (buf)
			{
				if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
				{
					ESP_LOGI(TAG, "parsing query string");
					char param[64];
					if (httpd_query_key_value(buf, "access_key", param, sizeof(param)) ==
						ESP_OK)
					{
						if (strcmp(param, ACCESS_KEY) != 0)
						{
							httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, NULL);
							free(buf);
							ESP_LOGI(TAG, "handler /login 403");
							return ESP_OK;
						}
					}
					if (httpd_query_key_value(buf, "ssid", param, sizeof(param)) ==
						ESP_OK)
					{
						if (strlen(param) != 0)
						{
							strncpy(ssid, param, sizeof(ssid) - 1);
						}
					}
					if (httpd_query_key_value(buf, "password", param, sizeof(param)) ==
						ESP_OK)
					{
						strncpy(password, param, sizeof(password) - 1);
					}
					ESP_LOGI(TAG, "Received ssid: %s, password: %s", ssid, password);
					ESP_LOGI(TAG, "GET /login HTTP 200");
					free(buf);
					httpd_resp_send(req, NULL, 0);

					nvs_handle_t nvs_handle;
					ESP_ERROR_CHECK(nvs_open("app.storage", NVS_READWRITE, &nvs_handle));
					nvs_set_str(nvs_handle, "app.wifi.ssid", ssid);
					nvs_set_str(nvs_handle, "app.wifi.pass", password);
					nvs_commit(nvs_handle);
					nvs_close(nvs_handle);
					esp_restart();
					return ESP_OK;
				}
				free(buf);
			}
		}
	}
	ESP_LOGE(TAG, "GET /login HTTP 400");
	httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, NULL);
	return ESP_FAIL;
}

httpd_uri_t root = {
	.uri = "/", .method = HTTP_GET, .handler = root_handler, .user_ctx = NULL};

httpd_uri_t login_uri = {.uri = "/login",
						 .method = HTTP_GET,
						 .handler = login_handler,
						 .user_ctx = NULL};

httpd_handle_t start_http_server()
{
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.stack_size = 8192;
	config.server_port = 80;

	ESP_ERROR_CHECK(httpd_start(&server, &config));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &login_uri));

	ESP_LOGI(TAG, "HTTP server started");

	return server;
}

void stop_http_server(httpd_handle_t server)
{
	if (!server)
	{
		ESP_LOGI(TAG, "stop_http_server: server = NULL");
		return;
	}
	ESP_ERROR_CHECK(httpd_stop(server));
	ESP_LOGI(TAG, "HTTP server stopped");
}
