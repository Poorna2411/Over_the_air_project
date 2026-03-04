#include "ota_uart.h"

void uart4_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    RCC->APB1ENR |= RCC_APB1ENR_UART4EN;

    GPIOD->MODER |= (2<<(0*2)) | (2<<(1*2));
    GPIOD->AFR[0] |= (8<<(0*4)) | (8<<(1*4));

    UART4->BRR = 16000000/115200;

    UART4->CR1 = USART_CR1_TE |
                 USART_CR1_RE |
                 USART_CR1_UE;
}

uint8_t uart4_recv(void)
{
    while(!(UART4->ISR & USART_ISR_RXNE));
    return UART4->RDR;
}

void uart4_send(char c)
{
    while(!(UART4->ISR & USART_ISR_TXE));
    UART4->TDR=c;
}

void uart4_print(char *s)
{
    while(*s)
        uart4_send(*s++);
}