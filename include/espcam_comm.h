#ifndef _ESPCAM_COMM_H_
#define _ESPCAM_COMM_H_

void start_uart_espcam();
void espcam_enqueue_message(const char* message);
void start_esp32cam_communication();

#endif