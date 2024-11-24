#include <stdio.h>
#include "gpio_lib.h"


void gpio_init(){

    /*Config GPIO for keypad*/
    gpio_config_t io_conf_row_keypad;
    io_conf_row_keypad.intr_type = GPIO_INTR_DISABLE;                                                     // Không có ngắt
    io_conf_row_keypad.mode = GPIO_MODE_OUTPUT;                                                           // Chế độ đầu ra
    io_conf_row_keypad.pin_bit_mask = (1ULL << ROW_1)|(1ULL << ROW_2)|(1ULL << ROW_3)|(1ULL << ROW_4)|(1ULL << RELAY);    // Chọn chân GPIO
    io_conf_row_keypad.pull_down_en = 1;                                                                  // kéo xuống
    io_conf_row_keypad.pull_up_en = 0;                                                                    // Không kéo lên
    gpio_config(&io_conf_row_keypad); 

    gpio_config_t io_conf_col_keypad;
    io_conf_col_keypad.intr_type = GPIO_INTR_DISABLE;                                                     // Không có ngắt
    io_conf_col_keypad.mode = GPIO_MODE_INPUT;                                                            // Chế độ đầu vào 
    io_conf_col_keypad.pin_bit_mask = (1ULL << COL_1)|(1ULL << COL_2)|(1ULL << COL_3);                    // Chọn chân GPIO
    io_conf_col_keypad.pull_down_en = 1;                                                                  // kéo xuống
    io_conf_col_keypad.pull_up_en = 0;                                                                    // Không kéo lên
    gpio_config(&io_conf_col_keypad);
}



char keypad_value[4][3] = {
                            {'1','2','3'},
                            {'4','5','6'},
                            {'7','8','9'},
                            {'*','0','#'}
                                        };

char keypad_get_char(){

    /*check ROW 1*/
    gpio_set_level(ROW_1, 1);
    gpio_set_level(ROW_2, 0);
    gpio_set_level(ROW_3, 0);
    gpio_set_level(ROW_4, 0);
    if(gpio_get_level(COL_1) == 1) {return keypad_value[0][0];}
        else if(gpio_get_level(COL_2) == 1) {return keypad_value[0][1];}
            else if(gpio_get_level(COL_3) == 1) {return keypad_value[0][2];}

    /*check ROW 2*/
    gpio_set_level(ROW_1, 0);
    gpio_set_level(ROW_2, 1);
    gpio_set_level(ROW_3, 0);
    gpio_set_level(ROW_4, 0);
    if(gpio_get_level(COL_1) == 1) {return keypad_value[1][0];}
        else if(gpio_get_level(COL_2) == 1) {return keypad_value[1][1];}
            else if(gpio_get_level(COL_3) == 1) {return keypad_value[1][2];}

    /*check ROW 3*/
    gpio_set_level(ROW_1, 0);
    gpio_set_level(ROW_2, 0);
    gpio_set_level(ROW_3, 1);
    gpio_set_level(ROW_4, 0);
    if(gpio_get_level(COL_1) == 1) {return keypad_value[2][0];}
        else if(gpio_get_level(COL_2) == 1) {return keypad_value[2][1];}
            else if(gpio_get_level(COL_3) == 1) {return keypad_value[2][2];}

    /*check ROW 4*/
    gpio_set_level(ROW_1, 0);
    gpio_set_level(ROW_2, 0);
    gpio_set_level(ROW_3, 0);
    gpio_set_level(ROW_4, 1);
    if(gpio_get_level(COL_1) == 1) {return keypad_value[3][0];}
        else if(gpio_get_level(COL_2) == 1) {return keypad_value[3][1];}
            else if(gpio_get_level(COL_3) == 1) {return keypad_value[3][2];}

    return '\0';
}


void keypad_init(){
    /*set all pin ROW to high level*/
    gpio_set_level(ROW_1, 1);
    gpio_set_level(ROW_2, 1);
    gpio_set_level(ROW_3, 1);
    gpio_set_level(ROW_4, 1);

    gpio_set_level(COL_1, 0);
    gpio_set_level(COL_2, 0);
    gpio_set_level(COL_3, 0);
}


void open_door(void){
    gpio_set_level(RELAY, 1);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    gpio_set_level(RELAY, 0);
}


void func1(void)
{

}
