#ifndef _MQTT_H_
#define _MQTT_H_

void mqtt_app_start();
int mqtt_app_publish(char* topic, char* message, int qos);
void mqtt_app_close();
bool is_mqtt_connected();

#endif