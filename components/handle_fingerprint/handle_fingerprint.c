#include <stdio.h>
#include "handle_fingerprint.h"

static const char* TAG = "handle_fingerprint";

TaskHandle_t fingerprint_task_handle = NULL;




void handle_input_fingerprint_idle(){

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
    if(x != 0){
        if(USER[x].en == 1){
            ESP_LOGI(TAG, "PS_SEARCH: Fingerprint found at position: %d\n", x);
            currentstate = STATE_OPENDOOR;
        }
        else if(USER[x].en == 0){
            ESP_LOGI(TAG, "PS_SEARCH: Fingerprint not found.\n");
            currentstate = STATE_IDLE;
        }
    }
    
}



void handle_setting_fingerprint(){
    ESP_LOGI(TAG, "SETTING FINGERPRINT:  1.ADD FINGERPRINT       2.DELETE FINGERPRINT");
    char press_keypad_2;
    vTaskDelay(500 / portTICK_PERIOD_MS);
    while(1){
        press_keypad_2 = keypad_get_char();
        vTaskDelay(500/portTICK_PERIOD_MS);
        if(press_keypad_2 == '1'){ 
            printf("ADD FINGERPRINT: \n");
            printf("1.NEW USER        2.OLD USER \n");
            char press_keypad_3;

            do{
                press_keypad_3 = keypad_get_char();
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }while(press_keypad_3 == 0);
            
            if(press_keypad_3 == '1'){
                ESP_LOGW(TAG, "ADD NEW USER FINGER: ");
                for(int i = 1; i <= MAX_USERS; i++){
                    if((USER[i].id == 0)){
                        if(USER[i].fingerprint_enable == 0){
                            PS_Enroll(i);
                            USER[i].id = i;
                            USER[i].en = 1;
                            USER[i].fingerprint_enable = 1;
                            ESP_LOGI(TAG, "ADD FINGERPRINT SUCCESS !!");
                            currentstate = STATE_IDLE;
                            break;
                        }
                    }
                }
            }
            
            if(press_keypad_3 == '2'){
                ESP_LOGW(TAG, "ADD OLD USER FINGER: ");
                uint8_t p = choose_user();
                if(USER[p].id == p){
                    PS_Enroll(p);
                    USER[p].fingerprint_enable = 1;
                    ESP_LOGI(TAG, "ADD FINGERPRINT SUCCESS !!");
                    press_keypad_3 = 0;
                    currentstate = STATE_IDLE;
                    break;
                }else{
                    ESP_LOGI(TAG, "USER NO EXIST !! ");
                    press_keypad_3 = 0;
                    currentstate = STATE_IDLE;
                    break;
                }
            }
            press_keypad_2 = 0;
            break;
        }

        if(press_keypad_2 == '2'){
            printf("DELETE FINGERPRINT: \n"); 
            uint8_t p = choose_user();
            if(USER[p].id == p){
                PS_Delete(p, 1);
                ESP_LOGI(TAG, "DELETE FINGERPRINT OF USER %d", p);
                currentstate = STATE_IDLE;
            }else{
                ESP_LOGI(TAG, "USER NO EXIST !! ");
                currentstate = STATE_IDLE;
            }
            press_keypad_2 = 0;
            break;
        }
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}


void fingerprint_task(){

    //if(verify_password_of_AS608() == true){

        while(1){

            switch(currentstate){

                case STATE_IDLE:
                    handle_input_fingerprint_idle();
                break;

                case STATE_SETTING:
                break;

                case STATE_SETTING_1:
                break;

                case STATE_SETTING_2:
                    handle_setting_fingerprint();
                break;

                case STATE_DISABLE:
                break;

                case STATE_OPENDOOR:
                    open_door();
                    vTaskDelay(1000/portTICK_PERIOD_MS);
                    currentstate = STATE_IDLE;
                break;
            }

        }
        
    // }else{

    //     printf("Did not find fingerprint sensor \n");

    // }

    // vTaskDelay(1000/portTICK_PERIOD_MS);

}

void create_fingerprint_task(void){
    xTaskCreate(fingerprint_task, "fingerprint task ", 2048, NULL, 5, &fingerprint_task_handle);
}

void func3(void){}