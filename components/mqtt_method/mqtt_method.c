#include "mqtt_method.h"

#define BROKER_URI "mqtt://broker.hivemq.com:1883" // IP máy chạy MQTT Broker
#define COMMAND_TOPIC "esp32/command"
#define RESPONSE_TOPIC "esp32/response"

void func8 (void) {

}


esp_mqtt_client_handle_t mqtt_client;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            printf("Connected to MQTT Broker\n");
            esp_mqtt_client_subscribe(mqtt_client, COMMAND_TOPIC, 0);
            break;

        case MQTT_EVENT_DATA:
            printf("Received data: %.*s from topic: %.*s\n",
                   event->data_len, event->data,
                   event->topic_len, event->topic);

            // Giả lập thực hiện nhiệm vụ
            if (strncmp(event->data, "{\"action\":\"start_task\"}", event->data_len) == 0) {
                printf("Task started...\n");
                vTaskDelay(2000 / portTICK_PERIOD_MS); // Giả lập nhiệm vụ mất 2 giây
                esp_mqtt_client_publish(mqtt_client, RESPONSE_TOPIC, "Task completed", 0, 0, 0);
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