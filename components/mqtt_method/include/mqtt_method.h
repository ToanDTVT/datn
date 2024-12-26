#ifndef MQTT_METHOD_H
#define MQTT_METHOD_H

#include "gpio_lib.h"
#include "wifi_config.h"
#include "user_data.h"
#include "uart_lib.h"

extern Student students[MAX_STUDENTS];

void func8 (void);
void mqtt_app_start();
void esp32_pub_for_esp32cam_sub_MSSV_ENTER();
void esp32_pub_for_esp32cam_sub_DISPLAY_INFOR(char *MSSV);

#endif //MQTT_METHOD_H