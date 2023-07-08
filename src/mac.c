#include "esp_mac.h"
#include "esp_err.h"
#include "string.h"

#define MACSTRF "%02x%02x%02x%02x%02x%02x"
static char mac_address[18];

void mac_address_init() {
    uint8_t addr[8];
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(addr));
    sprintf(mac_address, MACSTRF, MAC2STR(addr));
}

char* get_mac_address() {
    return mac_address;
}