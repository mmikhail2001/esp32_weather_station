#include "http_server.h"


static char login[64] = "";
static char password[64] = "";
static httpd_handle_t server = NULL;
static const char *TAG = "HTTP_SERVER";

static esp_err_t root_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET)
    {
        httpd_resp_sendstr(req, "<html><body>"
                        "<form onsubmit=\"submitForm(event)\">"
                        "Login: <input type=\"text\" name=\"login\"><br>"
                        "Password: <input type=\"password\" name=\"password\"><br>"
                        "<input type=\"submit\" value=\"Submit\">"
                        "</form>"
                        "<div id=\"message\"></div>"
                        "<script>"
                        "function submitForm(event) {"
                        "  event.preventDefault();"
                        "  const login = document.getElementsByName('login')[0].value;"
                        "  const password = document.getElementsByName('password')[0].value;"
                        " fetch('/login?login=' + encodeURIComponent(login) + '&password=' + encodeURIComponent(password))"  // Изменение на использование fetch
                        "  .then(response => {"
                        "    if (response.ok) {"
                        "      return response.text();"
                        "    } else {"
                        "      throw new Error('Request failed');"
                        "    }"
                        "  })"
                        "  .then(text => {"
                        "    document.getElementById('message').innerHTML = 'Data applied successfully.';"
                        "    setTimeout(function () {"
                        "      document.getElementById('message').innerHTML = '';"
                        "    }, 3000);"
                        "  })"
                        "  .catch(error => {"
                        "    document.getElementById('message').innerHTML = 'Data not applied: ' + error;"
                        "    setTimeout(function () {"
                        "      document.getElementById('message').innerHTML = '';"
                        "    }, 3000);"
                        "  });"
                        "}"
                        "</script>"
                        "</body></html>");
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
            char* buf = malloc(buf_len);
            if (buf)
            {
                if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
                {
                    char param[64];
                    if (httpd_query_key_value(buf, "login", param, sizeof(param)) == ESP_OK)
                    {
                        if (strlen(param) != 0) {
                            strncpy(login, param, sizeof(login) - 1);
                        }
                    }
                    if (httpd_query_key_value(buf, "password", param, sizeof(param)) == ESP_OK)
                    {
                        strncpy(password, param, sizeof(password) - 1);
                    }
                    httpd_resp_sendstr(req, "Data applied successfully.");
                    ESP_LOGI(TAG, "Received login: %s, password: %s", login, password);
                    ESP_LOGI(TAG, "GET /login HTTP successful");
                    free(buf);
                    return ESP_OK;
                }
                free(buf);
            }
        }
     }
    ESP_LOGE(TAG, "GET /login HTTP failed");
    httpd_resp_sendstr(req, "Bad Request");
    // TODO: это ответ 400?
    return ESP_FAIL;
}

httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_handler,
    .user_ctx = NULL};

httpd_uri_t login_uri = {
    .uri = "/login",
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

    xEventGroupSetBits(net_event_group, HTTP_STARTED);

    ESP_LOGI(TAG, "HTTP server started");

    return server;
}