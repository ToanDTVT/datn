#ifndef GPIO_LIB_H
#define GPIO_LIB_H

#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_compiler.h"
#include "esp_log.h"

#include "driver/dedic_gpio.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#include "esp_rom_sys.h"
#include "esp_system.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "protocol_examples_common.h"
#include "cJSON.h"

#include "lwip/err.h"
#include "lwip/sys.h"

//#include "user_data.h"

// extern DataPacket USER[MAX_USERS];
// extern Data_send_server data;
// extern Type_open_door action;


/*GPIO for keypad*/
//ROW
#define ROW_1 GPIO_NUM_33             //pin2 keypad
#define ROW_2 GPIO_NUM_12             //pin7 keypad
#define ROW_3 GPIO_NUM_14             //pin6 keypad
#define ROW_4 GPIO_NUM_26             //pin4 keypad

//COL
#define COL_1 GPIO_NUM_25             //pin5 keypad
#define COL_2 GPIO_NUM_32             //pin1 keypad
#define COL_3 GPIO_NUM_27             //pin3 keypad


/*GPIO for relay*/
#define RELAY GPIO_NUM_13             


void func1(void);

void gpio_init();
void keypad_init();
char keypad_get_char();
void open_door(void);


#endif // GPIO_LIB_H
