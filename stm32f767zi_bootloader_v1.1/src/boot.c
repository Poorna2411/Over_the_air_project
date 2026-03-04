#include "boot.h"
#include "config.h"
#include "debug_uart.h"
#include "led.h"

typedef void (*pFunction)(void);

void jump_to_app(void)
{
    uint32_t appStack = *(volatile uint32_t*)APP_START;
    uint32_t appEntry = *(volatile uint32_t*)(APP_START + 4);

    debug_print("Jumping directly to application...\r\n");

    /* Turn off bootloader LED */
    led_off();

    /* Disable interrupts */
    __disable_irq();

    /* Stop SysTick */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Disable all NVIC interrupts */
    for(int i=0;i<8;i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    /* Set vector table to application */
    SCB->VTOR = APP_START;

    /* Set application stack pointer */
    __set_MSP(appStack);

    /* Jump to reset handler */
    ((pFunction)appEntry)();
}