#ifndef OTA_UART_H
#define OTA_UART_H

#include "stm32f7xx.h"

void uart4_init(void);
uint8_t uart4_recv(void);
void uart4_send(char c);
void uart4_print(char *s);

#endif