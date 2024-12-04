#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H


#include "gpio_lib.h"


// // Nhóm sự kiện để quản lý kết nối Wi-Fi
// static EventGroupHandle_t wifi_event_group;
// const int WIFI_CONNECTED_BIT = BIT0;

bool is_wifi_connected();

void task_wifi_init();
//void create_task_wireless(void);
// void obtain_time();
// bool is_access_allowed(char *allowed_days, char *start_time, char *end_time);
void func4(void);

#endif // WIFI_CONFIG_H