#ifndef _WIFI_H_
#define _WIFI_H_

#define USE_STA_DEFAULT 0

void wifi_init();
void connect_wifi_sta(const char*, const char*);
void esp_data_message(char* message);

#endif
