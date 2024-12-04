#ifndef USER_DATA_H
#define USER_DATA_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "gpio_lib.h"

// #define MAX_USERS 10
// #define SIZE_OF_PASSWORD 5
#define MAX_STUDENTS 10

// typedef struct {
// 	uint8_t en;
//     uint8_t id;
// 	uint8_t password_enable;
// 	uint8_t fingerprint_enable;
//     char user_password[SIZE_OF_PASSWORD];
//     uint8_t user_fingerprint[6][139];
// } DataPacket;


// typedef struct {
// 	uint8_t user;
// 	char type[15];
// }Data_send_server;

// typedef struct {
// 	bool pass_open_door;
// 	bool fing_open_door;
// }Type_open_door;

// enum statemachine {
// 	STATE_IDLE,
// 	STATE_OPENDOOR,
// 	STATE_DISABLE,
// 	STATE_SETTING,
// 	STATE_SETTING_1,
// 	STATE_SETTING_2
// };


enum statemachine {
	STATE_IDLE,
	STATE_MSSV,
	STATE_CHECK_INFOR,
	STATE_HANDLE_PASSWORD,
	STATE_HANDLE_FINGERPRINT,
	STATE_LOG_IN,
	STATE_SIGN_IN
};



typedef struct {
    int id;
    char full_name[50];
    char student_id[20];
    char position[20];
    char email[50];
	char password[10];
	int fingerprint;
	char allowed_days[50];
	char start_time[5];
	char end_time[5];
	int pass_en;
	int fing_en;
} Student;

//int student_count;

//uint8_t choose_user();
void func(void);


#endif //USER_DATA_H