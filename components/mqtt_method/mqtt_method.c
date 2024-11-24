#include "mqtt_method.h"

void func5 (void){
    
}

// /*=============================================================================================================*/
// //{clientId:"3112",userName:"esp32",password:"Toan"}
// esp_mqtt_client_config_t mqtt_cfg = {
//     .broker = {
//         .address = {
//             .hostname = "demo.thingsboard.io",    // Set hostname only
//             .port = 1883,                         // Default MQTT port
//             .transport = MQTT_TRANSPORT_OVER_TCP,      // Set transport type, or remove this line
//             //.uri = "mqtt://broker.hivemq.com:1883",
//         },
//         .verification = {
//             .use_global_ca_store = false,
//         },
//     },
//     .credentials = {
//         .username = "esp32",
//         .client_id = "9073d5b0-9b89-11ef-af67-a38a7671daf5",
//         .set_null_client_id = false,
//         .authentication = {
//             .password = "Toan",
//         },
//     },
//     .session = {
//         .keepalive = 120,
//         .disable_clean_session = false,
//     },
//     .network = {
//         .reconnect_timeout_ms = 10000,
//         .timeout_ms = 10000,
//         .disable_auto_reconnect = false,
//     },
//     .task = {
//         .priority = 5,
//         .stack_size = 4096,
//     }, 
//     .buffer = {
//         .size = 1024,
//     },
//     .outbox = {
//         .limit = 2048,
//     },
// };


// /*=========================================================================================================*/
// static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
//     esp_mqtt_event_handle_t event = event_data;
//     esp_mqtt_client_handle_t client = event->client;

//     switch (event->event_id) {

//         case MQTT_EVENT_CONNECTED:
//             ESP_LOGI("MQTT", "MQTT_EVENT_CONNECTED");
//             esp_mqtt_client_subscribe(event->client, "esp32/data", 0);
//             ESP_LOGI("MQTT", "Subscribed to RPC requests");
//             break;

//         case MQTT_EVENT_DATA:
//             ESP_LOGI("MQTT", "MQTT_EVENT_DATA");
//             printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
//             printf("DATA=%.*s\r\n", event->data_len, event->data);
            
//             // Kiểm tra method cho RELAY
//             if (strstr(event->data, "\"method\":\"getValue\"") != NULL) {
//                 if (strstr(event->data, "\"params\":true") != NULL) {
//                     gpio_set_level(RELAY, 1);  // Bật LED1
//                 } else if (strstr(event->data, "\"params\":false") != NULL) {
//                     gpio_set_level(RELAY, 0);  // Tắt LED1
//                 }
//             }
//             // Kiểm tra method cho USER1
//             if (strstr(event->data, "\"method\":\"setUSER1\"") != NULL) {
//                 if (strstr(event->data, "\"params\":true") != NULL) {
//                     //gpio_set_level(GPIO_PIN, 1);  // Bật LED1
//                     USER[1].en = 1;
//                 } else if (strstr(event->data, "\"params\":false") != NULL) {
//                     //gpio_set_level(GPIO_PIN, 0);  // Tắt LED1
//                     USER[1].en = 0;
//                 }
//             }
//             // Kiểm tra method cho USER2
//             else if (strstr(event->data, "\"method\":\"setUSER2\"") != NULL) {
//                 if (strstr(event->data, "\"params\":true") != NULL) {
//                     //gpio_set_level(GPIO_PIN, 1);  // Bật LED2
//                     USER[2].en = 1;
//                 } else if (strstr(event->data, "\"params\":false") != NULL) {
//                     //gpio_set_level(GPIO_PIN, 0);  // Tắt LED2
//                     USER[2].en = 0;
//                 }
//             }

//             break;
            
//         default:
//             ESP_LOGI("MQTT", "Event ID: %d", event->event_id);
//             break;
//     }
// }


// void mqtt_app_start(){
    
//     esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
//     esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
//     esp_mqtt_client_start(client);
// }






#define MQTT_BROKER_URI "mqtt://broker.hivemq.com:1883"
#define MQTT_TOPIC "esp32/data"

TaskHandle_t mqtt_task_t = NULL;

static const char *MQTT_TAG = "MQTT_CLIENT";

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(MQTT_TAG, "MQTT kết nối thành công");
            esp_mqtt_client_subscribe(event->client, MQTT_TOPIC, 0);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(MQTT_TAG, "Nhận dữ liệu từ MQTT: %.*s", event->data_len, event->data);
            // Xử lý dữ liệu (JSON)
            break;

        default:
            break;
    }
}

void mqtt_app_start(){
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                // .hostname = "broker.hivemq.com",
                // .port = 1883,
                .uri = MQTT_BROKER_URI,
            },
        },
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}


// void mqtt_task(void *pvParameters) {

//     esp_mqtt_client_config_t mqtt_cfg = {
//         .broker = {
//             .address = {
//                 .hostname = "broker.hivemq.com",
//                 .port = 1883,
//                 //.uri = MQTT_BROKER_URI,
//             },
//         },
//     };

//     while(1) {
//         esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
//         esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
//         esp_mqtt_client_start(client);

//         vTaskDelay(100 / portTICK_PERIOD_MS); 
//     }
//     vTaskDelete(NULL);
// }

//  void create_mqtt_task (void) {
//     xTaskCreate(&mqtt_task, "mqtt_task", 4096, NULL, 5, &mqtt_task_t);
//  }