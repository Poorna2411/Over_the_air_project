#include "led.h"

static void delay_short(uint32_t d)
{
    for(volatile uint32_t i=0;i<d;i++);
}

void led_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    GPIOB->MODER &= ~(3<<(7*2));
    GPIOB->MODER |= (1<<(7*2));
}

void led_on(void)
{
    GPIOB->BSRR = (1<<7);
}

void led_off(void)
{
    GPIOB->BSRR = (1<<(7+16));
}

void led_toggle(void)
{
    GPIOB->ODR ^= (1<<7);
}

/* Slow blink (boot window) */
void led_slow_blink(void)
{
    led_toggle();
    delay_short(1200000);
}

/* Medium blink (flash erase) */
void led_medium_blink(void)
{
    led_toggle();
    delay_short(400000);
}

/* Fast blink (firmware write) */
void led_fast_blink(void)
{
    led_toggle();
    delay_short(100000);
}

/* OTA complete pattern */
void led_flash_complete(void)
{
    for(int i=0;i<3;i++)
    {
        led_on();
        delay_short(200000);
        led_off();
        delay_short(200000);
    }
}