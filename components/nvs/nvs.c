#include "nvs.h"

static const char *TAG = "nvs";

// 存储数据块到 NVS
void nvs_write_blob(const char *namespace, const char *key, const void *value, size_t length)
{
    // 初始化 NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    // 打开 NVS 命名空间
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open(namespace, NVS_READWRITE, &nvs_handle));

    // 存储字符串键值对到 NVS
    ESP_ERROR_CHECK(nvs_set_blob(nvs_handle, key, value, length));

    // 提交更改
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));

    ESP_LOGI(TAG, "%s blob data commit success", key);
    // 关闭 NVS 命名空间
    nvs_close(nvs_handle);
}

// 从 NVS 读数据块
void nvs_read_blob(const char *namespace, const char *key, void *out_value, size_t *length)
{
    // 打开 NVS 命名空间
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open(namespace, NVS_READONLY, &nvs_handle));

    esp_err_t ret = nvs_get_blob(nvs_handle, key, out_value, length);
    if (ret != ESP_OK) 
        ESP_LOGI(TAG, "Error reading WiFi SSID from NVS\n");

    // 关闭 NVS 命名空间
    nvs_close(nvs_handle);
}

void nvs_update_wifi_message(const char *key, const char *ssid, const char *password)
{
    if( (key == NULL) || (ssid == NULL) || (password == NULL) ){
        ESP_LOGI(TAG, "null pointer error");
        return;
    }

    size_t ssid_len = strlen(ssid);
    size_t password_len = strlen(password);
    if( (ssid_len > 31) || (password_len > 63)){
        ESP_LOGI(TAG, "parameter overlength error");
        return;
    }

    wifi_base_data_t wifi_message = {
        .ssid = {0},
        .password = {0},
    };

    memcpy(wifi_message.ssid, ssid, ssid_len);
    memcpy(wifi_message.password, password, password_len);

    nvs_data_t nvs_data = {
        .namespace = "wifi",
        .key = key,
        .value = &wifi_message,
        .length = sizeof(wifi_message)
    };
    
    nvs_write_blob(nvs_data.namespace, nvs_data.key, nvs_data.value, nvs_data.length);
}

void nvs_read_wifi_message(const char *key, wifi_base_data_t *wifi_message)
{
    if(wifi_message == NULL){
        ESP_LOGI(TAG, "null pointer error");
        return;
    }

    nvs_data_t nvs_data = {
        .namespace = "wifi",
        .key = key,
        .value = (void *)wifi_message,
        .length = sizeof(*wifi_message)
    };

    size_t length = 0;
    nvs_read_blob(nvs_data.namespace, nvs_data.key, NULL, &length);
    ESP_LOGI(TAG, "%s blob len : %d", nvs_data.key, length);
    if(length == 0){
        ESP_LOGI(TAG, "This data block has no data");
        return;
    }

    nvs_read_blob(nvs_data.namespace, nvs_data.key, nvs_data.value, &nvs_data.length);
    ESP_LOGI(TAG, "%s ssid: %s password: %s",nvs_data.key, wifi_message->ssid, wifi_message->password);
}


void nvs_update_wifi_message_default(void)
{
    nvs_update_wifi_message(STA_NVS_KEY, STA_SSID_DEFAULT, STA_PASSWORD_DEFAULT);
    nvs_update_wifi_message(AP_NVS_KEY, AP_SSID_DEFAULT, AP_PASSWORD_DEFAULT);

    wifi_base_data_t wifi_message;
    nvs_read_wifi_message(STA_NVS_KEY, &wifi_message);
    nvs_read_wifi_message(AP_NVS_KEY, &wifi_message);
}