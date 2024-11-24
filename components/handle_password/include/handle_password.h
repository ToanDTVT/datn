#ifndef HANDLE_PASSWORD_H
#define HANDLE_PASSWORD_H

#include "gpio_lib.h"
#include "uart_lib.h"
#include "user_data.h"
//#include "wireless.h"

extern DataPacket USER[MAX_USERS];
extern Data_send_server data;
extern Type_open_door action;

extern volatile enum statemachine currentstate;

extern char correct_password[SIZE_OF_PASSWORD];
extern char enter_password[SIZE_OF_PASSWORD];
extern int number_fail;

void handle_setting_password(char *enter_password);
int check_password(char *enter_password, char press_keypad_2);
void handle_input_password_idle(char *enter_password);
int check_pass_open_door(char *enter_password);

void password_task (void * pvParameters);
void create_password_task(void);


#endif //HANDLE_PASSWORD_H
