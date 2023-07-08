#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#define DOMINGO 0b0000001
#define SEGUNDA 0b0000010
#define TERCA   0b0000100
#define QUARTA  0b0001000
#define QUINTA  0b0010000
#define SEXTA   0b0100000
#define SABADO  0b1000000

struct schedule {
    bool active;
    int quantity;
    int8_t weekdays;
    int8_t h;
    int8_t m;
    int8_t s;
    int open_time_in_min;
};

bool scheduler_feed_time();
bool scheduler_equals(struct schedule* s1, struct schedule* s2);
void scheduler_save(char* key, char* value);
size_t scheduler_load(char* key, char* value);
void scheduler_encode(struct schedule* schedule, char* key, char* value);
struct schedule scheduler_decode(char* key, char* value);
void scheduler_print(struct schedule* obj);
int8_t scheduler_get_current_week_day();
struct schedule scheduler_decode_json(char* json);

#endif