#ifndef __UDP__
#define __UDP__

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/event_groups.h"

#include <lwip/dns.h>
#include "esp_log.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "../wifi/wifi.h"
#include "../nvs/nvs.h"


void udp_task(void *parameter);
void udp_client_task(void *pvParameters);
#endif
