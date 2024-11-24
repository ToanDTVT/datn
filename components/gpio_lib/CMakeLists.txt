idf_component_register(SRCS "gpio_lib.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver gpio_lib esp_wifi mqtt nvs_flash esp_event esp_http_client protocol_examples_common json user_data)
