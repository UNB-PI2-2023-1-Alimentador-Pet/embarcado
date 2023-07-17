#include "cJSON.h"
#include "esp_log.h"
#include "esp_err.h"
#include "string.h"
#include "stdbool.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "mqtthandler.h"
#include "mqtt.h"
#include "mac.h"
#include "scheduler.h"
#include "funcoes.h"
#include "variaveis_globais.h"
#include "status.h"
#include "time_handle.h"
#include "cJSON.h"

#define SERVER_SENDER_ATTR "server"
#define SENDER_ESP_ATTR "esp32"

static char user_hash[60];
static size_t user_hash_size = 0;

void send_status() {
    cJSON* obj = cJSON_CreateObject();

    cJSON_AddStringToObject(obj, "action", "status");
    cJSON_AddNumberToObject(obj, "reservatory_level", getNivelRacao());
    cJSON_AddBoolToObject(obj, "open", get_bandeja_aberta());
    cJSON_AddStringToObject(obj, "user_hash", get_user_hash(NULL));
    get_time();

    char* data = cJSON_Print(obj);
    char topic[20];
    sprintf(topic, "/feeder/%s", get_mac_address());

    mqtt_app_publish(topic, data, 1);
    cJSON_Delete(obj);
}

void remote_open(char* json) {
    ESP_LOGI("REMOTE OPEN", "DATA: %s", json);
    cJSON* obj = cJSON_Parse(json);
    if (obj == NULL) {
        ESP_LOGI("REMOTE OPEN", "ERROR AT PARSING JSON");
        return;
    }

    cJSON* obj_action = cJSON_GetObjectItem(obj, "action");
    char* action = cJSON_GetStringValue(obj_action);

    if (strcmp(action, "feed")) {
        ESP_LOGI("REMOTE OPEN", "ACTION NOT FEED");
        cJSON_Delete(obj);
        return;
    }

    int quantidade = 0;
    int tempo_bandeja = 0;

    cJSON* obj_quantidade = cJSON_GetObjectItem(obj, "quantidade");
    quantidade = cJSON_GetNumberValue(obj_quantidade);

    cJSON* obj_tempo_bandeja = cJSON_GetObjectItem(obj, "tempoBandeja");
    tempo_bandeja = cJSON_GetNumberValue(obj_tempo_bandeja);

    aciona_fluxo_de_tarefas(tempo_bandeja, quantidade);

    cJSON_Delete(obj);
}

void schedule_save_handler(const char* json) {
    char key[30];
    char value[30];
    struct schedule schedule = scheduler_decode_json(json);
    ESP_LOGI("JSON RECEIVED", "%s", json);
    scheduler_encode(&schedule, key, value);
    scheduler_save(key, value);
}

char* get_user_hash(size_t* hash_size) {

    // strcpy(user_hash, "de14b0b7-e193-495b-891c-0cc5de86ed77");
    // return user_hash;

    if (user_hash_size == 0) {
        nvs_handle_t handle;
        nvs_open("nvs", NVS_READONLY, &handle);

        size_t user_hash_size;
        nvs_get_str(handle, "user_hash", user_hash, &user_hash_size);

        nvs_close(handle);
    }
    if (hash_size != NULL) {
        *hash_size = user_hash_size;
    }
    return user_hash;
}

bool verify_equal(const char* actual, const char* expected) {
    return !strcmp(actual, expected);
}

bool verify_sender_and_userhash(cJSON* obj, const char* expected_sender) {
    cJSON* sender_obj = cJSON_GetObjectItem(obj, "sender");
    cJSON* userhash_obj = cJSON_GetObjectItem(obj, "user_hash");
    
    char* sender = cJSON_GetStringValue(sender_obj);
    char* userhash = cJSON_GetStringValue(userhash_obj);

// TODO implement get_user_hash
    if (verify_equal(sender, expected_sender) && verify_equal(userhash, "somehash")) {
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

// void schedule_save_handler(const char* json) {
//     cJSON* obj = cJSON_Parse(json);

//     if (!verify_sender_and_userhash(obj, SERVER_SENDER_ATTR)) return;

//     cJSON* schedule_array = cJSON_GetObjectItem(obj, "schedule");
//     int schedule_array_size = cJSON_GetArraySize(schedule_array);

//     for (int i = 0; i < schedule_array_size; i++) {
//         cJSON* item = cJSON_GetArrayItem(schedule_array, i);
//         cJSON* qtde_obj = cJSON_GetObjectItem(item, "qtde");
//         cJSON* timestamp_obj = cJSON_GetObjectItem(item, "timestamp");

//         int qtde = cJSON_GetNumberValue(qtde_obj);
//         int timestamp = cJSON_GetNumberValue(timestamp_obj);

//         // TODO
//     }

//     cJSON_Delete(obj);
// }

void send_tank_level(int value) {
    cJSON* obj = cJSON_CreateObject();

    cJSON_AddStringToObject(obj, "sender", SENDER_ESP_ATTR);
    // cJSON_AddStringToObject(obj, "user_hash", get_user_hash());
    cJSON_AddNumberToObject(obj, "value", value);

    char* string = cJSON_Print(obj);

// TODO change prefix to correct name
    mqtt_app_publish("prefix/tank_level", string, 1);

    cJSON_Delete(obj);
}