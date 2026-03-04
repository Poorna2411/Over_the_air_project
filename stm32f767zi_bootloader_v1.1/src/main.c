#include "config.h"
#include "delay.h"
#include "debug_uart.h"
#include "ota_uart.h"
#include "flash.h"
#include "boot.h"
#include "led.h"

#include <stdio.h>

/* ================= MAIN ================= */

int main(void)
{
    usart3_init();
    uart4_init();
    led_init();

    led_on();

    debug_print("\r\n=================================\r\n");
    debug_print(" STM32F767 OTA BOOTLOADER START\r\n");
    debug_print("=================================\r\n");

    debug_print("Bootloader Address : 0x08000000\r\n");
    debug_print("Application Address: 0x08020000\r\n");
    debug_print("Waiting for OTA trigger...\r\n");

    /* ================= BOOT WINDOW ================= */

    for(int t=0; t<BOOT_WINDOW; t++)
    {
        led_slow_blink();   // slow blink during boot window

        char msg[40];
        sprintf(msg,"Boot window %d\r\n",BOOT_WINDOW - t);
        debug_print(msg);

        if(UART4->ISR & USART_ISR_RXNE)
        {
            if(uart4_recv() == 'U')
            {
                debug_print("OTA TRIGGER RECEIVED\r\n");

                /* ================= FLASH ERASE ================= */

                debug_print("Starting Flash Erase...\r\n");

                flash_erase_app();

                uart4_print("READY");

                /* ================= RECEIVE SIZE ================= */

                uint32_t size = 0;

                for(int i=0;i<4;i++)
                    size = (size<<8) | uart4_recv();

                char msg2[60];
                sprintf(msg2,"Firmware size: %lu bytes\r\n",size);
                debug_print(msg2);

                uart4_print("OK");

                /* ================= RECEIVE FIRMWARE ================= */

                uint32_t addr = APP_START;
                uint32_t received = 0;

                debug_print("Receiving firmware...\r\n");

                while(received < size)
                {
                    uint32_t word = 0xFFFFFFFF;

                    for(int j=0;j<4;j++)
                    {
                        if(received < size)
                        {
                            word &= ~(0xFF << (8*j));
                            word |= uart4_recv() << (8*j);
                            received++;
                        }
                    }

                    flash_write_word(addr,word);

                    led_fast_blink();   // fast blink during flash write

                    char msg3[60];
                    sprintf(msg3,"Flash write: 0x%08lX\r\n",addr);
                    debug_print(msg3);

                    addr += 4;

                    if(received % 1024 == 0)
                    {
                        uint32_t percent = (received * 100) / size;

                        char msg4[50];
                        sprintf(msg4,"Progress: %lu%%\r\n",percent);
                        debug_print(msg4);
                    }

                    uart4_print("ACK");
                }

                uart4_print("DONE");

                debug_print("Firmware received successfully\r\n");
                debug_print("OTA UPDATE COMPLETE\r\n");

                led_flash_complete();

                FLASH->CR |= FLASH_CR_LOCK;

                debug_print("Flash locked\r\n");
                debug_print("System resetting...\r\n");

                delay();

                NVIC_SystemReset();
            }
        }

        delay();
    }

    /* ================= BOOT APPLICATION ================= */

    debug_print("Boot window expired\r\n");
    debug_print("Booting application...\r\n");

    led_off();

    jump_to_app();

    while(1);
}