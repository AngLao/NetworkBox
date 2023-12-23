
set(srcs
    "nvs.c"
)

set(dependencies
    nvs_flash
)

idf_component_register( SRCS ${srcs}
                        INCLUDE_DIRS "." 
                        REQUIRES ${dependencies}
                        )