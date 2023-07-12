#include "mqtt_client.h"
#include "esp_err.h"
#include "esp_log.h"

#include "mqtt.h"
#include "status.h"
#include "mqtthandler.h"

static const char* TAG = "MQTT_CLIENT";
static esp_mqtt_client_handle_t client;

void mqtt_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    ESP_LOGI(TAG, "Event dispached from event loop base=%s, event_id=%d", event_base, (int)event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    char topic[120];
    
    switch((esp_mqtt_event_id_t)event_id) {
        
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Client connected");

            // char schedules_topic[40];
            // strcpy(schedules_topic, "schedules/");
            // strcpy(&schedules_topic[10], get_user_hash(NULL));

            // ESP_LOGI("SCHEDULE TOPIC", "%s", schedules_topic);

            // esp_mqtt_client_subscribe(client, schedules_topic, 1);

            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT client disconnected");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Data received from topic: %s", event->topic);
            // TODO ALL
            // handle_mqtt_data();
            break;
        
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            ESP_LOGE(TAG, "Last errno string: %s", strerror(event->error_handle->esp_transport_sock_errno));
            break;

        default:
            ESP_LOGI(TAG, "Other event id: %d", event->event_id);
            break;
    }
}

void mqtt_app_start() {

    // TODO set broker config
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker =  {
            .address = {
                .uri = "mqtt://test.mosquitto.org",
                .port = 1883
            }
        }
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    // esp_mqtt_client_subscribe
}

int mqtt_app_publish(char* topic, char* message, int qos) {
    int msg_id = -1;
    msg_id = esp_mqtt_client_publish(client, topic, message, 0, qos, 0);
    ESP_LOGI(TAG, "Message sent to topic: %s", topic);
    return msg_id;
}