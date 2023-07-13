#include "mqtt_client.h"
#include "esp_err.h"
#include "esp_log.h"

#include "mqtt.h"
#include "status.h"
#include "mqtthandler.h"
#include "mac.h"

static const char* TAG = "MQTT_CLIENT";
static esp_mqtt_client_handle_t client;

static char schedules_topic[50];
static char app_topic[20];

static bool mqtt_connected = false;

void mqtt_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    ESP_LOGI(TAG, "Event dispached from event loop base=%s, event_id=%d", event_base, (int)event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    // char topic[120];
    
    switch((esp_mqtt_event_id_t)event_id) {
        
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Client connected");
            mqtt_connected = true;
            ESP_LOGI("SUBSCRIBE", "%s", app_topic);
            esp_mqtt_client_subscribe(client, schedules_topic, 1);
            // ESP_LOGI("TOPIC SUBSCRIBE", "%d", result);
            esp_mqtt_client_subscribe(client, app_topic, 1);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT client disconnected");
            mqtt_connected = false;
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            // ESP_LOGI(TAG, "Subscribed to topic: %s", event->topic);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Data received from topic: %s", event->topic);
            
            if (!strncmp(schedules_topic, event->topic, event->topic_len)) {
                schedule_save_handler(event->data);
            }
            
            else if (!strncmp(app_topic, event->topic, event->topic_len)) {
                remote_open(event->data);
            }

            break;
        
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            ESP_LOGE(TAG, "Last errno string: %s", strerror(event->error_handle->esp_transport_sock_errno));
            break;

        default:
            ESP_LOGI(TAG, "Other event id: %d", event->event_id);
            ESP_LOGI(TAG, "Error: %s", strerror(event->error_handle->esp_transport_sock_errno));
            break;
    }
}

void mqtt_app_start() {

    memset(schedules_topic, 0, 50);
    memset(app_topic, 0, 20);

    ESP_LOGI("TOPIC", "%s", get_user_hash(NULL));
    
    strcpy(schedules_topic, "schedules/");
    strcpy(&schedules_topic[10], get_user_hash(NULL));

    strcpy(app_topic, "feeder/");
    strcpy(&app_topic[7], get_mac_address());

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker =  {
            .address = {
                .uri = "mqtt://test.mosquitto.org:1883",
                // .port = 1883,
                // .
            }
        },
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    // esp_mqtt_client_subscribe
}

int mqtt_app_publish(char* topic, char* message, int qos) {
    int msg_id = -1;
    msg_id = esp_mqtt_client_publish(client, topic, message, 0, qos, 0);
    ESP_LOGI(TAG, "Message sent to topic: %s", topic);
    return msg_id;
}

void mqtt_app_close() {
    mqtt_connected = false;
    esp_mqtt_client_stop(client);
    esp_mqtt_client_destroy(client);
}

bool is_mqtt_connected() {
    return mqtt_connected;
}