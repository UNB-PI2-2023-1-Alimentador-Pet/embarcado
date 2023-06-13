#include "mqtthandler.h"
#include "cJSON.h"
#include "esp_err.h"
#include "string.h"
#include "stdbool.h"
#include "mqtt.h"

#define SERVER_SENDER_ATTR "server"
#define SENDER_ESP_ATTR "esp32"

bool verify_equal(const char* actual, const char* expected) {
    return !strcmp(actual, expected);
}

bool verify_sender_and_userhash(cJSON* obj, const char* expected_sender) {
    cJSON* sender_obj = cJSON_GetObjectItem(obj, "sender");
    cJSON* userhash_obj = cJSON_GetObjectItem(obj, "user_hash");
    
    char* sender = cJSON_GetStringValue(sender_obj);
    char* userhash = cJSON_GetStringValue(userhash_obj);

// TODO implement get_user_hash
    if (verify_equal(sender, expected_sender) && verify_equal(userhash, get_user_hash())) {
        return true;
    }
    return false;
}

void provide_food_handler(const char* json) {
    cJSON* obj = cJSON_Parse(json);   

    if (!verify_sender_and_userhash(obj, SERVER_SENDER_ATTR)) return;

    cJSON* qtde_obj = cJSON_GetObjectItem(obj, "qtde");
    int qtde = cJSON_GetNumberValue(qtde_obj);

    // TODO call function to actually provide food

    cJSON_Delete(obj);
}

void close_feeder_handler(const char* json) {
    cJSON* obj = cJSON_Parse(json);

    if (!verify_sender_and_userhash(obj, SERVER_SENDER_ATTR)) return;

    cJSON* value_obj = cJSON_GetObjectItem(obj, "value");
    int value = cJSON_GetNumberValue(value_obj);

    // TODO call funcion to actually close the feeder

    cJSON_Delete(obj);
}

void schedule_save_handler(const char* json) {
    cJSON* obj = cJSON_Parse(json);

    if (!verify_sender_and_userhash(obj, SERVER_SENDER_ATTR)) return;

    cJSON* schedule_array = cJSON_GetObjectItem(obj, "schedule");
    int schedule_array_size = cJSON_GetArraySize(schedule_array);

    for (int i = 0; i < schedule_array_size; i++) {
        cJSON* item = cJSON_GetArrayItem(schedule_array, i);
        cJSON* qtde_obj = cJSON_GetObjectItem(item, "qtde");
        cJSON* timestamp_obj = cJSON_GetObjectItem(item, "timestamp");

        int qtde = cJSON_GetNumberValue(qtde_obj);
        int timestamp = cJSON_GetNumberValue(timestamp_obj);

        // TODO
    }

    cJSON_Delete(obj);
}

void send_tank_level(int value) {
    cJSON* obj = cJSON_CreateObject();

    cJSON_AddStringToObject(obj, "sender", SENDER_ESP_ATTR);
    cJSON_AddStringToObject(obj, "user_hash", get_user_hash());
    cJSON_AddNumberToObject(obj, "value", value);

    char* string = cJSON_Print(obj);

// TODO change prefix to correct name
    mqtt_app_publish("prefix/tank_level", string, 1);

    cJSON_Delete(obj);
}