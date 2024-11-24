idf_component_register(SRCS "http_method.c"
                    INCLUDE_DIRS "include"
                    REQUIRES http_method gpio_lib mqtt json user_data)