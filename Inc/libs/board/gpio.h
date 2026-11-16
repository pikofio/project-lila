#ifndef GPIO_H
#define GPIO_H
//here lives implementation for dynamic register initializations and operations such as ENABLES, SETUPS, BIT SETS/RESETS, and GPIO

/*
# so i finally decided to implement this...
# library for generalized way to access and work with GPIOx registers pin configuration
# 
# what function each pin does should be specified elsewhere
# please refer to target datasheet to figure out what pin has what AF wired to it
#
# made by Tomas Warcholek for project-lila
# 06/11/2026
*/

#include <stdio.h>
#include "stm32f407xx.h"
#include <stdint.h>
#include "libs/board/pins.h"

#define GPIO_ERROR(msg) \
    printf("[GPIO_ERROR] %s\n", (msg))
#define GPIO_WARNING(msg) \
    printf("[GPIO_WARNING] %s\n", (msg))

// two possible states in IO registers
typedef enum {
    STATE_LOW,
    STATE_HIGH
} GPIO_STATE;

// definition of GPIOx register options
typedef enum {
    INPUT       = 0UL,
    OUTPUT      = 1UL,
    ALTERNATE   = 2UL,
    ANALOG      = 3UL
}GPIO_MODE;

typedef enum {
    PUSH_PULL   = 0UL,
    OPEN_DRAIN  = 1UL
}GPIO_OTYPE;

typedef enum {
    LOW         = 0UL,
    MEDIUM      = 1UL,
    HIGH        = 2UL,
    VERY_HIGH   = 3UL
}GPIO_OSPEED;

typedef enum {
    NO_PUPU     = 0UL,
    PULL_UP     = 1UL,
    PULL_DOWN   = 2UL
}GPIO_PUPD;

#define GPIOx_BASE AHB1PERIPH_BASE      //where first GPIO is located in memory
#define GPIOx_STEP 0x400UL              //step between each GPIO blocks
#define GPIOx_REGISTER_STEP 0x04UL      //step between each register in each GPIO block

/*
#   definition of possible alternate functions
*/

typedef enum {
    // AF0
    SYSTEM_      = 0UL,

    // AF1
    TIM_1_        = 1UL,
    TIM_2_        = 1UL,

    // AF2
    TIM_3_        = 2UL,
    TIM_4_        = 2UL,
    TIM_5_        = 2UL,

    // AF3
    TIM_8_        = 3UL,
    TIM_9_        = 3UL,
    TIM_10_       = 3UL,
    TIM_11_       = 3UL,

    // AF4
    I2C_1_        = 4UL,
    I2C_2_        = 4UL,
    I2C_3_        = 4UL,

    // AF5
    SPI_1_        = 5UL,
    SPI_2_        = 5UL,

    // AF6
    SPI_3_        = 5UL,

    // AF7
    USART_1_      = 7UL,
    USART_2_      = 7UL,
    USART_3_      = 7UL,

    // AF8
    UART_4_       = 8UL,
    UART_5_       = 8UL,
    USART_6_      = 8UL,

    // AF9
    CAN_1_        = 9UL,
    CAN_2_        = 9UL,
    TIM_12_       = 9UL,
    TIM_13_       = 9UL,
    TIM_14_       = 9UL,

    // AF10
    OTG_FS_      = 10UL,
    OTG_HS_      = 10UL,

    // AF11
    ETH_         = 11UL,

    // AF12
    FSMC_        = 12UL,
    SDIO_        = 12UL,
    OTG_HS_FS_   = 12UL,

    // AF13
    DCMI_        = 13UL,

    // AF15
    EVENTOUT_    = 15UL
} ALT_FUNC;

//calculates memory offset of GPIOx based on working pin 
uint32_t get_gpio_offset (PIN pin)
{
    return GPIOx_BASE + (GPIOx_STEP * (pin >> 4));
}

//clock initialization for working GPIO
void gpiox_init(PIN pin)
{
    RCC->AHB1ENR |= (1UL << ((uint32_t)pin >> 4));
}

//for all-in-1 place to setup pin use this function
void pin_setup(PIN pin, GPIO_MODE mode, GPIO_OTYPE otype, GPIO_OSPEED ospeed, GPIO_PUPD pull)
{
    uint32_t pin_pos = ((uint32_t)pin & 0xFUL);

    if ((RCC->AHB1ENR & (1UL << ((uint32_t)pin >> 4))) == 0UL)
    {
        GPIO_WARNING("pin setup while gpio_init() not called, calling gpio_init()now.");
        gpiox_init(pin);
    }
    GPIO_TypeDef *GPIOx = (GPIO_TypeDef *)get_gpio_offset(pin);

    GPIOx->MODER    &= ~(0x3UL << (pin_pos * 2));
    GPIOx->MODER    |= (mode    << pin_pos*2);

    GPIOx->OTYPER   &= ~(0x1UL << pin_pos);
    GPIOx->OTYPER   |= (otype   << pin_pos);

    GPIOx->OSPEEDR  &= ~(0x3UL << (pin_pos * 2));
    GPIOx->OSPEEDR  |= (ospeed  << pin_pos*2);

    GPIOx->PUPDR    &= ~(0x3UL << (pin_pos * 2));
    GPIOx->PUPDR    |= (pull    << pin_pos*2);
}

//use this alongside pin_setup(PIN pin, GPIO_MODE ALTERNATE, ...) to choose what alternate function should be used
void pin_set_alternate(PIN pin, ALT_FUNC func){

    GPIO_TypeDef *GPIOx = (GPIO_TypeDef *)get_gpio_offset(pin);
    uint32_t pin_pos = ((uint32_t)pin & 0xFUL);

        GPIOx->AFR[pin_pos >> 3] &= ~(15UL << (pin_pos & 0x7UL) * 4);
        GPIOx->AFR[pin_pos >> 3] |= (func << (pin_pos & 0x7UL) * 4);
}

//read input state once from IDR register on working pin
GPIO_STATE gpio_get_input(PIN pin)
{
    GPIO_TypeDef *GPIOx = (GPIO_TypeDef *)get_gpio_offset(pin);
    uint32_t pin_pos = ((uint32_t)pin & 0xFUL);

    return GPIOx->IDR & (1UL << pin_pos) ? STATE_HIGH : STATE_LOW;
}

//set output state once in ODR register through BSRR register on working pin
void gpio_set_output(PIN pin, GPIO_STATE state)
{
    GPIO_TypeDef *GPIOx = (GPIO_TypeDef *)get_gpio_offset(pin);
    uint32_t pin_pos = ((uint32_t)pin & 0xFUL);

    if (state == STATE_HIGH)
    {
        GPIOx->BSRR = (1UL << pin_pos);
    }
    else if(state == STATE_LOW)
    {
        GPIOx->BSRR = (1UL << (pin_pos+16));
    }
}

#endif //and thats all my friends