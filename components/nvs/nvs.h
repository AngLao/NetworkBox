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

#define UDP_MULTICAST_KEY       "udp_multicast"

typedef struct wifi_info
{
    uint8_t ssid[32];      // ssid 最大限制长度 32
    uint8_t password[64];  // password 最大限制长度 64
}wifi_info_t;

typedef struct ip_info
{
    char ip_str[46];  // 最大IPv6地址的字符串长度
    uint16_t port;
}ip_info_t;


typedef struct nvs_data
{
    const char *namespace;
    const char *key;
    void *value;
    size_t length;
}nvs_data_t;

int nvs_write_blob(const char *namespace, const char *key, const void *value, size_t length);
int nvs_read_blob(const char *namespace, const char *key, void *out_value, size_t *length);

int nvs_read_wifi_info(const char *key, wifi_info_t *wifi_message);
int nvs_update_wifi_info(const char *key, const char *ssid, const char *password);

int nvs_read_ip_info(const char *key, ip_info_t *ip_info);
int nvs_update_ip_info(const char *key, const char *ip, const uint16_t port);

 
#endif
