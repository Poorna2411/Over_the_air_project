#include "flash.h"
#include "debug_uart.h"
#include "config.h"
#include <stdio.h>

void flash_unlock(void)
{
    if(FLASH->CR & FLASH_CR_LOCK)
    {
        FLASH->KEYR=0x45670123;
        FLASH->KEYR=0xCDEF89AB;
    }
}

void flash_erase_app(void)
{
    debug_print("Starting Flash Erase...\r\n");

    flash_unlock();

    for(int sector=5; sector<=11; sector++)
    {
        char msg[50];
        sprintf(msg,"Erasing sector %d...\r\n",sector);
        debug_print(msg);

        while(FLASH->SR & FLASH_SR_BSY);

        FLASH->CR &= ~FLASH_CR_SNB;
        FLASH->CR |= FLASH_CR_SER;
        FLASH->CR |= (sector << FLASH_CR_SNB_Pos);
        FLASH->CR |= FLASH_CR_STRT;

        while(FLASH->SR & FLASH_SR_BSY);

        debug_print("Sector erased\r\n");

        FLASH->CR &= ~FLASH_CR_SER;
    }

    debug_print("Flash erase completed\r\n");
}

void flash_write_word(uint32_t addr,uint32_t data)
{
    while(FLASH->SR & FLASH_SR_BSY);

    FLASH->CR &= ~FLASH_CR_PSIZE;
    FLASH->CR |= FLASH_CR_PSIZE_1;

    FLASH->CR |= FLASH_CR_PG;

    *(volatile uint32_t*)addr=data;

    while(FLASH->SR & FLASH_SR_BSY);

    FLASH->CR &= ~FLASH_CR_PG;
}