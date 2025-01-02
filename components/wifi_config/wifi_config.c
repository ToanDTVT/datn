#include "wifi_config.h"


static const char *TAG = "WIFI";


// Định nghĩa biến để kiểm tra trạng thái kết nối
static bool wifi_connected = false;

// Hàm xử lý sự kiện Wi-Fi
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi started, attempting to connect...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_connected = false;
        ESP_LOGW(TAG, "Wi-Fi disconnected, retrying...");
        esp_wifi_connect(); // Tự động kết nối lại
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        wifi_connected = true;
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}




bool is_wifi_connected() {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        return true;
    }
    return false;
}





void task_wifi_init(){

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.nvs_enable = true;
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Xóa và khởi tạo lại NVS nếu gặp lỗi thiếu trang hoặc phiên bản NVS cũ
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t sta_cfg = {
        .sta = {
            .ssid = "QT3112",                               //BaPhuQuy           TP-LINK_4550AA      Ds Place guest          QT3112
            .password = "12345678",                        //BaTrpTNMT62            61924666        88888888      12345678
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    // vTaskDelay(pdMS_TO_TICKS(1000));
    // ESP_ERROR_CHECK(esp_wifi_connect());

}




void initialize_sntp() {
    printf("Initializing SNTP...\n");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

void obtain_time() {
    initialize_sntp();

    // Đợi thời gian được đồng bộ
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (timeinfo.tm_year < (2024 - 1900) && ++retry < retry_count) {
        printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (timeinfo.tm_year > (2024 - 1900)) {
        printf("Time synchronized: %s", asctime(&timeinfo));
    } else {
        printf("Failed to synchronize time.\n");
    }
}




bool is_access_allowed(char *allowed_days, char *start_time, char *end_time) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char current_day[10];
    strftime(current_day, sizeof(current_day), "%A", &timeinfo);

    if (strstr(allowed_days, current_day)) {
        char current_time[6];
        strftime(current_time, sizeof(current_time), "%H:%M", &timeinfo);

        return strcmp(current_time, start_time) >= 0 && strcmp(current_time, end_time) <= 0;
    }
    return false;
}