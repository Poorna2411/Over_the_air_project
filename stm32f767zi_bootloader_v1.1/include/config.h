#ifndef CONFIG_H
#define CONFIG_H

#include "stm32f7xx.h"

#define APP_START 0x08020000
#define APP_END   0x081FFFFF

#define DEBUG_BAUD 115200
#define OTA_BAUD   115200

#define BOOT_WINDOW 30   // boot wait time

#endif