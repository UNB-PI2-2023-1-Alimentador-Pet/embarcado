#ifndef _MQTTHANDLER_H_
#define _MQTTHANDLER_H_

void provide_food_handler(const char* json);
void close_feeder_handler(const char* json);
void schedule_save_handler(const char* json);
void send_tank_level(int value);

char* get_user_hash(size_t* hash_size);

void send_status();
void remote_open(char* json);

#endif