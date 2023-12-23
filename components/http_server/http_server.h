#ifndef _HTTP_SERVER_
#define _HTTP_SERVER_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_log.h>
#include <esp_http_server.h>
#include "protocol_examples_utils.h"

#include "response_html.h"
#include "../nvs/nvs.h"

void http_server_task(void *parameter);

#endif
