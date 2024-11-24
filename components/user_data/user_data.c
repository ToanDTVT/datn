#include <stdio.h>
#include "user_data.h"


uint8_t choose_user(){
    uint8_t userID = 0;
    char press_keypad = 0;
    printf("USER ID: \n");
    while(1){
        press_keypad = keypad_get_char();
        if((press_keypad >= '0') && (press_keypad <= '9')){
            printf("%c", press_keypad);
            userID = userID*10 + (press_keypad - '0');
        }else if(press_keypad == '#'){
            break;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    printf("\n");
    return userID;
}

void func(void)
{

}
