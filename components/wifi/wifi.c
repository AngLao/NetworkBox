#include "wifi.h"

static const char *TAG = "wifi";

/* WiFi sta连接事件组 */
static EventGroupHandle_t sta_wifi_event_group;
static EventBits_t sta_event_status = 0;

/* WiFi事件响应标志 */
#define STA_WIFI_WAIT           0
#define STA_WIFI_CONNECTED      1
#define STA_WIFI_DISCONNECTED   2

static uint8_t sta_connect_status = 0;

uint8_t wifi_sta_isconnect(void)
{
    return sta_connect_status;
}

static void sta_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        sta_event_status = STA_WIFI_DISCONNECTED;
        sta_connect_status = 0;
        ESP_LOGI(TAG,"WiFi event sta disconnected");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        sta_connect_status = 1;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));

    }
}

static void sta_create_event_loop(void)
{
    /* 创建事件组 */
    if (sta_wifi_event_group == NULL) {
        sta_wifi_event_group = xEventGroupCreate();
        if (sta_wifi_event_group == NULL) 
            ESP_LOGI(TAG, "start sta connect error");
    }

    /* 初始化 WiFi STA 接口 */
    esp_netif_t *sta_handle = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_handle == NULL){
        assert(esp_netif_create_default_wifi_sta());

        /* 注册WiFi连接事件循环 */
        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            &sta_event_handler,
                                                            NULL,
                                                            &instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_STA_GOT_IP,
                                                            &sta_event_handler,
                                                            NULL,
                                                            &instance_got_ip));
    }
}

static void sta_delete_event_loop(void)
{
    /* 销毁事件组 */
    if (sta_wifi_event_group != NULL) {
        vEventGroupDelete(sta_wifi_event_group);
        sta_wifi_event_group = NULL;  // 重置句柄，避免误用
    }

    /* 销毁 WiFi STA 接口 */
    esp_netif_t *sta_handle = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_handle != NULL) 
        esp_netif_destroy(sta_handle);

    /* 注销WiFi连接事件循环 */
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,
                                                ESP_EVENT_ANY_ID,
                                                &sta_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,
                                                IP_EVENT_STA_GOT_IP,
                                                &sta_event_handler));      
} 

void wifi_init_sta(void) {
    wifi_config_t wifi_config = {
        .sta = {
        },
    };
    /* 从flsh中读取WiFi配置 */
    wifi_info_t wifi_message;
    nvs_read_wifi_info(STA_NVS_KEY, &wifi_message);
    memcpy(wifi_config.sta.ssid, wifi_message.ssid, sizeof(wifi_message.ssid));
    memcpy(wifi_config.sta.password, wifi_message.password, sizeof(wifi_message.password));

    ESP_LOGI(TAG, "STA mode ssid: %s password: %s",wifi_config.sta.ssid, wifi_config.sta.password);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_connect());
    ESP_LOGI(TAG, "-----------start sta connect-----------");  
}

static void ap_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void) {
    assert(esp_netif_create_default_wifi_ap());
    
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &ap_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .max_connection = 2,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };
    /* 从flsh中读取WiFi配置 */
    wifi_info_t wifi_message;
    nvs_read_wifi_info(AP_NVS_KEY, &wifi_message);
    memcpy(wifi_config.ap.ssid, wifi_message.ssid, sizeof(wifi_message.ssid));
    memcpy(wifi_config.ap.password, wifi_message.password, sizeof(wifi_message.password));

    /* 无密码模式 */
    if (strlen((const char *)wifi_config.ap.password) == 0) 
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;

    ESP_LOGI(TAG, "AP mode ssid: %s password: %s",wifi_config.ap.ssid, wifi_config.ap.password);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void wifi_task(void *parameter) {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    /* 初始化 AP 模式 */
    wifi_init_softap();

    /* 初始化 STA 模式 */
    sta_create_event_loop();
    wifi_init_sta();
    
    while (1) {
        /* 连接失败或者断开连接进行重连 */
        if (sta_event_status == STA_WIFI_DISCONNECTED) {
            /* 等待事件发生 */
            sta_event_status = STA_WIFI_WAIT;
            ESP_LOGI(TAG, "retry to connect");
            wifi_init_sta();
        }
        
        /* 自动重连时间间隔 */
        vTaskDelay(pdMS_TO_TICKS(15000)); 
    }
}
