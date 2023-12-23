#include <string.h>
#include <stdlib.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_system.h>

#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"

#include "../components/wifi/wifi.h"
#include "../components/http_server/http_server.h"
#include "../components/espressif__mdns/include/mdns.h"

void app_main(void)
{
    /* 系统基础配置初始化 */
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    /* 初始化mdns */
    ESP_ERROR_CHECK(mdns_init());
    /* 配置局域网域名 */    
    ESP_ERROR_CHECK(mdns_hostname_set("NetworkBox"));    
    ESP_ERROR_CHECK(mdns_service_add(NULL, "_http", "_tcp", 80, NULL,0));    

    /* 自定义任务 */
    xTaskCreate(http_server_task, "http_server_task", 2048, NULL, 3, NULL);
    xTaskCreate(wifi_task, "wifi_task", 4096, NULL, 3, NULL);
}
