#include "mqtt_method.h"

#define BROKER_URI "mqtt://broker.hivemq.com:1883" // IP máy chạy MQTT Broker
#define COMMAND_TOPIC "esp32/command"
#define OPENDOOR_TOPIC "esp32/command/opendoor"
#define REGISTER_FINGERPRINT_TOPIC "esp32/command/registerfingerprint"
#define RESPONSE_TOPIC "esp32/response"

static const char* TAG = "mqtt";

void func8 (void) {

}


esp_mqtt_client_handle_t mqtt_client;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            printf("Connected to MQTT Broker\n");
            esp_mqtt_client_subscribe(mqtt_client, COMMAND_TOPIC, 0);
            esp_mqtt_client_subscribe(mqtt_client, OPENDOOR_TOPIC, 0);
            esp_mqtt_client_subscribe(mqtt_client, REGISTER_FINGERPRINT_TOPIC, 0);
            break;

        case MQTT_EVENT_DATA:
            printf("Received data: %.*s from topic: %.*s\n",
                   event->data_len, event->data,
                   event->topic_len, event->topic);


            if (strncmp(event->topic, OPENDOOR_TOPIC, event->topic_len) == 0) {

                printf("Task open door started...\n");
                open_door();
                vTaskDelay(1000/portTICK_PERIOD_MS);
                esp_mqtt_client_publish(mqtt_client, RESPONSE_TOPIC, "Task completed", 0, 0, 0);

            } else if (strncmp(event->topic, REGISTER_FINGERPRINT_TOPIC, event->topic_len) == 0) {
                // printf("Task register fingerprint started...\n");
                // open_door();
                // vTaskDelay(1000/portTICK_PERIOD_MS);
                // esp_mqtt_client_publish(mqtt_client, RESPONSE_TOPIC, "Task completed", 0, 0, 0);

                char payload[100];
                char MSSV_mqtt[20];
                strncpy(payload, event->data, event->data_len);
                payload[event->data_len] = '\0';
                // Parse JSON data
                cJSON *json = cJSON_Parse(payload);
                cJSON *mssv = cJSON_GetObjectItem(json, "mssv");
                strncpy(MSSV_mqtt, mssv->valuestring, sizeof(MSSV_mqtt) - 1);
                for(int i = 0; i <= MAX_STUDENTS; i++) {
                    if((strcmp(MSSV_mqtt, students[i].student_id) == 0) && (students[i].id != 0)) {
                        PS_Enroll(students[i].fingerprint);
                        ESP_LOGI(TAG, "Đăng ký vân tay thành công !!");
                        esp_mqtt_client_publish(mqtt_client, RESPONSE_TOPIC, "Task completed", 0, 0, 0);
                        memset(MSSV_mqtt, 0, sizeof(MSSV_mqtt));
                        vTaskDelay(1000/portTICK_PERIOD_MS);
                        break;
                    }
                }

            }
            
            break;

        default:
            break;
    }
}


void mqtt_app_start(){
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = BROKER_URI,
            }
        }
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}