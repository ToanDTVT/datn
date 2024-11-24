#include <stdio.h>
#include <stdint.h>
#include "uart_lib.h"


static const char* TAG = "UART";

void uart_init() {
    const uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    
    // Cấu hình UART
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
}


void uart2_init() {
    const uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    
    // Cấu hình UART
    ESP_ERROR_CHECK(uart_param_config(UART_NUM2, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM2, TX_PIN_2, RX_PIN_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM2, BUF_SIZE * 2, 0, 0, NULL, 0));
}

void send_data_to_rasp(void) {
    DataPacket command;  // Initialize the structure (add actual data if needed)
    command.id = 0;
    command.fingerprint_enable = 1;
    command.password_enable = 1;


    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 139; j++) {
            command.user_fingerprint[i][j] = 3;  // Giá trị ngẫu nhiên từ 0 đến 255
        }
    }


    for (int i = 0; i < SIZE_OF_PASSWORD - 1; i++) {
        command.user_password[i] = 7;  // Ký tự ngẫu nhiên từ 'A' đến 'Z'
    }
    command.user_password[SIZE_OF_PASSWORD - 1] = '\0';


    size_t packet_size = sizeof(command);  // Get the size of the structure
    
    uint8_t buffer[packet_size];
    memcpy(buffer, &command, packet_size);  // Copy the structure data into the buffer
    
    uart_write_bytes(UART_NUM2, (const char*) buffer, packet_size);  // Send buffer over UART
}

bool verify_password_of_AS608(){
    uint8_t command[] = {
        0xEF, 0x01,                                   // Header (2 bytes)
        0xFF, 0xFF, 0xFF, 0xFF,                       // Address (4 bytes)
        0x01,                                         // Packet Identifier (1 bytes)
        0x00, 0x07,                                   // Length (2 bytes)
        0x13,                                         // Intruction Code (command) (1 bytes)
        0x00, 0x00, 0x00, 0x00,                       // Data (N bytes)
        0x00, 0x1B                                    // Checksum (2 bytes)
    };
    uart_write_bytes(UART_NUM, (const char*) command, sizeof(command));
    // printf("Send %d byte: ", sizeof(command));
    // for (int t = 0; t < sizeof(command); t++){
    //     printf("%02x ", command[t]);
    // }
    // printf("\n");

    uint8_t response[128];
    uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    // int length = uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    // if (length > 0) {
    //     printf("Received %d bytes: ", length);
    //     for (int i = 0; i < length; i++) {
    //         printf("%02X ", response[i]);
    //     }
    //     printf("\n");
    // }
    if(response[9] == 0x00){
        return true;
    }else return false;
}


// Lấy ảnh vân tay từ cảm biến
uint8_t PS_GetImage() {

    uint8_t command[] = {
        0xEF, 0x01,                                 // Header (2 bytes)
        0xFF, 0xFF, 0xFF, 0xFF,                     // Address (4 bytes)
        0x01,                                       // Packet Identifier (1 bytes)
        0x00, 0x03,                                 // Length (2 bytes)
        0x01,                                       // Intruction Code (1 bytes)
        0x00, 0x05                                  // Checksum (2 bytes)
    };
    uart_write_bytes(UART_NUM, (const char*) command, sizeof(command));
    // printf("Send %d byte: ", sizeof(command));
    // for (int t = 0; t < sizeof(command); t++){
    //     printf("%02x ", command[t]);
    // }
    //printf("\n");

    uint8_t response[12];
    uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    // int length = uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    // if (length > 0) {
    //     printf("Received %d bytes: ", length);
    //     for (int i = 0; i < length; i++) {
    //         printf("%02X ", response[i]);
    //     }
    //     printf("\n");
    // }
    //ESP_LOGW(TAG, "PS_GETIMAGE: response: %02x", response[9]);
    return response[9]; // Mã phản hồi
}


// Tạo đặc trưng vân tay từ hình ảnh (buffer_id = 1 hoặc 2)
uint8_t PS_GenChar(uint8_t buffer_id) {
    //uint8_t data[1] = {buffer_id};
    uint8_t command[] = {
        0xEF, 0x01,                            // Header (2 bytes)
        0xFF, 0xFF, 0xFF, 0xFF,                // Address (4 bytes)
        0x01,                                  // Packet Identifier (1 bytes)
        0x00, 0x04,                            // Length (2 bytes)
        0x02,                                  // Intruction Code (1 bytes)
        buffer_id,                             // CharBuffer1 or CharBuffer2 (1 bytes)
        0x00, (0x07 + buffer_id)               // Checksum (2 bytes)
    };
    uart_write_bytes(UART_NUM, (const char*) command, sizeof(command));
    
    uint8_t response[128];
    uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    // int length = uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    // if (length > 0) {
    //     printf("Received %d bytes: ", length);
    //     for (int i = 0; i < length; i++) {
    //         printf("%02X ", response[i]);
    //     }
    //     printf("\n");
    // }
    //ESP_LOGW(TAG, "PS_GENCHAR: response: %02x", response[9]);
    return response[9]; // Mã phản hồi
}


// Kết hợp hai đặc trưng vân tay thành mẫu
uint8_t PS_RegModel() {
    //send_command(0x03, NULL, 0); // Gửi lệnh kết hợp đặc trưng vân tay
    uint8_t command[] = {
        0xEF, 0x01,                       // Header (2 bytes)
        0xFF, 0xFF, 0xFF, 0xFF,           // Address (4 bytes)
        0x01,                             // Packet Identifier (1 bytes)
        0x00, 0x03,                       // Length (2 bytes)
        0x05,                             // Intruction Code (1 bytes)
        0x00, 0x09                        // Checksum (2 bytes)
    };
    uart_write_bytes(UART_NUM, (const char*) command, sizeof(command));
    // printf("Send %d byte: ", sizeof(command));
    // for (int t = 0; t < sizeof(command); t++){
    //     printf("%02x ", command[t]);
    // }
    // printf("\n");

    uint8_t response[128];
    uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    // int length = uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    // if (length > 0) {
    //     printf("Received %d bytes: ", length);
    //     for (int i = 0; i < length; i++) {
    //         printf("%02X ", response[i]);
    //     }
    //     printf("\n");
    // }
    //ESP_LOGW(TAG, "PS_REGMODEL: response: %02x", response[9]);
    return response[9]; // Mã phản hồi
}



// Lưu mẫu vân tay vào bộ nhớ (page_id: vị trí lưu trữ)
uint8_t PS_Store(uint8_t buffer_id, uint16_t page_id) {
    // uint8_t data[3];
    // data[0] = buffer_id;
    // data[1] = (page_id >> 8) & 0xFF;
    // data[2] = page_id & 0xFF;
    // send_command(0x06, data, 3); // Gửi lệnh lưu mẫu vân tay
    uint8_t high_byte_page_id = (page_id >> 8) & 0xFF; 
    uint8_t low_byte_page_id = page_id & 0xFF ;
    uint16_t check_sum = 0x01 + 0x06 + 0x06 + buffer_id + page_id;
    uint8_t command[] = {
        0xEF, 0x01,                                              // Header (2 bytes)
        0xFF, 0xFF, 0xFF, 0xFF,                                  // Address (4 bytes)
        0x01,                                                    // Packet Identifier (1 bytes)
        0x00, 0x06,                                              // Length (2 bytes)
        0x06,                                                    // Intruction Code (1 bytes)
        buffer_id,                                               // CharBuffer1 or CharBuffer2 (1 bytes) (buffer nào được chọn để gáng đặc trưng)
        high_byte_page_id, low_byte_page_id,                     // Page ID (2 bytes) (page được chọn để lưu dữ liệu vân tay từ buffer được chọn)
        (check_sum >> 8) & 0xFF, (check_sum & 0xFF)              // Checksum (2 bytes)
    };
    uart_write_bytes(UART_NUM, (const char*) command, sizeof(command));
    // printf("Send %d byte: ", sizeof(command));
    // for (int t = 0; t < sizeof(command); t++){
    //     printf("%02x ", command[t]);
    // }
    // printf("\n");

    uint8_t response[128];
    uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    // int length = uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    // if (length > 0) {
    //     printf("Received %d bytes: ", length);
    //     for (int i = 0; i < length; i++) {
    //         printf("%02X ", response[i]);
    //     }
    //     printf("\n");
    // }
    ESP_LOGW(TAG, "PS_STORE: response: %02x", response[9]);
    if (response[9] == 0x00) {
        ESP_LOGI(TAG, "PS_STORE: Fingerprint stored successfully.");
    } else {
        ESP_LOGI(TAG, "PS_STORE: Failed to store fingerprint.");
    }
    
    return response[9]; // Mã phản hồi
}


void PS_UpChar(uint8_t buffer_id, uint8_t buffer_data_fingerprint[6][139]){
    uint16_t checksum = 0x01 + 0x04 + 0x08 + buffer_id;
    uint8_t command[] = {
        0xEF, 0x01,                                      // Header (2 bytes)
        0xFF, 0xFF, 0xFF, 0xFF,                          // Address (4 bytes)
        0x01,                                            // Packet Identifier (1 bytes)
        0x00, 0x04,                                      // Length (2 bytes)
        0x08,                                            // Intruction Code (1 bytes)
        buffer_id,                                       // CharBuffer được chọn để lưu đặc trưng (1 or 2) (1 bytes)
        ((checksum >> 8) & 0xFF), (checksum & 0xFF)      // Checksum (2 bytes)
    };
    uart_write_bytes(UART_NUM, (const char*) command, sizeof(command));

    uint8_t ACK_packet[12];  //gói ACK gồm 12 bytes
    //uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    int ACK_len = uart_read_bytes(UART_NUM, ACK_packet, sizeof(ACK_packet), 1000 / portTICK_PERIOD_MS);
    ESP_LOGW(TAG, "PS_UPCHAR_ACK: response: %02x", ACK_packet[9]);
    if (ACK_len > 0) {
        printf("Received %d bytes ACK: ", ACK_len);
        for (int i = 0; i < ACK_len; i++) {
            printf("%02X ", ACK_packet[i]);
        }
        printf("\n");
    }
    
    uint8_t Data_packet_1[139]; 
    uint8_t Data_packet_2[139];
    uint8_t Data_packet_3[139]; 
    uint8_t Data_packet_4[139];
    uint8_t Data_packet_5[139];
    uint8_t Data_packet_6[139];
    //uint8_t Data_packet_7[139];
    
    int Data_len_1 = uart_read_bytes(UART_NUM, Data_packet_1, sizeof(Data_packet_1), 1000 / portTICK_PERIOD_MS);
    ESP_LOGW(TAG, "PS_UPCHAR_DATA_1: Packet Identifier: %02x", Data_packet_1[6]);
    if (Data_len_1 > 0) {
        printf("Received %d bytes Data: ", Data_len_1);
        for (int i = 0; i < Data_len_1; i++) {
            buffer_data_fingerprint[0][i] = Data_packet_1[i];
            printf("%02X ", Data_packet_1[i]);
        }
        printf("\n");
    }

    int Data_len_2 = uart_read_bytes(UART_NUM, Data_packet_2, sizeof(Data_packet_2), 1000 / portTICK_PERIOD_MS);
    ESP_LOGW(TAG, "PS_UPCHAR_DATA_2: Packet Identifier: %02x", Data_packet_2[6]);
    if (Data_len_2 > 0) {
        printf("Received %d bytes Data: ", Data_len_2);
        for (int i = 0; i < Data_len_2; i++) {
            buffer_data_fingerprint[1][i] = Data_packet_2[i];
            printf("%02X ", Data_packet_2[i]);
        }
        printf("\n");
    }

    int Data_len_3 = uart_read_bytes(UART_NUM, Data_packet_3, sizeof(Data_packet_3), 1000 / portTICK_PERIOD_MS);
    ESP_LOGW(TAG, "PS_UPCHAR_DATA_3: Packet Identifier: %02x", Data_packet_3[6]);
    if (Data_len_3 > 0) {
        printf("Received %d bytes Data: ", Data_len_3);
        for (int i = 0; i < Data_len_3; i++) {
            buffer_data_fingerprint[2][i] = Data_packet_3[i];
            printf("%02X ", Data_packet_3[i]);
        }
        printf("\n");
    }

    int Data_len_4 = uart_read_bytes(UART_NUM, Data_packet_4, sizeof(Data_packet_4), 1000 / portTICK_PERIOD_MS);
    ESP_LOGW(TAG, "PS_UPCHAR_DATA_4: Packet Identifier: %02x", Data_packet_4[6]);
    if (Data_len_4 > 0) {
        printf("Received %d bytes Data: ", Data_len_4);
        for (int i = 0; i < Data_len_4; i++) {
            buffer_data_fingerprint[3][i] = Data_packet_4[i];
            printf("%02X ", Data_packet_4[i]);
        }
        printf("\n");
    }
    

    int Data_len_5 = uart_read_bytes(UART_NUM, Data_packet_5, sizeof(Data_packet_5), 1000 / portTICK_PERIOD_MS);
    ESP_LOGW(TAG, "PS_UPCHAR_DATA_5: Packet Identifier: %02x", Data_packet_5[6]);
    if (Data_len_5 > 0) {
        printf("Received %d bytes Data: ", Data_len_5);
        for (int i = 0; i < Data_len_5; i++) {
            buffer_data_fingerprint[4][i] = Data_packet_5[i];
            printf("%02X ", Data_packet_5[i]);
        }
        printf("\n");
    }


    int Data_len_6 = uart_read_bytes(UART_NUM, Data_packet_6, sizeof(Data_packet_6), 1000 / portTICK_PERIOD_MS);
    ESP_LOGW(TAG, "PS_UPCHAR_DATA_6: Packet Identifier: %02x", Data_packet_6[6]);
    if (Data_len_6 > 0) {
        printf("Received %d bytes Data: ", Data_len_6);
        for (int i = 0; i < Data_len_6; i++) {
            buffer_data_fingerprint[5][i] = Data_packet_6[i];
            printf("%02X ", Data_packet_6[i]);
        }
        printf("\n");
    }


    for(int i = 0; i < 6; i++){
        ESP_LOGI(TAG, "Buffer_data_fingerprint[%d][]: ", i);
        for(int x = 0; x < 139; x++){
            printf("%02x ", buffer_data_fingerprint[i][x]);
        }
        printf("\n");
    }

    
}


void PS_DownChar(uint8_t buffer_id, uint8_t buffer_data_fingerprint[6][139]){
    uint16_t checksum = 0x01 + 0x04 + 0x09 + buffer_id;
    uint8_t command[] = {
        0xEF, 0x01,                                      // Header (2 bytes)
        0xFF, 0xFF, 0xFF, 0xFF,                          // Address (4 bytes)
        0x01,                                            // Packet Identifier (1 bytes)
        0x00, 0x04,                                      // Length (2 bytes)
        0x09,                                            // Intruction Code (1 bytes)
        buffer_id,                                       // CharBuffer được chọn để tải xuống
        ((checksum >> 8) & 0xFF), (checksum & 0xFF)      // Checksum (2 bytes)
    };              
    uart_write_bytes(UART_NUM, (const char*) command, sizeof(command));

    uint8_t ACK_packet[12];
    //uart_read_bytes(UART_NUM, ACK_packet, sizeof(ACK_packet), 1000 / portTICK_PERIOD_MS);
    int ACK_len = uart_read_bytes(UART_NUM, ACK_packet, sizeof(ACK_packet), 1000 / portTICK_PERIOD_MS);
    ESP_LOGW(TAG, "PS_DOWNCHAR_ACK: response: %02x", ACK_packet[9]);
    if (ACK_len > 0) {
        printf("Received %d bytes ACK: ", ACK_len);
        for (int i = 0; i < ACK_len; i++) {
            printf("%02X ", ACK_packet[i]);
        }
        printf("\n");
    }

    for(int num_packet = 0; num_packet < 6; num_packet++){
        uint8_t small_packet_data[139] = {0};
        ESP_LOGI(TAG, "PS_DOWNCHAR: Send packet_data %d in sensor: ", num_packet);
        for(int x = 0; x < sizeof(small_packet_data); x++){
            small_packet_data[x] = buffer_data_fingerprint[num_packet][x];
            printf("%02x ", small_packet_data[x]);
        }
        printf(" \n");
        uart_write_bytes(UART_NUM, (const char*) small_packet_data, sizeof(small_packet_data));
    }

    uint8_t ACK_packet_2[12];
    //uart_read_bytes(UART_NUM, ACK_packet, sizeof(ACK_packet), 1000 / portTICK_PERIOD_MS);
    int ACK_len_2 = uart_read_bytes(UART_NUM, ACK_packet_2, sizeof(ACK_packet_2), 1000 / portTICK_PERIOD_MS);
    ESP_LOGW(TAG, "PS_DOWNCHAR_ACK_2: response: %02x", ACK_packet_2[9]);
    if (ACK_len_2 > 0) {
        printf("Received %d bytes ACK_2: ", ACK_len_2);
        for (int i = 0; i < ACK_len_2; i++) {
            printf("%02X ", ACK_packet_2[i]);
        }
        printf("\n");
    }
}



// Hàm PS_Enroll: Đăng ký vân tay
void PS_Enroll(uint16_t page_id) {
    uint8_t status;
    ESP_LOGW(TAG, "Place your finger on the sensor.");
    
    // Lần 1: Lấy ảnh và tạo đặc trưng
    do {
        status = PS_GetImage();
    } while (status != 0x00); // Chờ lấy ảnh thành công

    status = PS_GenChar(1); // Tạo đặc trưng ở buffer 1
    if (status != 0x00) {
        ESP_LOGE(TAG, "PS_GENCHAR_1: Failed to generate character from image. Error: %d\n", status);
        return;
    }

    ESP_LOGW(TAG, "Remove your finger and place it again.\n");
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Chờ người dùng bỏ tay ra

    // Lần 2: Lấy ảnh và tạo đặc trưng lần 2
    do {
        status = PS_GetImage();
    } while (status != 0x00); // Chờ lấy ảnh thành công

    status = PS_GenChar(2); // Tạo đặc trưng ở buffer 2
    if (status != 0x00) {
        ESP_LOGE(TAG, "PS_GENCHAR_2: Failed to generate character from image. Error: %d\n", status);
        return;
    }

    // Kết hợp hai đặc trưng thành mẫu vân tay
    status = PS_RegModel();
    if (status != 0x00) {
        ESP_LOGE(TAG, " PS_REGMODEL: Failed to combine fingerprint templates. Error: %d\n", status);
        return;
    }

    // Lưu mẫu vân tay vào bộ nhớ
    status = PS_Store(1, page_id); // Lưu mẫu ở buffer 1 vào vị trí page_id
    if (status == 0x00) {
        ESP_LOGI(TAG, "PS_STORE: Fingerprint enrolled successfully at position %d.\n", page_id);
    } else {
        ESP_LOGE(TAG, "PS_STORE: Failed to store fingerprint. Error: %d\n", status);
    }
}


uint8_t PS_Search(uint8_t buffer_id, uint16_t start_page, uint16_t page_num) {
    // uint8_t data[4];
    // data[0] = (start_page >> 8) & 0xFF;
    // data[1] = start_page & 0xFF;
    // data[2] = (page_num >> 8) & 0xFF;
    // data[3] = page_num & 0xFF;
    // send_command(0x04, data, 4);
    uint8_t high_byte_start_page = (start_page >> 8) & 0xFF; 
    uint8_t low_byte_start_page = start_page & 0xFF ;
    uint8_t high_byte_page_num = (page_num >> 8) & 0xFF; 
    uint8_t low_byte_page_num = page_num & 0xFF ;
    uint16_t checksum = 0x01 + 0x08 + 0x04 + buffer_id + start_page + page_num;
    uint8_t command[] = {
        0xEF, 0x01,                                           // Header (2 bytes)
        0xFF, 0xFF, 0xFF, 0xFF,                               // Address (4 bytes)
        0x01,                                                 // Packet Identifier (1 bytes)
        0x00, 0x08,                                           // Length (2 bytes)
        0x04,                                                 // Intruction Code (1 bytes)
        buffer_id,                                            // CharBuffer được chọn để tìm dữ liệu (1 or 2 ) (1 bytes)
        high_byte_start_page, low_byte_start_page,            // Page bắt đầu tìm  (2 bytes)
        high_byte_page_num, low_byte_page_num,                // Số lượng page  (2 bytes )
        (checksum >> 8) & 0xFF, (checksum & 0xFF)             // Checksum (2 bytes)
    };
    uart_write_bytes(UART_NUM, (const char*) command, sizeof(command));
    // printf("Send %d byte: ", sizeof(command));
    // for (int t = 0; t < sizeof(command); t++){
    //     printf("%02x ", command[t]);
    // }
    // printf("\n");


    uint8_t response[128];
    uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    // int length = uart_read_bytes(UART_NUM, response, sizeof(response), 1000 / portTICK_PERIOD_MS);
    // if (length > 0) {
    //     printf("Received %d bytes: ", length);
    //     for (int i = 0; i < length; i++) {
    //         printf("%02X ", response[i]);
    //     }
    //     printf("\n");
    // }

    //ESP_LOGW(TAG, "PS_SEARCH: response: %02x", response[9]);
    if (response[9] == 0x00) {
        //ESP_LOGI(TAG, "PS_SEARCH: Fingerprint found at position: %d\n", (response[10] << 8) | response[11]);
        return ((response[10] << 8) | response[11]);
    } else {
        ESP_LOGI(TAG, "PS_SEARCH: Fingerprint not found.\n");
        return 0;
    }
}


void PS_Delete(uint16_t page_id, uint16_t num) {
    // uint8_t data[4];
    // data[0] = (page_id >> 8) & 0xFF;
    // data[1] = page_id & 0xFF;
    // data[2] = (num >> 8) & 0xFF;
    // data[3] = num & 0xFF;
    
    // send_command(0x0C, data, 4);
    uint16_t checksum = 0x01 + 0x07 + 0x0C + page_id + num;
    uint8_t command[] = {
        0xEF, 0x01,                                         // Header (2 bytes)
        0xFF, 0xFF, 0xFF, 0xFF,                             // Address (4 bytes)
        0x01,                                               // Packet Identifier (1 bytes)
        0x00, 0x07,                                         // Length (2 bytes)
        0x0C,                                               // Intruction Code (1 bytes)
        ((page_id >> 8) & 0xFF), (page_id & 0xFF),          // PageID bắt đầu muốn xóa (2 bytes)
        ((num >> 8) & 0xFF), (num & 0xFF),                  // Số lượng PageID xóa dần từ PageID bắt đầu (2 bytes)
        ((checksum >> 8) & 0xFF), (checksum & 0xFF)         // Checksum (2 bytes)
    };
                     // | Header   | Address               | PI  | Length    | Cmmd| page_id                                  | num                              | Checksum                                   |
    uart_write_bytes(UART_NUM, (const char*) command, sizeof(command));

    uint8_t ACK_packet[12];
    int ACK_len = uart_read_bytes(UART_NUM, ACK_packet, sizeof(ACK_packet), 1000 / portTICK_PERIOD_MS);
    ESP_LOGW(TAG, "PS_DELETE_ACK: response: %02x", ACK_packet[9]);
    if (ACK_len > 0) {
        printf("Received %d bytes ACK: ", ACK_len);
        for (int i = 0; i < ACK_len; i++) {
            printf("%02X ", ACK_packet[i]);
        }
        printf("\n");
    }

    if (ACK_packet[9] == 0x00) {
        printf("Fingerprint deleted successfully.\n");
    } else {
        printf("Failed to delete fingerprint.\n");
    }
}


