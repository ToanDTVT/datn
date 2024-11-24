#ifndef MQTT_H
#define MQTT_H

#include "gpio_lib.h"
#include "user_data.h"
#include "mqtt_client.h"
#include "cJSON.h"

extern DataPacket USER[MAX_USERS];
extern Data_send_server data;
extern Type_open_door action;

void mqtt_app_start();
// void mqtt_task(void *pvParameters);
//void create_mqtt_task (void);
void func5 (void);

#endif //MQTT_H