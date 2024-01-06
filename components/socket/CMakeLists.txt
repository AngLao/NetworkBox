
set(srcs
    "udp.c"
   
)

set(dependencies
    freertos
    lwip
    log
    wifi
    nvs
)

idf_component_register( SRCS ${srcs}
                        INCLUDE_DIRS "." 
                        REQUIRES ${dependencies}
                        )