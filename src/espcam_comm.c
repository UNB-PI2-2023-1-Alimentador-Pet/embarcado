#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "string.h"
#include "espcam_comm.h"
#include "crc16.h"
#include "cJSON.h"

#define UART_BUFFER_SIZE (1024 * 2)
static const uart_port_t uart_num = UART_NUM_2;

static QueueHandle_t queue;

void start_uart_espcam() {
    
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT
    };
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 17, 16, 22, 19));

    // Setup UART buffered IO with event queue
    // const int uart_buffer_size = (1024 * 2);
    QueueHandle_t uart_queue;
    // Install UART driver using an event queue here
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, UART_BUFFER_SIZE, UART_BUFFER_SIZE, 10, &uart_queue, 0));
    queue = xQueueCreate(3, UART_BUFFER_SIZE);
    start_esp32cam_communication();
}

void espcam_enqueue_message(const char* message) {
    xQueueSend(queue, (void*)message, (TickType_t)1);
}

static void echo_task(void *arg) {
    char* rxbuffer = malloc(UART_BUFFER_SIZE);
    char* txbuffer = malloc(UART_BUFFER_SIZE);
    char* msgbuffer = malloc(UART_BUFFER_SIZE);
    while(1) {
        xQueueReceive(queue, msgbuffer, portMAX_DELAY);
        ESP_LOGI("SYNC", "message queue: %s", msgbuffer);
        int8_t tries = 0;
        uart_flush(UART_NUM_2);
        while (1) {
            memset(rxbuffer, 0, UART_BUFFER_SIZE);
            memset(txbuffer, 0, UART_BUFFER_SIZE);
            
            char* sync_str = "SYNC";
            uart_write_bytes(uart_num, sync_str, strlen(sync_str));

            vTaskDelay(400 / portTICK_PERIOD_MS);

            int length = 0;
            // ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_num, (size_t*)&length));
            length = uart_read_bytes(uart_num, rxbuffer, UART_BUFFER_SIZE, 100);
            ESP_LOGI("SYNC", "buffer(%d): %s", length, rxbuffer);
            if (!strcmp(rxbuffer, sync_str)) {
                
                ESP_LOGI("SYNC", "YES");

                sprintf(txbuffer, "%03hhd%s", strlen(msgbuffer), msgbuffer);
                int16_t crc = calcula_CRC((unsigned char*)txbuffer, strlen(txbuffer));
                sprintf(&txbuffer[strlen(txbuffer)], "%05hd", crc);

                uart_write_bytes(UART_NUM_2, txbuffer, strlen(txbuffer));

                memset(rxbuffer, 0, UART_BUFFER_SIZE);
                vTaskDelay(500 / portTICK_PERIOD_MS);

                memset(rxbuffer, 0, UART_BUFFER_SIZE);
                length = uart_read_bytes(uart_num, rxbuffer, UART_BUFFER_SIZE, 100);

                cJSON* obj = cJSON_Parse(rxbuffer);
                
                // ESP_LOGI("EspCamStatus", "Data: %s", rxbuffer);
                // ESP_LOGI("EspCamStatus", "Has item: %d", cJSON_HasObjectItem(obj, "status"));

                bool resp_status = cJSON_IsTrue(cJSON_GetObjectItem(obj, "status"));
                ESP_LOGI("EspCamStatus", "%d", resp_status);
                if (resp_status) {
                    cJSON_Delete(obj);
                    break;
                }

                cJSON_Delete(obj);


                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            
            if (++tries >= 5) {
                break;
            }
        }
    }
}

void start_esp32cam_communication() {
    xTaskCreate(echo_task, "uart_echo_task", 4096, NULL, 10, NULL);
}