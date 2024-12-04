#include "http_method.h"

#define TAG "HTTP_CLIENT"





void process_json_data(const char *json_data) {
    cJSON *json_array = cJSON_Parse(json_data);
    if (json_array == NULL || !cJSON_IsArray(json_array)) {
        ESP_LOGE(TAG, "Failed to parse JSON or not an array");
        return;
    }

    int student_count = 0;

    // Lặp qua từng đối tượng trong mảng JSON
    int array_size = cJSON_GetArraySize(json_array);
    for (int i = 0; i < array_size && student_count < MAX_STUDENTS; i++) {
        cJSON *item = cJSON_GetArrayItem(json_array, i);

        // Lấy từng trường dữ liệu
        cJSON *id = cJSON_GetObjectItem(item, "id");
        cJSON *full_name = cJSON_GetObjectItem(item, "full_name");
        cJSON *student_id = cJSON_GetObjectItem(item, "student_id");
        cJSON *position = cJSON_GetObjectItem(item, "position");
        cJSON *email = cJSON_GetObjectItem(item, "email");
        cJSON *password = cJSON_GetObjectItem(item, "password");
        cJSON *pass_en = cJSON_GetObjectItem(item, "pass_en");
        cJSON *fing_en = cJSON_GetObjectItem(item, "fing_en");

        if (!id || !full_name || !student_id || !position || !email || !password || !pass_en || !fing_en) {
            ESP_LOGE(TAG, "Missing fields in JSON object");
            continue;
        }

        // Tạo một đối tượng sinh viên
        students[student_count].id = id->valueint;
        strncpy(students[student_count].full_name, full_name->valuestring, sizeof(students[student_count].full_name) - 1);
        strncpy(students[student_count].student_id, student_id->valuestring, sizeof(students[student_count].student_id) - 1);
        strncpy(students[student_count].position, position->valuestring, sizeof(students[student_count].position) - 1);
        strncpy(students[student_count].email, email->valuestring, sizeof(students[student_count].email) - 1);
        strncpy(students[student_count].password, password->valuestring, sizeof(students[student_count].password) - 1);
        students[student_count].pass_en = pass_en->valueint;
        students[student_count].fing_en = fing_en->valueint;
        

        student_count++;

        // Lưu vào NVS
        nvs_handle_t nvs_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
        if (err == ESP_OK) {
            char key[20];
            snprintf(key, sizeof(key), "student_%d", students[student_count].id);
            char value[200];
            snprintf(value, sizeof(value), "%s|%s|%s|%s|%s|%d|%d",  students[student_count].full_name, 
                                                                    students[student_count].student_id, 
                                                                    students[student_count].position, 
                                                                    students[student_count].email, 
                                                                    students[student_count].password, 
                                                                    students[student_count].pass_en, 
                                                                    students[student_count].fing_en);

            nvs_set_str(nvs_handle, key, value); // Lưu thông tin sinh viên
            nvs_commit(nvs_handle); // Ghi vào flash
            nvs_close(nvs_handle);

            ESP_LOGI(TAG, "Stored student: %s", students[student_count].full_name);
        } else {
            ESP_LOGE(TAG, "Failed to open NVS");
        }
    }

    
    // Giải phóng bộ nhớ JSON
    cJSON_Delete(json_array);
}






// Xử lý sự kiện HTTP
esp_err_t http_event_handler(esp_http_client_event_t *evt) {

    static char *buffer = NULL;
    static int buffer_len = 0;

    switch (evt->event_id)
    {
        case HTTP_EVENT_ON_DATA:
            //printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);

            if (!esp_http_client_is_chunked_response(evt->client)) {
                buffer = realloc(buffer, buffer_len + evt->data_len + 1);
                memcpy(buffer + buffer_len, evt->data, evt->data_len);
                buffer_len += evt->data_len;
                buffer[buffer_len] = 0; // Kết thúc chuỗi
            }

        break;

        case HTTP_EVENT_ON_FINISH:
            if (buffer) {
                ESP_LOGI(TAG, "HTTP Response: %s", buffer);
                process_json_data(buffer); // Xử lý JSON
                free(buffer);
                buffer = NULL;
                buffer_len = 0;
            }
        break;

        default:
        break;
    }
    return ESP_OK;
}




void http_get_task(void *pvParameters) {
    
    
    while (1){
        // Chờ cho đến khi kết nối Wi-Fi thành công
        while (!is_wifi_connected()) {
            ESP_LOGI(TAG, "Chưa kết nối Wi-Fi, đang chờ...");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Chờ 1 giây
        }

        esp_http_client_config_t config_get = {
            .url = "http://192.168.2.15:3000/api/get-database",
            .method = HTTP_METHOD_GET,
            .buffer_size = 4096,       // Kích thước bộ đệm nhận
            .buffer_size_tx = 2048,    // Kích thước bộ đệm gửi (nếu POST, PUT)
            .cert_pem = NULL,
            .event_handler = http_event_handler
        };

        esp_http_client_handle_t client = esp_http_client_init(&config_get);
        esp_http_client_perform(client);
        esp_http_client_cleanup(client);


        // ESP_LOGI(TAG, "STUDENT_LIST");
        // for (int i = 0; i < 5; i++) {
        //     ESP_LOGI("STUDENT", "ID: %d, Full Name: %s, Student ID: %s, Position: %s, Email: %s, Password: %s, Pass Enabled: %d, Fingerprint Enabled: %d",
        //             students[i].id, students[i].full_name, students[i].student_id, students[i].position,
        //             students[i].email, students[i].password, students[i].pass_en, students[i].fing_en);
        // }

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    
}


void create_http_task(void) {
    xTaskCreate(http_get_task, "HTTP task", 8192, NULL, 5, NULL);
}





void send_login_request(int student_x) {
    char post_data[256];
    snprintf(post_data, sizeof(post_data), "{\"full_name\":\"%s\",\"student_id\":\"%s\",\"position\":\"%s\"}",
                                             students[student_x].full_name, 
                                             students[student_x].student_id,
                                             students[student_x].position);

    esp_http_client_config_t config = {
        .url = "http://192.168.2.15:3000/api/get-esp32",
        .cert_pem = NULL,
        .method = HTTP_METHOD_POST,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data)); 

    // esp_err_t err = esp_http_client_perform(client);
    // if (err == ESP_OK) {
    //     int content_length = esp_http_client_get_content_length(client);
    //     char *response_buffer = malloc(content_length + 1);

    //     if (response_buffer) {
    //         int data_read = esp_http_client_read(client, response_buffer, content_length);
    //         if (data_read > 0) {
    //             response_buffer[data_read] = '\0';
    //             ESP_LOGI(TAG, "Server Response: %s", response_buffer);

    //             // Kiểm tra kết quả
    //             if (strstr(response_buffer, "\"status\":\"success\"")) {
    //                 ESP_LOGI(TAG, "Login successful!");
    //             } else {
    //                 ESP_LOGI(TAG, "Login failed!");
    //             }
    //         }
    //         free(response_buffer);
    //     } else {
    //         ESP_LOGE(TAG, "Failed to allocate memory for response buffer");
    //     }
    // } else {
    //     ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    // }

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}








// void send_infor_register_fingerprint(int student_x) {
//     char post_data[256];
//     snprintf(post_data, sizeof(post_data), "{\"full_name\":\"%s\",\"student_id\":\"%s\",\"position\":\"%s\"}",
//                                              students[student_x].full_name, 
//                                              students[student_x].student_id,
//                                              students[student_x].position);

//     esp_http_client_config_t config = {
//         .url = "http://192.168.2.15:3000/api/get-esp32",
//         .cert_pem = NULL,
//         .method = HTTP_METHOD_POST,
//     };

//     esp_http_client_handle_t client = esp_http_client_init(&config);

//     esp_http_client_set_header(client, "Content-Type", "application/json");
//     esp_http_client_set_post_field(client, post_data, strlen(post_data)); 

//     esp_http_client_perform(client);
//     esp_http_client_cleanup(client);
// }









void fetch_access_time(const char *student_id) {
    char url[256];
    snprintf(url, sizeof(url), "http://192.168.2.15:3000/api/getAccessTime?student_id=%s", student_id);

    esp_http_client_config_t config = {
        .url = url,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int content_length = esp_http_client_get_content_length(client);
        char *buffer = malloc(content_length + 1);
        if (buffer) {
            esp_http_client_read(client, buffer, content_length);
            buffer[content_length] = '\0';
            printf("Access time data: %s\n", buffer);
            free(buffer);
        }
    } else {
        printf("HTTP request failed: %s\n", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void func6 (void) {

}