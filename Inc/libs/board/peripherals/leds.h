#ifndef LEDS_H
#define LEDS_H
//here lives implementation for LEDS

//TODO
/*
    change GPIO registers to gpio.h functions once its implemented
*/

/*
#
# support for onboard usr leds
#
# has functions such as:
#
#   register and clock initialization using init function
#
#   turn target led on / off
#   set all leds on / off at the same time
#
#   'somewhat asynchronous' led blinking
#
#       initialized by blink function
#       [note] for this to work you need to periodically call update function to update LEDS state 
#
#  [note] no, external leds not supported support, but feel free to implement it yourself ;D 
#
# made by Tomas Warcholek for project-lila
# 28/10/2026
#
*/

#include "stm32f407xx.h"
#include "timing.h"
#include <stdint.h>

//definition of onboard LED colors
//note that LED_BASE is used mainly for offset calculation, not as alternative for LED_GREEN
typedef enum{
    LED_BASE    = 12U,
    LED_GREEN   = 12U,
    LED_ORANGE  = 13U,
    LED_RED     = 14U,
    LED_BLUE    = 15U
}LEDS_ClrDef;

//just bunch of #defines, nothing interesting to see here..
//note that LED_ALL_XN is to work with all leds at the same time and save little bit of computing power
#define LED_GREEN_EN    (0x1UL << (LED_GREEN    * 2U))
#define LED_ORANGE_EN   (0x1UL << (LED_ORANGE   * 2U))
#define LED_RED_EN      (0x1UL << (LED_RED      * 2U))
#define LED_BLUE_EN     (0x1UL << (LED_BLUE     * 2U))
#define LED_ALL_EN      (0x55UL<< (LED_BASE     * 2U))

#define LED_GREEN_ON    (0x1UL << LED_GREEN)
#define LED_ORANGE_ON   (0x1UL << LED_ORANGE)
#define LED_RED_ON      (0x1UL << LED_RED)
#define LED_BLUE_ON     (0x1UL << LED_BLUE)
#define LED_ALL_ON      (0xFUL << LED_BASE)


//vector definition for blinking functionality
//its 640bits what?! o_O
uint32_t LED_VECTOR[4]              = {0U,0U,0U,0U};    //is in blinking mode?
uint32_t LED_STATE_VECTOR[4]        = {0U,0U,0U,0U};    //is led on/off?
uint32_t LED_TIMES_VECTOR[4]        = {0U,0U,0U,0U};    //how many times should led blink
uint32_t LED_LAST_BLINK_VECTOR[4]   = {0U,0U,0U,0U};    //last recorded blink time in ticks
uint32_t LED_BLINK_LENGTH_VECTOR[4] = {0U,0U,0U,0U};    //blink state period

//clock and GPIO initialization for internal leds
void leds_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    GPIOD->MODER |= LED_ALL_EN;
}
//self explanatory?
void leds_set_on_all(void)
{
    GPIOD->BSRR = LED_ALL_ON;
}
//self explanatory?
void leds_set_off_all(void)
{
    GPIOD->BSRR = LED_ALL_ON << 16U;
}
//self explanatory?
void leds_set_on(LEDS_ClrDef LED)
{
    GPIOD->BSRR = 1UL << LED;
}
//self explanatory?
void leds_set_off(LEDS_ClrDef LED)
{
    GPIOD->BSRR = 1UL << (LED + 16U);
}
//led blink initialization for target led
//note that this function can also update already blinking leds, 
//setting times to 0 will turn led off in the next update cycle

void leds_blink_on(LEDS_ClrDef LED, uint32_t blink_length, uint32_t times)
{
    if(times == 0)
    {
        LED_TIMES_VECTOR[LED-LED_BASE] = 0;
    }
    else
    {
        LED_TIMES_VECTOR[LED-LED_BASE] += times;                //how many blink repetitions?
    }
                                   
    LED_BLINK_LENGTH_VECTOR[LED-LED_BASE] = (blink_length/2);   //sets period for each on/off state
    LED_VECTOR[LED-LED_BASE] = 1U;                              //mark LED as its in blinking mode

    leds_set_on(LED);                                           //initial LED turn on
    LED_STATE_VECTOR[LED-LED_BASE] = 1U;                        //updates LED state as on
}


//function to update blinking LEDs
//this needs to be called periodically
//placing this in your main loop is suggested
//[note]this is only required for blinking functionality
void leds_blink_update()
{

    for (uint8_t i = 0U; i < 4U; i++) 
    {
        if(LED_VECTOR[i] == 0U) //is led in blinking mode?
        {
            continue;
        }
        if(systick_ticks - LED_LAST_BLINK_VECTOR[i] >= LED_BLINK_LENGTH_VECTOR[i])  //is it time for LED to blink?
        {
            if (LED_STATE_VECTOR[i] == 0U) //this is the blinking 
            {
                leds_set_on(i+LED_BASE);
                LED_STATE_VECTOR[i] = 1U;
            }
            else
            {
                leds_set_off(i+LED_BASE);
                LED_STATE_VECTOR[i] = 0U;
                LED_TIMES_VECTOR[i]--;

                if(LED_TIMES_VECTOR[i]==0U)
                {
                    LED_VECTOR[i] = 0U;
                }
            }
            LED_LAST_BLINK_VECTOR[i] = systick_ticks;
        }
    }
}

#endif //and thats all my friends