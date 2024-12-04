#include "wifi_config.h"





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

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t sta_cfg = {
        .sta = {
            .ssid = "BaPhuQuy",                    //BaPhuQuy           TP-LINK_4550AA      Ds Place guest          QT3112
            .password = "BaTrpTNMT62",                        //BaTrpTNMT62            61924666        88888888      12345678
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_ERROR_CHECK(esp_wifi_connect());

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