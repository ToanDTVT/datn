#include <stdio.h>
#include "handle_password.h"


 #define TAG "password"

// char correct_password[SIZE_OF_PASSWORD] = {1, '1', '1', '1', '1'};
// char enter_password[SIZE_OF_PASSWORD];
// int number_fail = 0;

TaskHandle_t password_task_handle = NULL;


int check_pass_open_door(char *enter_password){
    int i;
    for(i = 0; i <= MAX_USERS; i++){
        if((strcmp(enter_password, correct_password) == 0) || (strcmp(enter_password, USER[i].user_password) == 0)){
            if(USER[i].en == 0){
                break;
            }
            printf("=========================================== \n");
            printf("SUCCESSFUL!! OPEN THE DOOR! \n");
            printf("id: %d \n", USER[i].id);
            printf("=========================================== \n");

            action.pass_open_door = true;
            data.user = USER[i].id;
            strcpy(data.type, "password");

            // // Gửi JSON data lên ThingsBoard trên topic "v1/devices/me/telemetry"
            // char json_data[100];
            // snprintf(json_data, sizeof(json_data), "{\"user\": %d, \"type\": %s}", data.user, data.type);
            // int msg_id = esp_mqtt_client_publish(client, "v1/devices/me/telemetry", json_data, 0, 1, 0);
            // ESP_LOGI("MQTT", "Sent telemetry data, msg_id=%d", msg_id);

            return 1;    //Mat khau dung
        }
    }
    printf("=========================================== \n");
    printf("FAILURE!! TRY AGAIN! \n");
    printf("=========================================== \n");
    action.pass_open_door = false;
    return 0;   //Mat khau sai
}



void handle_input_password_idle(char *enter_password){

    int x = 1;
    char press_keypad;
    enter_password[0] = 1;
    printf("Password: \n");

    while(x < SIZE_OF_PASSWORD){
        press_keypad = keypad_get_char();
        if((press_keypad != 0) && (press_keypad != '*') && (press_keypad != '#')){
            printf("%c \n", press_keypad);
            enter_password[x] = press_keypad;
            press_keypad = 0;
            x++;
        }
        if(press_keypad == '*'){
            currentstate = STATE_SETTING;
            vTaskDelay(300/portTICK_PERIOD_MS);
            break;
        }
        vTaskDelay(300/portTICK_PERIOD_MS);
    }
    printf("\n");

    if(check_pass_open_door(enter_password) == 1){
        currentstate = STATE_OPENDOOR;
        number_fail = 0;
    }else number_fail++;

}


int check_password(char *enter_password, char press_keypad_2){

    if(press_keypad_2 == '1'){
        int i;
        for(i = 1; i <= MAX_USERS; i++){
            if(USER[i].id == 0){
                for(int z = 0; z < SIZE_OF_PASSWORD; z++){USER[i].user_password[z] = enter_password[z];}
                USER[i].en = 1;                           //cho phép
                USER[i].id = i;                           //người sử dụng thứ i 
                ESP_LOGI(TAG, "HAVE SLOT!! ADD SUCCESS !!");
                return 1;                                 //còn chỗ trống
            }
        }
        ESP_LOGW(TAG, "FULL SLOT!! TRY AGAIN !!");
        return 2;                                         //hết chỗ trống
    }
    
    if(press_keypad_2 == '2'){
        int i;
        for(i = 1; i <= MAX_USERS; i++){
            if((USER[i].id != 0) && (strcmp(enter_password, USER[i].user_password) == 0)){
                memset(USER[i].user_password, 0, sizeof(USER[i].user_password));
                USER[i].id = 0;                                                                  //người sử dụng thứ i
                ESP_LOGI(TAG, "DELETE SUCCESS !!");
                return 3;                                                                        //xóa thành công
            }
        }
        ESP_LOGW(TAG, "PASSWORD NO EXIST !!");
        return 4;                                                                                //Không thành công 
    }

    return 0;
}


void handle_setting_password(char *enter_password){
    
    ESP_LOGI(TAG, "SETTING PASSWORD: 1.ADD PASSWORD         2.DELETE PASSWORD");
    char press_keypad_2;
    vTaskDelay(500/portTICK_PERIOD_MS);

    while(1){
        press_keypad_2 = keypad_get_char();
        if(press_keypad_2 == '1'){ printf("ADD PASSWORD: \n"); break;}
        if(press_keypad_2 == '2'){ printf("DELETE PASSWORD: \n"); break;}
        vTaskDelay(500/portTICK_PERIOD_MS);
    }

    vTaskDelay(500/portTICK_PERIOD_MS);

    int i = 1;
    char press_keypad_3; 
    memset(enter_password, 0, 5);
    enter_password[0] = 1;
    while(i < SIZE_OF_PASSWORD){
        press_keypad_3 = keypad_get_char();
        if(press_keypad_3 != 0){
            printf("%c \n", press_keypad_3);
            enter_password[i] = press_keypad_3;
            press_keypad_3 = 0;
            i++;
        }
        vTaskDelay(300/portTICK_PERIOD_MS);
    }
    printf("\n");
    if((check_password(enter_password, press_keypad_2) == 1) || (check_password(enter_password, press_keypad_2) == 3)){
        currentstate = STATE_IDLE;
    }

}


void password_task (void * pvParameters){

    while(1){

        switch(currentstate){

            case STATE_IDLE:
                memset(enter_password, 0, sizeof(enter_password));
                handle_input_password_idle(enter_password);
                vTaskDelay(500 / portTICK_PERIOD_MS);
            break;

            case STATE_SETTING:
                ESP_LOGI(TAG, "SETTING:   1.PASSWORD     2.FINGERPRINT");
                while(1){
                    char press_keypad_1 = keypad_get_char();
                    if(press_keypad_1 == '1'){
                        currentstate = STATE_SETTING_1;
                        break;
                    }
                    if(press_keypad_1 == '2'){
                        currentstate = STATE_SETTING_2;
                        break;
                    }
                    vTaskDelay(500/portTICK_PERIOD_MS);
                }
            break;

            case STATE_SETTING_1:
                handle_setting_password(enter_password);
            break;

            case STATE_SETTING_2:
            break;

            case STATE_DISABLE:
            break;

            case STATE_OPENDOOR:
                open_door();
                vTaskDelay(1000/portTICK_PERIOD_MS);
                currentstate = STATE_IDLE;
            break;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}

void create_password_task(void){
    xTaskCreate(password_task, "password task", 2048, NULL, 3, &password_task_handle);
}


void func2(void)
{

}
