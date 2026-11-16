#ifndef TIMING_H
#define TIMING_H
//here lives Systick and irq configuration
/*
# initialization for core clockrate
# 
# [note] SysTick_Init(uint8_t tick_period) must be called in your main function for correct delay and other configurations that are tickrate deependent! 
# made by Tomas Warcholek for project-lila
# 28/10/2026
*/


#include "stm32f407xx.h"
#include "system_stm32f4xx.h"

//core-clock tick update 

/**
/param tick_period is in miliseconds
 */
void SysTick_Init(uint8_t tick_period)
{
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / (1000U / tick_period));
    
}

//tick counter

volatile uint32_t systick_ticks = 0;

void SysTick_Handler(void)
{
    systick_ticks++;
}

#endif //and thats all my friends