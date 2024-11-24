#include "http_method.h"

#define TAG "HTTP_CLIENT"



// Xử lý sự kiện HTTP
esp_err_t http_event_handler(esp_http_client_event_t *evt) {

    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}




void http_get_task(void *pvParameters) {
    
    
    while (1){

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