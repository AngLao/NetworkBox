#ifndef _WIFI_
#define _WIFI_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif_types.h"
#include "esp_mac.h"
#include "event_groups.h"
#include "esp_log.h"

#include "../nvs/nvs.h"

void wifi_task(void *parameter);

#endif
