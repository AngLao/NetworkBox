#ifndef __NVS__
#define __NVS__

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#define STA_NVS_KEY             "STA"
#define STA_SSID_DEFAULT        "9527"
#define STA_PASSWORD_DEFAULT    "88888888"

#define AP_NVS_KEY              "AP"
#define AP_SSID_DEFAULT         "AngLao"
#define AP_PASSWORD_DEFAULT     "88888888"

// const char *STA_NVS_KEY = "STA";
// const char *STA_SSID_DEFAULT = "9527";
// const char *STA_PASSWORD_DEFAULT = "88888888";

// const char *AP_NVS_KEY = "AP";
// const char *AP_SSID_DEFAULT = "AngLao";
// const char *AP_PASSWORD_DEFAULT = "88888888";

typedef struct wifi_base_data
{
    uint8_t ssid[32];      // ssid 最大限制长度 32
    uint8_t password[64];  // password 最大限制长度 64
}wifi_base_data_t;

typedef struct wifi_message
{
    wifi_base_data_t ap;
    wifi_base_data_t sta;
}wifi_message_t;

typedef struct nvs_data
{
    const char *namespace;
    const char *key;
    void *value;
    size_t length;
}nvs_data_t;

void nvs_write_blob(const char *namespace, const char *key, const void *value, size_t length);
void nvs_read_blob(const char *namespace, const char *key, void *out_value, size_t *length);

void nvs_read_wifi_message(const char *key, wifi_base_data_t *wifi_message);
void nvs_update_wifi_message(const char *key, const char *ssid, const char *password);

void nvs_update_wifi_message_default(void);
 
#endif
