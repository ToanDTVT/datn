#include <stdio.h>
#include "uart_lib.h"
#include "gpio_lib.h"
#include "user_data.h"

#include "wifi_config.h"
#include "http_method.h"
#include "mqtt_method.h"

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
    func8();
    //nvs_flash_init();
    task_wifi_init();
    //obtain_time();
    create_http_task();
    mqtt_app_start();
    
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
                        printf("--THÔNG TIN SINH VIÊN-- \n");
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
                // fetch_access_time(students[student_x].student_id);
                // if (is_access_allowed(students[student_x].allowed_days, students[student_x].start_time, students[student_x].end_time)) {
                    if((students[student_x].pass_en == 1) && (students[student_x].fing_en == 1)) {currentstate = STATE_HANDLE_PASSWORD; break;}
                    if((students[student_x].pass_en == 1) && (students[student_x].fing_en == 0)) {currentstate = STATE_HANDLE_PASSWORD; break;}
                    if((students[student_x].pass_en == 0) && (students[student_x].fing_en == 1)) {currentstate = STATE_HANDLE_FINGERPRINT; break;}
                    if((students[student_x].pass_en == 0) && (students[student_x].fing_en == 0)) {
                        currentstate = STATE_MSSV; 
                        ESP_LOGW(TAG, "SINH VIÊN %s KHÔNG ĐƯỢC VÀO", students[student_x].student_id);
                        break;
                    }
                // } else {
                //     printf("Access denied.\n");
                // }
                
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
                } else {
                    ESP_LOGW(TAG, "MẬT KHẨU KHÔNG HỢP LỆ");
                }       
            break;

            case STATE_HANDLE_FINGERPRINT:
                uint8_t status;
                //ESP_LOGW(TAG, "Place your finger on the sensor.");
                do {
                    status = PS_GetImage();
                } while (status != 0x00); // Chờ lấy ảnh thành công
                status = PS_GenChar(1); // Tạo đặc trưng ở buffer 1
                if (status != 0x00) {
                //    ESP_LOGE(TAG, "PS_GENCHAR_1: Failed to generate character from image. Error: %d\n", status);
                }

                uint8_t x = PS_Search(1, 0, 160); 
                if(x == students[student_x].id){
                    ESP_LOGI(TAG, "PS_SEARCH: Fingerprint found at position: %d\n", x);
                    currentstate = STATE_LOG_IN;
                } else {
                    ESP_LOGW(TAG, "Thất bại!! Thử lại!!");
                }
                vTaskDelay(1000/portTICK_PERIOD_MS);
            break;

            case STATE_LOG_IN:
                send_login_request(student_x);
                open_door();
                vTaskDelay(1000/portTICK_PERIOD_MS);
                currentstate = STATE_IDLE;
            break;

            case STATE_SIGN_IN:
                PS_Enroll(students[student_x].id);
                ESP_LOGI(TAG, "Đăng ký vân tay thành công !!");

                currentstate = STATE_IDLE;
                vTaskDelay(1000/portTICK_PERIOD_MS);
            break;
        }

    }


}
