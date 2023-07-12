#ifndef _STATUS_H_
#define _STATUS_H_

#define CONN_IDLE 0
#define CONN_TRYING 1
#define CONN_FAIL 2
#define CONN_CONNECTED 3

void set_connection_status(uint8_t);
uint8_t get_connection_status();
void set_wifi_sta_saved(bool);
bool get_wifi_sta_saved();
void set_bandeja_aberta(bool state);
bool get_bandeja_aberta();
void init_task_send_status();


// void get_user_hash(char* buffer);

#endif