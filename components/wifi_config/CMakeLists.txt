idf_component_register(SRCS "wifi_config.c"
                    INCLUDE_DIRS "include"
                    REQUIRES wifi_config gpio_lib mqtt json user_data)