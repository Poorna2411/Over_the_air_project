#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#include "stm32f7xx.h"

void usart3_init(void);
void debug_print(char *s);

#endif