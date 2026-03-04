#ifndef FLASH_H
#define FLASH_H

#include "stm32f7xx.h"

void flash_unlock(void);
void flash_erase_app(void);
void flash_write_word(uint32_t addr, uint32_t data);

#endif