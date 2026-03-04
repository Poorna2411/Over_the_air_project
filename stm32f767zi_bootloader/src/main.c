#include "stm32f7xx.h"
#include <stdio.h>

#define APP_START 0x08020000
#define APP_END   0x081FFFFF

typedef void (*pFunction)(void);

/* ================= DELAY ================= */
void delay(void){
    for(volatile uint32_t i=0;i<800000;i++);
}

/* ================= DEBUG UART USART3 ================= */
void usart3_init(void){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    GPIOD->MODER |= (2<<(8*2))|(2<<(9*2));
    GPIOD->AFR[1] |= (7<<0)|(7<<4);

    USART3->BRR = 16000000/115200;
    USART3->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void debug_print(char *s){
    while(*s){
        while(!(USART3->ISR & USART_ISR_TXE));
        USART3->TDR=*s++;
    }
}

/* ================= UART4 OTA ================= */
void uart4_init(void){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    RCC->APB1ENR |= RCC_APB1ENR_UART4EN;

    GPIOD->MODER |= (2<<(0*2))|(2<<(1*2));
    GPIOD->AFR[0] |= (8<<(0*4))|(8<<(1*4));

    UART4->BRR = 16000000/115200;
    UART4->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

uint8_t uart4_recv(void){
    while(!(UART4->ISR & USART_ISR_RXNE));
    return UART4->RDR;
}

void uart4_send(char c){
    while(!(UART4->ISR & USART_ISR_TXE));
    UART4->TDR=c;
}

void uart4_print(char *s){
    while(*s) uart4_send(*s++);
}

/* ================= FLASH FUNCTIONS ================= */

void flash_unlock(void){
    if(FLASH->CR & FLASH_CR_LOCK){
        FLASH->KEYR=0x45670123;
        FLASH->KEYR=0xCDEF89AB;
    }
}

void flash_erase_app(void)
{
    debug_print("Erasing Flash...\r\n");

    flash_unlock();

    FLASH->SR |= FLASH_SR_OPERR |
                 FLASH_SR_WRPERR |
                 FLASH_SR_PGAERR |
                 FLASH_SR_PGPERR;

    for(int sector=5; sector<=11; sector++)
    {
        while(FLASH->SR & FLASH_SR_BSY);

        FLASH->CR &= ~FLASH_CR_SNB;
        FLASH->CR |= FLASH_CR_SER;
        FLASH->CR |= (sector << FLASH_CR_SNB_Pos);
        FLASH->CR |= FLASH_CR_STRT;

        while(FLASH->SR & FLASH_SR_BSY);

        FLASH->CR &= ~FLASH_CR_SER;
    }

    debug_print("Erase Done\r\n");
}

void flash_write_word(uint32_t addr, uint32_t data)
{
    while(FLASH->SR & FLASH_SR_BSY);

    FLASH->CR &= ~FLASH_CR_PSIZE;
    FLASH->CR |= FLASH_CR_PSIZE_1;  // 32-bit write

    FLASH->CR |= FLASH_CR_PG;

    *(volatile uint32_t*)addr = data;

    while(FLASH->SR & FLASH_SR_BSY);

    FLASH->CR &= ~FLASH_CR_PG;
}

/* ================= JUMP TO APPLICATION ================= */

void jump_to_app(void)
{
    uint32_t appStack = *(volatile uint32_t*)APP_START;
    uint32_t appEntry = *(volatile uint32_t*)(APP_START + 4);

    char msg[80];
    sprintf(msg,"SP=%08lX ENTRY=%08lX\r\n",appStack,appEntry);
    debug_print(msg);

    if((appStack & 0x2FFE0000) == 0x20000000)
    {
        debug_print("Jumping to Application\r\n");

        __disable_irq();
        SCB->VTOR = APP_START;
        __set_MSP(appStack);

        ((pFunction)appEntry)();
    }
    else
    {
        debug_print("failed\r\n");
    }
}

/* ================= MAIN ================= */

int main(void)
{
    usart3_init();
    uart4_init();

    debug_print("\r\n=== OTA BOOTLOADER START ===\r\n");

    /* Boot window (~3 sec) */
    for(int t=0; t<300; t++)
    {
        if(UART4->ISR & USART_ISR_RXNE)
        {
            if(uart4_recv()=='U')
            {
                debug_print("OTA Triggered\r\n");

                flash_erase_app();

                uart4_print("READY");

                /* Receive firmware size */
                uint32_t size=0;
                for(int i=0;i<4;i++)
                    size=(size<<8)|uart4_recv();

                uart4_print("OK");

                /* Receive firmware safely */
                uint32_t addr=APP_START;
                uint32_t received=0;

                while(received<size)
                {
                    uint32_t word=0xFFFFFFFF;

                    for(int j=0;j<4;j++)
                    {
                        if(received<size)
                        {
                            word&=~(0xFF<<(8*j));
                            word|=uart4_recv()<<(8*j);
                            received++;
                        }
                    }

                    flash_write_word(addr,word);
                    addr+=4;

                    uart4_print("ACK");
                }

                uart4_print("DONE");
                debug_print("OTA COMPLETE\r\n");

                FLASH->CR |= FLASH_CR_LOCK;

                delay();
                NVIC_SystemReset();
            }
        }
        delay();
    }

    debug_print("Booting Application...\r\n");
    jump_to_app();

    while(1);
}