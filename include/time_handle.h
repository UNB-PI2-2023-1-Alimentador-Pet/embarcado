#ifndef _TIME_HANDLE_H_
#define _TIME_HANDLE_H_

#include "time.h"

void sync_time();
struct tm get_time();
bool is_time_or_later(const char* _time);

#endif