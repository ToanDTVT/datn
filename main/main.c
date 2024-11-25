#include <stdio.h>
#include "uart_lib.h"
#include "gpio_lib.h"
#include "user_data.h"

#include "wifi_config.h"
#include "http_method.h"

// DataPacket USER[MAX_USERS];
// Data_send_server data;
// Type_open_door action;
//Student student;
Student students[MAX_STUDENTS];


volatile enum statemachine currentstate = STATE_IDLE;

static const char* TAG = "main";
char MSSV[20];
char enter_password[10];
int student_x;

// char correct_password[SIZE_OF_PASSWORD] = {1, '1', '1', '1', '1'};
// char enter_password[SIZE_OF_PASSWORD];
// int number_fail = 0;


void app_main(void)
{

    gpio_init();
    uart_init();
    keypad_init();

    //nvs_flash_init();
    task_wifi_init();
    create_http_task();
    
    verify_password_of_AS608();

    char press_keypad = 0;

    while(1) {

        switch(currentstate) {
            case STATE_IDLE:
                //Đợi nút nhấn bất kì  
                press_keypad = keypad_get_char();
                if(press_keypad != 0) {
                    currentstate = STATE_MSSV;
                }
                vTaskDelay(300/portTICK_PERIOD_MS);
            break;

            case STATE_MSSV:
                int i = 0;
                memset(MSSV, 0, sizeof(MSSV));         //Reset MSSV
                printf("--NHẬP MÃ SỐ SINH VIÊN-- \n");
                while(1) {
                    press_keypad = keypad_get_char();
                    if((press_keypad != 0) && (press_keypad != '*') && (press_keypad != '#')){
                        printf("%c \n", press_keypad);
                        MSSV[i] = press_keypad;
                        press_keypad = 0;
                        i++;
                    }
                    if(press_keypad == '*'){
                        printf("%c \n", press_keypad);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        break;
                    }
                    vTaskDelay(300/portTICK_PERIOD_MS);
                }

                for(int x = 0; x <= MAX_STUDENTS; x++){
                    if((strcmp(MSSV, students[x].student_id) == 0) && (students[x].id != 0)) {
                        printf("--THÔNG TIN SINH VIÊN--");
                        printf("id: %d \n", students[x].id);
                        printf("name: %s \n", students[x].full_name);
                        printf("mssv: %s \n", students[x].student_id);
                        printf("position: %s \n", students[x].position);

                        student_x = x;
                        currentstate = STATE_CHECK_INFOR;
                        break;
                    }
                    if(i == MAX_STUDENTS) {
                        ESP_LOGW(TAG, "SINH VIÊN KHÔNG TỒN TẠI");
                        ESP_LOGW(TAG, "THỬ LẠI");
                    }
                }
            break;

            case STATE_CHECK_INFOR:
                if((students[student_x].pass_en == 1) && (students[student_x].fing_en == 1)) {currentstate = STATE_HANDLE_PASSWORD; break;}
                if((students[student_x].pass_en == 1) && (students[student_x].fing_en == 0)) {currentstate = STATE_HANDLE_PASSWORD; break;}
                if((students[student_x].pass_en == 0) && (students[student_x].fing_en == 1)) {currentstate = STATE_HANDLE_FINGERPRINT; break;}
                if((students[student_x].pass_en == 0) && (students[student_x].fing_en == 0)) {
                    currentstate = STATE_MSSV; 
                    ESP_LOGW(TAG, "SINH VIÊN %s KHÔNG ĐƯỢC VÀO", students[student_x].student_id);
                    break;
                }
            break;

            case STATE_HANDLE_PASSWORD:
                int y = 0;
                memset(enter_password, 0, sizeof(enter_password));         //Reset MSSV
                printf("--NHẬP MẬT KHẨU-- \n");
                while(1) {
                    press_keypad = keypad_get_char();
                    if((press_keypad != 0) && (press_keypad != '*') && (press_keypad != '#')){
                        printf("%c \n", press_keypad);
                        enter_password[y] = press_keypad;
                        press_keypad = 0;
                        y++;
                    }
                    if(press_keypad == '*'){
                        printf("%c \n", press_keypad);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        break;
                    }
                    vTaskDelay(300/portTICK_PERIOD_MS);
                }

                if(strcmp(enter_password, students[student_x].password) == 0) {
                    ESP_LOGI(TAG, "NHẬP MẬT KHẨU THÀNH CÔNG !!");
                    if(students[student_x].fing_en == 1) {
                        currentstate = STATE_HANDLE_FINGERPRINT;
                        break;
                    } else {
                        currentstate = STATE_LOG_IN;
                        break;
                    }
                }        
            break;

            case STATE_HANDLE_FINGERPRINT:
                vTaskDelay(1000/portTICK_PERIOD_MS);
            break;

            case STATE_LOG_IN:
                open_door();
                vTaskDelay(1000/portTICK_PERIOD_MS);
                currentstate = STATE_IDLE;
            break;
        }

    }


}
