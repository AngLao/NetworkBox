#include "nvs.h"

static const char *TAG = "nvs";

// 存储数据块到 NVS
int nvs_write_blob(const char *namespace, const char *key, const void *value, size_t length)
{
    nvs_handle_t nvs_handle;
    if(nvs_open(namespace, NVS_READWRITE, &nvs_handle) != ESP_OK)
        return -1;

    if(nvs_set_blob(nvs_handle, key, value, length) != ESP_OK)
        return -1;

    // 写入flash
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));
    
    nvs_close(nvs_handle);
    return 0;
}

// 从 NVS 读数据块
int nvs_read_blob(const char *namespace, const char *key, void *out_value, size_t *length)
{
    nvs_handle_t nvs_handle;
    if( nvs_open(namespace, NVS_READONLY, &nvs_handle) != ESP_OK)
        return -1;

    if (nvs_get_blob(nvs_handle, key, out_value, length) != ESP_OK) 
        return -1;

    nvs_close(nvs_handle);
    return 0;
}

int nvs_update_wifi_info(const char *key, const char *ssid, const char *password)
{
    if( (key == NULL) || (ssid == NULL) || (password == NULL) ){
        ESP_LOGI(TAG, "null pointer error");
        return -1;
    }

    size_t ssid_len = strlen(ssid);
    size_t password_len = strlen(password);
    if( (ssid_len > 31) || (password_len > 63)){
        ESP_LOGI(TAG, "parameter overlength error");
        return -1;
    }

    wifi_info_t wifi_message = {
        .ssid = {0},
        .password = {0},
    };
    memcpy(wifi_message.ssid, ssid, ssid_len);
    memcpy(wifi_message.password, password, password_len);

    return nvs_write_blob("wifi", key, &wifi_message, sizeof(wifi_message));
}

int nvs_read_wifi_info(const char *key, wifi_info_t *wifi_info)
{
    if(wifi_info == NULL){
        ESP_LOGI(TAG, "wifi_info null pointer error");
        return -1;
    }

    size_t length = 0;
    nvs_read_blob("wifi", key, NULL, &length);
    if(length == 0){
        ESP_LOGI(TAG, "This data block has no data");
        return -1;
    }

    length = sizeof(*wifi_info);
    return nvs_read_blob("wifi", key, (void *)wifi_info, &length);
}

int nvs_update_ip_info(const char *key, const char *ip, const uint16_t port)
{
    if( (key == NULL) || (ip == NULL) ){
        ESP_LOGI(TAG, "null pointer error");
        return -1;
    }

    size_t ip_len = strlen(ip);
    if( (ip_len > 46)){
        ESP_LOGI(TAG, "ip parameter overlength error");
        return -1;
    }

    ip_info_t ip_info = {
        .ip_str = {0},
        .port = port,
    };
    memcpy(ip_info.ip_str, ip, ip_len);

    return nvs_write_blob("ip", key, &ip_info, sizeof(ip_info));
}

int nvs_read_ip_info(const char *key, ip_info_t *ip_info)
{
    if(ip_info == NULL){
        ESP_LOGI(TAG, "ip_info null pointer error");
        return -1;
    }

    size_t length = 0;
    nvs_read_blob("ip", key, NULL, &length);
    if(length == 0){
        ESP_LOGI(TAG, "This data block has no data");
        return -1;
    }

    length = sizeof(*ip_info);
    return nvs_read_blob("ip", key, (void *)ip_info, &length);
}