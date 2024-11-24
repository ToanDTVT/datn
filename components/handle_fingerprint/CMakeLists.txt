idf_component_register(SRCS "handle_fingerprint.c"
                    INCLUDE_DIRS "include"
                    REQUIRES handle_fingerprint uart_lib gpio_lib user_data)