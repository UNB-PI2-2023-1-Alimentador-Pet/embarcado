#ifndef _TIME_HANDLE_H_
#define _TIME_HANDLE_H_

#include "time.h"

void sync_time();
struct tm get_time();
// void get_hour_and_minute(int8_t* hour, int8_t* minute);
void get_hms(int8_t* hour, int8_t* minute, int8_t* second);
bool is_time_or_later(const char* _time);

#endif