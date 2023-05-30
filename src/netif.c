#include <stdio.h>
#include <inttypes.h>
#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types.h"
#include "hal/esp_flash_err.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_wifi.h"
#include "string.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi_types.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "wifi.h"

// void get_info() {
//     esp_netif_t* esp_netif = esp_netif_next(NULL);
//     size_t number_of_ifs = esp_netif_get_nr_of_ifs();

//     ESP_LOGI("netif", "number of interfaces: %d\n", number_of_ifs);

//     esp_netif_ip_info_t 

//     if (esp_netif_set_ip_info(esp_netif, ))
// }
