#include <stdio.h>
#include "uart_lib.h"
#include "gpio_lib.h"
#include "i2c_lib.h"
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


// void gpio_task(void *pvParameters) {
//     gpio_set_level(DOORBELL_BUTTON, 0);
//     gpio_set_level(DOORBELL, 0);
//     char press = 0;
//     while(1) {
//         press = keypad_get_char();
//         if(gpio_get_level(DOORBELL_BUTTON) == 1) {
//             gpio_set_level(DOORBELL, 1);
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//             gpio_set_level(DOORBELL, 0);
//             vTaskDelay(500 / portTICK_PERIOD_MS);
//             gpio_set_level(DOORBELL, 1);
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//             gpio_set_level(DOORBELL, 0);
//         }
//         if(press == '#'){
//             currentstate = STATE_IDLE;
//         }
//     }
// }



static esp_err_t i2c_master_init(void)
{

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    i2c_param_config(I2C_NUM_0, &conf);

    return i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}

void app_main(void)
{
    init_nvs();
    gpio_init();
    uart_init();
    i2c_master_init();
    lcd_init();
    lcd_clear();
    keypad_init();
    //nvs_flash_init();
    task_wifi_init();
    //obtain_time();
    create_http_task();
    mqtt_app_start();
    
    load_students_from_flash(students, MAX_STUDENTS);

    verify_password_of_AS608();

    char press_keypad = 0;

    while(1) {

        switch(currentstate) {
            case STATE_IDLE:
                vTaskDelay(300/portTICK_PERIOD_MS);
                press_keypad = 0;
                // Đợi nút nhấn bất kì
                if(is_wifi_connected() == true){
                    lcd_clear();  
                    lcd_put_cur(1, 3); lcd_send_string("ACCESS CONTROL");
                    lcd_put_cur(2, 5); lcd_send_string("    SYSTEM");
                }
                if(is_wifi_connected() == false){
                    lcd_clear();  
                    //lcd_put_cur(0, 0); lcd_send_string("WIFI IS NOT CONNECTED!!");
                    lcd_put_cur(1, 3); lcd_send_string("WIFI IS NOT");
                    lcd_put_cur(2, 5); lcd_send_string("    CONNECTED!!");
                    // ESP_ERROR_CHECK(esp_wifi_start());
                    // vTaskDelay(pdMS_TO_TICKS(1000));
                    // ESP_ERROR_CHECK(esp_wifi_connect());
                }
                // lcd_clear();  
                // lcd_put_cur(1, 3); lcd_send_string("ACCESS CONTROL");
                // lcd_put_cur(2, 5); lcd_send_string("    SYSTEM");
                press_keypad = keypad_get_char();
                if(press_keypad != 0) {
                    esp32_pub_for_esp32cam_sub_MSSV_ENTER();
                    lcd_clear();
                    currentstate = STATE_MSSV;
                }
                vTaskDelay(300/portTICK_PERIOD_MS);

                // uint8_t buffer_data_fingerprint[6][139];
                // PS_UpChar(1, buffer_data_fingerprint[6][139]);
            break;

            case STATE_MSSV:
                int i = 0;
                memset(MSSV, 0, sizeof(MSSV));         //Reset MSSV
                printf("--NHẬP MÃ SỐ SINH VIÊN-- \n");
                lcd_clear();
                lcd_put_cur(1, 0); lcd_send_string("ENTER STUDENT ID");
                lcd_put_cur(2, 3);
                while(1) {
                    press_keypad = keypad_get_char();
                    
                    if((press_keypad != 0) && (press_keypad != '*') && (press_keypad != '#')){
                        printf("%c \n", press_keypad);
                        MSSV[i] = press_keypad;
                        lcd_send_data(press_keypad);
                        press_keypad = 0;
                        i++;
                    }
                    if(press_keypad == '*'){
                        printf("%c \n", press_keypad);
                        lcd_send_data(press_keypad);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        break;
                    }
                    if(press_keypad == '#') {currentstate = STATE_IDLE; press_keypad = 0; break;}
                    vTaskDelay(300/portTICK_PERIOD_MS);
                }
                if(currentstate == STATE_IDLE) {break;}

                for(int x = 0; x <= MAX_STUDENTS; x++){
                    if((strcmp(MSSV, students[x].student_id) == 0) && (students[x].id != 0)) {
                        printf("--THÔNG TIN SINH VIÊN-- \n");
                        printf("id: %d \n", students[x].id);
                        printf("name: %s \n", students[x].full_name);
                        printf("mssv: %s \n", students[x].student_id);
                        printf("position: %s \n", students[x].position);
                        
                        lcd_clear();
                        lcd_put_cur(0, 2); lcd_send_string("--INFORMATION--");
                        lcd_put_cur(1, 0); lcd_send_string("Name: "); lcd_send_string(students[x].full_name);
                        lcd_put_cur(2, 0); lcd_send_string("MSSV: "); lcd_send_string(students[x].student_id);
                        lcd_put_cur(3, 0); lcd_send_string("Position: "); lcd_send_string(students[x].position);

                        esp32_pub_for_esp32cam_sub_DISPLAY_INFOR(MSSV);
                        vTaskDelay(3000/portTICK_PERIOD_MS);
                        student_x = x;
                        currentstate = STATE_CHECK_INFOR;
                        break;
                    }
                    if(x == MAX_STUDENTS) {
                        ESP_LOGW(TAG, "SINH VIÊN KHÔNG TỒN TẠI");
                        ESP_LOGW(TAG, "THỬ LẠI");
                        lcd_clear();
                        lcd_put_cur(0, 3); lcd_send_string("STUDENT DOES");
                        lcd_put_cur(1, 3); lcd_send_string("NOT EXCIST!");
                        lcd_put_cur(3, 3); lcd_send_string("TRY AGAIN !");
                        vTaskDelay(3000/portTICK_PERIOD_MS);
                    }
                }

            break;

            case STATE_CHECK_INFOR:
                if(press_keypad == '#') {currentstate = STATE_IDLE; press_keypad = 0; break;}
                // fetch_access_time(students[student_x].student_id);
                // if (is_access_allowed(students[student_x].allowed_days, students[student_x].start_time, students[student_x].end_time)) {
                    if((students[student_x].pass_en == 1) && (students[student_x].fing_en == 1)) {
                        currentstate = STATE_HANDLE_PASSWORD; break;
                    }
                    if((students[student_x].pass_en == 1) && (students[student_x].fing_en == 0)) {currentstate = STATE_HANDLE_PASSWORD; break;}
                    if((students[student_x].pass_en == 0) && (students[student_x].fing_en == 1)) {currentstate = STATE_HANDLE_FINGERPRINT; break;}
                    if((students[student_x].pass_en == 0) && (students[student_x].fing_en == 0)) {
                        currentstate = STATE_MSSV; 
                        ESP_LOGW(TAG, "SINH VIÊN %s KHÔNG ĐƯỢC VÀO", students[student_x].student_id);
                        lcd_clear();
                        lcd_put_cur(1, 3); lcd_send_string("STUDENT "); lcd_send_string(students[student_x].student_id);
                        lcd_put_cur(2, 3); lcd_send_string("IS NOT ALLOW IN !");
                        vTaskDelay(3000/portTICK_PERIOD_MS);
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
                lcd_clear();
                lcd_put_cur(0, 0); lcd_send_string("STUDENT: "); lcd_send_string(students[student_x].student_id);
                lcd_put_cur(1, 3); lcd_send_string("ENTER PASSWORD");
                lcd_put_cur(2, 3);
                while(1) {
                    press_keypad = keypad_get_char();
    
                    if((press_keypad != 0) && (press_keypad != '*') && (press_keypad != '#')){
                        printf("%c \n", press_keypad);
                        enter_password[y] = press_keypad;
                        lcd_send_data(press_keypad);
                        press_keypad = 0;
                        y++;
                    }
                    if(press_keypad == '*'){
                        printf("%c \n", press_keypad);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        break;
                    }
                    if(press_keypad == '#') {currentstate = STATE_IDLE; press_keypad = 0; break;}
                    vTaskDelay(300/portTICK_PERIOD_MS);
                }
                if(currentstate == STATE_IDLE) {break;}

                if(strcmp(enter_password, students[student_x].password) == 0) {
                    ESP_LOGI(TAG, "NHẬP MẬT KHẨU THÀNH CÔNG !!");
                    lcd_put_cur(3, 3); lcd_send_string("CORRECT PASSWORD");
                    if(students[student_x].fing_en == 1) {
                        currentstate = STATE_HANDLE_FINGERPRINT;
                        break;
                    } else {
                        currentstate = STATE_LOG_IN;
                        break;
                    }
                } else {
                    ESP_LOGW(TAG, "MẬT KHẨU KHÔNG HỢP LỆ");
                    lcd_put_cur(3, 3); lcd_send_string("WRONG PASSWORD");
                }       
            break;

            case STATE_HANDLE_FINGERPRINT:
                uint8_t status;
                ESP_LOGW(TAG, "Place your finger on the sensor.");
                lcd_clear();
                lcd_put_cur(0, 0); lcd_send_string("STUDENT: "); lcd_send_string(students[student_x].student_id);
                lcd_put_cur(1, 2); lcd_send_string("ENTER FINGERPRINT: ");
                lcd_put_cur(2, 3);
                do {
                    press_keypad = keypad_get_char();
                    if(press_keypad == '#') {currentstate = STATE_IDLE; press_keypad = 0; break;}
                    status = PS_GetImage();
                } while (status != 0x00); // Chờ lấy ảnh thành công
                if(currentstate == STATE_IDLE) {break;}

                status = PS_GenChar(1); // Tạo đặc trưng ở buffer 1
                if (status != 0x00) {
                //    ESP_LOGE(TAG, "PS_GENCHAR_1: Failed to generate character from image. Error: %d\n", status);
                }

                uint8_t x = PS_Search(1, 0, 160); 
                if(x == students[student_x].fingerprint){
                    ESP_LOGI(TAG, "PS_SEARCH: Fingerprint found at position: %d\n", x);
                    char buf[20];
                    sprintf(buf, "ID FINGERPRINT: %d", students[student_x].fingerprint);
                    lcd_send_string(buf);
                    lcd_put_cur(3, 3); lcd_send_string("CORRECT FINGERPRINT");
                    currentstate = STATE_LOG_IN;
                } else {
                    ESP_LOGW(TAG, "Thất bại!! Thử lại!!");
                    lcd_put_cur(3, 3); lcd_send_string("WRONG FINGERPRINT");
                }
                vTaskDelay(300/portTICK_PERIOD_MS);
            break;

            case STATE_LOG_IN:
                send_login_request(student_x);
                lcd_clear();
                lcd_put_cur(1, 2); lcd_send_string("ACCESS SUCCESSFUL");
                lcd_put_cur(2, 3);
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
