#include "debug_uart.h"

void usart3_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    GPIOD->MODER |= (2<<(8*2)) | (2<<(9*2));
    GPIOD->AFR[1] |= (7<<0) | (7<<4);

    USART3->BRR = 16000000/115200;

    USART3->CR1 = USART_CR1_TE |
                  USART_CR1_RE |
                  USART_CR1_UE;
}

void debug_print(char *s)
{
    while(*s)
    {
        while(!(USART3->ISR & USART_ISR_TXE));
        USART3->TDR=*s++;
    }
}