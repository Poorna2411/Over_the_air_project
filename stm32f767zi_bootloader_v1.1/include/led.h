#ifndef LED_H
#define LED_H

#include "stm32f7xx.h"

void led_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);

void led_slow_blink(void);
void led_medium_blink(void);
void led_fast_blink(void);
void led_flash_complete(void);

#endif