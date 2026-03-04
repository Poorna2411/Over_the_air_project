#include "delay.h"

void delay(void)
{
    for(volatile uint32_t i=0;i<800000;i++);
}