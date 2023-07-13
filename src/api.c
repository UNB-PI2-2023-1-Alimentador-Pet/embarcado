#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "string.h"
#include "cJSON.h"
#include "mqtthandler.h"
#include "time_handle.h"

#define APISERVER "miaufeeder.herokuapp.com"

static int current_id;

static const char* root_ca =
"-----BEGIN CERTIFICATE-----\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\n"
"-----END CERTIFICATE-----\n";

int get_current_id() {
    return current_id = 0;
}

esp_err_t api_create_history_event_handler(esp_http_client_event_t* evt) {
    
    static int output_len;
    static char* output_buffer;

    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD("HTTP CREATE HISTORY HANDLER", "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE("HTTP CREATE HISTORY HANDLER", "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD("HTTP CREATE HISTORY HANDLER", "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
        default:
            break;
    }
    return ESP_OK;
}

int api_create_history(char* horario, int qtd_total) {
    // This returns the id of the history
    // TODO send request
    int id = 0;
    char data_buffer[200];
    char resp_buffer[500];

    esp_http_client_config_t config = {
        .host = APISERVER,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .path = "/histories/new",
        .method = HTTP_METHOD_POST,
        .cert_pem = root_ca,
        .user_data = resp_buffer,
        .event_handler = api_create_history_event_handler
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

    ESP_LOGI("HTTP CLIENT", "buffer: %s", data_buffer);

    esp_http_client_handle_t handle = esp_http_client_init(&config);

    esp_http_client_set_header(handle, "Content-Type", "application/json");

    esp_http_client_set_post_field(handle, data_buffer, strlen(data_buffer));

    esp_err_t err = esp_http_client_perform(handle);

    if (err != ESP_OK) {
        ESP_LOGE("HTTP CLIENT", "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    int status_code = esp_http_client_get_status_code(handle);
    ESP_LOGI("HTTP CLIENT", "HTTP STATUS CODE: %d", status_code);
    ESP_LOGI("HTTP CLIENT", "Content Length: %"PRId64, esp_http_client_get_content_length(handle));

    if (status_code != 200) {
        esp_http_client_cleanup(handle);
        return 0;
    }

    ESP_LOGI("HTTP_CLIENT", "/histories/new resp buffer: %s", resp_buffer);
    esp_http_client_cleanup(handle);

    cJSON* resp = cJSON_Parse(resp_buffer);

    id = cJSON_GetNumberValue(cJSON_GetObjectItem(resp, "id"));

    cJSON_Delete(resp);

    current_id = id;
    return id;
}

void api_update_history(int id, int quantidade_consumida) {
    char path[20];
    sprintf(path, "/histories/%d", current_id);

    esp_http_client_config_t config = {
        .host = APISERVER,
        .path = path,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .cert_pem = root_ca,
        .method = HTTP_METHOD_PUT,
    };

    cJSON* obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "quantidadeConsumida", quantidade_consumida);

    esp_http_client_handle_t handle = esp_http_client_init(&config);

    char* request_data = cJSON_Print(obj);
    size_t request_data_len = strlen(request_data);

    esp_http_client_set_header(handle, "Content-Type", "application/json");
    esp_http_client_set_post_field(handle, request_data, request_data_len);
    esp_err_t err = esp_http_client_perform(handle);

    if (err != ESP_OK) {
        ESP_LOGE("HTTP CLIENT", "HTTP PUT request failed: %s", esp_err_to_name(err));
    }

    ESP_LOGI("HTTP CLIENT", "HTTP STATUS CODE: %d", esp_http_client_get_status_code(handle));
    ESP_LOGI("HTTP CLIENT", "Content Length: %"PRId64, esp_http_client_get_content_length(handle));

    cJSON_Delete(obj);
    esp_http_client_cleanup(handle);
}