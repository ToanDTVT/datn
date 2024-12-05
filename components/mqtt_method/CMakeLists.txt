idf_component_register(SRCS "mqtt_method.c"
                    INCLUDE_DIRS "include"
                    REQUIRES mqtt_method gpio_lib mqtt json user_data wifi_config uart_lib)