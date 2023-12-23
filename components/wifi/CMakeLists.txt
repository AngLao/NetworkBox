
set(srcs
    "wifi.c"
)

set(dependencies
    esp_wifi
    esp_event
    esp_hw_support
    log
    nvs
)

idf_component_register( SRCS ${srcs}
                        INCLUDE_DIRS "." 
                        "$ENV{IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos"
                        REQUIRES ${dependencies}
                        )