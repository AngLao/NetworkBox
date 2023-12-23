
set(srcs
    "http_server.c"
)

set(dependencies 
    esp_http_server 
    protocol_examples_common
    nvs
)

idf_component_register( SRCS ${srcs}
                        INCLUDE_DIRS "." 
                        REQUIRES ${dependencies}
                        )