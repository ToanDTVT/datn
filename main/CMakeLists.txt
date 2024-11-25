idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES uart_lib gpio_lib user_data wifi_config http_method)
