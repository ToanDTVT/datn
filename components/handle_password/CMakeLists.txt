idf_component_register(SRCS "handle_password.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver handle_password gpio_lib user_data uart_lib wifi_config)
