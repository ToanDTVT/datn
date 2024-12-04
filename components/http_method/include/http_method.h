#ifndef HTTP_METHOD_H
#define HTTP_METHOD_H

#include "gpio_lib.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "user_data.h"
#include "wifi_config.h"

extern Student students[MAX_STUDENTS];

//void fetch_database();

//void http_get_task();
void create_http_task(void);
void send_login_request(int student_x);
void fetch_access_time(const char *student_id);
void func6 (void);

#endif //HTTP_METHOD_H