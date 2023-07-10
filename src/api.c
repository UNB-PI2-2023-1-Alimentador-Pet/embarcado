#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "string.h"
#include "cJSON.h"
#include "mqtthandler.h"
#include "time_handle.h"

#define APISERVER "https://miaufeeder.herokuapp.com"

int api_create_history(char* horario, int qtd_total) {
    // This returns the id of the history
    // TODO send request
    int id = 0;
    char data_buffer[200];
    esp_http_client_config_t config = {
        .url = APISERVER,
        .path = "/histories/new",
        .method = HTTP_METHOD_POST
    };

    char date[12];
    get_current_date(date, 12);

    cJSON* obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "userHash", get_user_hash(NULL));
    cJSON_AddStringToObject(obj, "data", date);
    cJSON_AddNumberToObject(obj, "quantidadeTotal", qtd_total);
    cJSON_AddNumberToObject(obj, "quantidadeConsumida", 0);
    cJSON_AddStringToObject(obj, "horario", horario);

    strcpy(data_buffer, cJSON_Print(obj));

    cJSON_Delete(obj);

    esp_http_client_handle_t handle = esp_http_client_init(&config);
    esp_http_client_set_post_field(handle, data_buffer, strlen(data_buffer));

    esp_err_t err = esp_http_client_perform(handle);

    if (err != ESP_OK) {
        ESP_LOGE("HTTP CLIENT", "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    ESP_LOGI("HTTP CLIENT", "HTTP STATUS CODE: %d", esp_http_client_get_status_code(handle));

    char resp_buffer[500];
    esp_http_client_read_response(handle, resp_buffer, 500);
    
    cJSON* resp = cJSON_Parse(resp_buffer);

    id = cJSON_GetNumberValue(cJSON_GetObjectItem(resp, "id"));

    cJSON_Delete(resp);
    esp_http_client_cleanup(handle);

    return id;
}

void api_update_history(int id, int quantidade_consumida) {
    char path[20];
    sprintf(path, "histories/%d", id);

    esp_http_client_config_t config = {
        .url = APISERVER,
        .path = path,
        .method = HTTP_METHOD_PUT,
    };

    cJSON* obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "quantidadeConsumida", quantidade_consumida);

    esp_http_client_handle_t handle = esp_http_client_init(&config);

    char* request_data = cJSON_Print(obj);
    size_t request_data_len = strlen(request_data);

    esp_http_client_set_post_field(handle, request_data, request_data_len);
    esp_err_t err = esp_http_client_perform(handle);

    if (err != ESP_OK) {
        ESP_LOGE("HTTP CLIENT", "HTTP PUT request failed: %s", esp_err_to_name(err));
    }

    ESP_LOGI("HTTP CLIENT", "HTTP STATUS CODE: %d", esp_http_client_get_status_code(handle));

    cJSON_Delete(obj);
    esp_http_client_cleanup(handle);
}