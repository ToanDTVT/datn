
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_compiler.h"
#include "esp_log.h"
#include "driver/dedic_gpio.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_rom_sys.h"
#include "user_data.h"

#define UART_NUM UART_NUM_1
#define UART_NUM2 UART_NUM_2 
#define BUF_SIZE 1024
#define RD_BUF_SIZE (BUF_SIZE)
#define TX_PIN GPIO_NUM_17
#define RX_PIN GPIO_NUM_16

#define TX_PIN_2 GPIO_NUM_4
#define RX_PIN_2 GPIO_NUM_5

void uart_init();
void uart2_init();


void send_data_to_rasp(void);

// uint16_t calculate_checksum(uint8_t *packet, int length);
// void send_command(uint8_t command, uint8_t *data, int data_len);
// void receive_response(uint8_t *response, int len);

uint8_t PS_GetImage();
uint8_t PS_GenChar(uint8_t buffer_id);
uint8_t PS_RegModel();

uint8_t PS_Store(uint8_t buffer_id, uint16_t page_id);
void PS_UpChar(uint8_t buffer_id, uint8_t buffer_data_fingerprint[6][139]);
void PS_DownChar(uint8_t buffer_id, uint8_t buffer_data_fingerprint[6][139]);

void PS_Enroll(uint16_t page_id);
uint8_t PS_Search(uint8_t buffer_id, uint16_t start_page, uint16_t page_num);
void PS_Delete(uint16_t page_id, uint16_t num);
bool verify_password_of_AS608();