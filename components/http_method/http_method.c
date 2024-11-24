#include "http_method.h"

#define TAG "HTTP_CLIENT"




void process_json_data(const char *json_data) {
    cJSON *json_array = cJSON_Parse(json_data);
    if (json_array == NULL || !cJSON_IsArray(json_array)) {
        ESP_LOGE(TAG, "Failed to parse JSON or not an array");
        return;
    }

    // Lặp qua từng đối tượng trong mảng JSON
    int array_size = cJSON_GetArraySize(json_array);
    for (int i = 0; i < array_size; i++) {
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
        Student student;
        student.id = id->valueint;
        strncpy(student.full_name, full_name->valuestring, sizeof(student.full_name) - 1);
        strncpy(student.student_id, student_id->valuestring, sizeof(student.student_id) - 1);
        strncpy(student.position, position->valuestring, sizeof(student.position) - 1);
        strncpy(student.email, email->valuestring, sizeof(student.email) - 1);
        strncpy(student.password, password->valuestring, sizeof(student.password) - 1);
        student.pass_en = pass_en->valueint;
        student.fing_en = fing_en->valueint;

        // Lưu vào NVS
        nvs_handle_t nvs_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
        if (err == ESP_OK) {
            char key[20];
            snprintf(key, sizeof(key), "student_%d", student.id);
            char value[200];
            snprintf(value, sizeof(value), "%s|%s|%s|%s|%s|%d|%d", student.full_name, student.student_id, student.position, student.email, student.password, student.pass_en, student.fing_en);

            nvs_set_str(nvs_handle, key, value); // Lưu thông tin sinh viên
            nvs_commit(nvs_handle); // Ghi vào flash
            nvs_close(nvs_handle);

            ESP_LOGI(TAG, "Stored student: %s", student.full_name);
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

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    
}


void create_http_task(void) {
    xTaskCreate(http_get_task, "HTTP task", 8192, NULL, 5, NULL);
}


void func6 (void) {

}