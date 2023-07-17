#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include <stdbool.h>
#include "cJSON.h"
#include <string.h>

#include "scheduler.h"
#include "time_handle.h"

#define SECOND(h, m, s) (h * 60 + m) * 60 + s

int8_t scheduler_get_week_day(int8_t week_index) {
    switch (week_index) {
        case 0:
            return DOMINGO;
        // case '0':
        //     return DOMINGO;
        case 1:
            return SEGUNDA;
        // case '1':
        //     return SEGUNDA;
        case 2:
            return TERCA;
        // case '2':
        //     return TERCA;
        case 3:
            return QUARTA;
        // case '3':
        //     return QUARTA;
        case 4:
            return QUINTA;
        // case '4':
        //     return QUINTA;
        case 5:
            return SEXTA;
        // case '5':
        //     return SEXTA;
        case 6:
            return SABADO;
        // case '6':
        //     return SABADO;
        default:
            return 0;
    }
}

struct schedule scheduler_decode_json(char* json) {
    // ESP_LOGI("DECODE JSON", "%s", json);
    cJSON* obj = cJSON_Parse(json);
    
    struct schedule schedule;
    schedule.weekdays = 0;

    cJSON* ativo = cJSON_GetObjectItem(obj, "ativo");
    schedule.active = cJSON_IsTrue(ativo);

    cJSON* quantidade = cJSON_GetObjectItem(obj, "quantidade");
    schedule.quantity = cJSON_GetNumberValue(quantidade);

    cJSON* horario = cJSON_GetObjectItem(obj, "horario");
    char* _horario = cJSON_GetStringValue(horario);
    sscanf(_horario, "%02hhd:%02hhd:%02hhd", &schedule.h, &schedule.m, &schedule.s);

    cJSON* tempoBandeja = cJSON_GetObjectItem(obj, "tempoBandeja");
    schedule.open_time_in_min = cJSON_GetNumberValue(tempoBandeja);

    cJSON* recorrencia = cJSON_GetObjectItem(obj, "recorrencia");
    int recorrencia_array_size = cJSON_GetArraySize(recorrencia);
    for (int i = 0; i < recorrencia_array_size; i++) {
        cJSON* item = cJSON_GetArrayItem(recorrencia, i);
        schedule.weekdays = schedule.weekdays | scheduler_get_week_day((int8_t)cJSON_GetNumberValue(item));
    }
    
    cJSON_Delete(obj);
    return schedule;
}

// TO TEST
bool scheduler_feed_time(char* horario, int* quantidade_total, int* tempo_bandeja) {
    bool result = false;
    int8_t current_week_day = scheduler_get_current_week_day();
    int8_t hour, minute, second;
    get_hms(&hour, &minute, &second);
    int current_second_of_day = SECOND(hour, minute, second);

    nvs_iterator_t it = NULL;
    char value[10];

    esp_err_t res = nvs_entry_find("nvs", "nvs", NVS_TYPE_STR, &it);
    while (res == ESP_OK) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);

        ESP_LOGI("KEY", "%s", info.key);
        int8_t scheduled_days = 0;
        sscanf(&info.key[2], "%03hhd", &scheduled_days);

        if (info.key[0] == 'S' && info.key[1] == 'C' && (current_week_day & scheduled_days) > 0) {
            memset(value, 0, 10);
            scheduler_load(info.key, value);
            struct schedule schedule = scheduler_decode(info.key, value);
            scheduler_print(&schedule);
            int scheduled_second_of_day = SECOND(schedule.h, schedule.m, schedule.s);
            int timedelta = scheduled_second_of_day - current_second_of_day;
            if (schedule.active && timedelta <= 0 && timedelta >= -30) {
                result = true;
                sprintf(horario, "%02hhd:%02hhd:%02hhd", schedule.h, schedule.m, schedule.s);
                *quantidade_total = schedule.quantity;
                *tempo_bandeja = schedule.open_time_in_min;
                scheduler_print(&schedule);
                break;
            }
        }

        res = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);
    
    return result;
}

bool scheduler_equals(struct schedule* s1, struct schedule* s2) {
    bool equal = true;
    equal = (s1->weekdays == s2->weekdays) && equal;
    equal = (s1->h == s2->h) && equal;
    equal = (s1->m == s2->m) && equal;
    equal = (s1->s == s2->s) && equal;
    return equal;
}

void scheduler_save(char* key, char* value) {
    nvs_handle_t handle;
    nvs_open("nvs", NVS_READWRITE, &handle);
    
    nvs_set_str(handle, key, value);

    nvs_commit(handle);

    nvs_close(handle);
}

size_t scheduler_load(char* key, char* value) {
    nvs_handle_t handle;
    nvs_open("nvs", NVS_READONLY, &handle);
    
    size_t length = 0;

    nvs_get_str(handle, key, NULL, &length);

    nvs_get_str(handle, key, value, &length);

    nvs_close(handle);
    return length;
}

void scheduler_encode(struct schedule* schedule, char* key, char* value) {
    key[0] = 'S';
    key[1] = 'C';
    sprintf(&key[2], "%03hhd_", schedule->weekdays);
    sprintf(&key[6], "%02hhd_%02hhd_%02hhd", schedule->h, schedule->m, schedule->s);

    value[0] = schedule->active ? '1' : '0';
    sprintf(&value[1], "%04d_%04d", schedule->quantity, schedule->open_time_in_min);
}

struct schedule scheduler_decode(char* key, char* value) {
    struct schedule schedule;
    
    sscanf(&key[2], "%hhd_%hhd_%hhd_%hhd", &schedule.weekdays, &schedule.h, &schedule.m, &schedule.s);

    schedule.active = value[0] == '1' ? true : false;
    sscanf(&value[1], "%d_%d", &schedule.quantity, &schedule.open_time_in_min);

    return schedule;
}

void scheduler_print(struct schedule* obj) {
    printf("struct schedule: %d, %d, %d - %d:%d:%d - %d\n", obj->active, obj->quantity, obj->weekdays, obj->h, obj->m, obj->s, obj->open_time_in_min);
}

// TO TEST
int8_t scheduler_get_current_week_day() {
    struct tm now = get_time();
    char datetime[2] = {0, 0};
    strftime(datetime, 2, "%w", &now);

    return scheduler_get_week_day(datetime[0]-48);
}

