#include "http_server.h"


#define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN  (64)

static const char *TAG = "http_server";

static esp_err_t root_handler(httpd_req_t *req)
{
    /* 访问根目录直接返回主页面 */
    httpd_resp_set_hdr(req, "Header-1", "Value-1");

    char *ip = "192.168.0.0";
    char *port = "80";
    char *response_html_buffer = malloc(sizeof(root_html)+100);
    if(response_html_buffer == NULL){
        httpd_resp_send(req, "out of memory", HTTPD_RESP_USE_STRLEN);
        ESP_LOGI(TAG, "out of memory to send html");
        return ESP_OK;
    }
    sprintf(response_html_buffer, root_html, ip, port);
    httpd_resp_send(req, response_html_buffer, HTTPD_RESP_USE_STRLEN);

    ESP_LOGI(TAG, "There are new visits coming");
    return ESP_OK;
}

static const httpd_uri_t root_uri_config = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_handler,
};

static esp_err_t login_handler(httpd_req_t *req)
{
    size_t content_length = req->content_len;

    char *buffer = malloc(content_length + 1);
    if (buffer == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    /* 从请求中读取请求体数据 */
    if (httpd_req_recv(req, buffer, content_length) <= 0) {
        free(buffer);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    buffer[content_length] = '\0';

    /* 解析表单数据 */
    char ssid[32], password[64];
    if (httpd_query_key_value(buffer, "username", ssid, sizeof(ssid)) == ESP_OK &&
        httpd_query_key_value(buffer, "password", password, sizeof(password)) == ESP_OK) {
        ESP_LOGI(TAG, "receive ssid: %s, password: %s", ssid, password); 

        /* 收到wifi sta模式配置数据 使用nvs写入flash储存 */
        if(nvs_update_wifi_info(STA_NVS_KEY, ssid, password) == 0)
            ESP_LOGI(TAG, "update_wifi_info success"); 
    }

    httpd_resp_send(req, "Login successful", HTTPD_RESP_USE_STRLEN);

    free(buffer);
    return ESP_OK;
}

static const httpd_uri_t login_uri_config = {
    .uri       = "/login",
    .method    = HTTP_POST,
    .handler   = login_handler,
    .user_ctx  = NULL
};

static httpd_handle_t start_http_server(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &root_uri_config);
        httpd_register_uri_handler(server, &login_uri_config);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}


void http_server_task(void *parameter)
{
    static httpd_handle_t server = NULL;
    server = start_http_server();

    while (server) {
        
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
    
}