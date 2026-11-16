#ifndef UART_H
#define UART_H
//here lives implementation of uart

/*
# asynchronous USART read/write library
# 
# uses interupts to handle TX/RX response
# 
# use uart_send(uint8_t *message, <size of the message in bytes>); for transmission of a message
# use uart_send_char(uint8_t char); for single byte transmission
#
# data reception is handled inside the interrupt handler, but message storing itself is handled by uart_update() function 
# which should be called periodically to avoid overflow and corruption of recieved message
# update function handles 1 byte per run, this can be change by modifying UART_MAX_UPDATE_BYTES to desired number of bytes
#
# maximum acceptable string and buffer length can be change by modifying UART_MAX_BUFFER_SIZE 
# [!WARNING] note that UART_MAX_BUFFER_SIZE must be a power of 2
#
# made by Tomas Warcholek for project-lila
# 14/11/2026
*/

#include "libs/board/gpio.h"
#include "pins.h"
#include "stm32f407xx.h"
#include <string.h>

// global variable definitions

#ifndef UART_MAX_BUFFER_SIZE
#define UART_MAX_BUFFER_SIZE 128U
#endif

#ifndef UART_MAX_UPDATE_BYTES
#define UART_MAX_UPDATE_BYTES 1U
#endif

#define UART_BUFFER_MASK (UART_MAX_BUFFER_SIZE - 1)

volatile uint8_t RX_CIRCLE_BUFFER[UART_MAX_BUFFER_SIZE]={0};
static uint8_t TX_BUFFER[UART_MAX_BUFFER_SIZE]={0};

volatile uint8_t RX_CIRCLE_INDEX = 0;
volatile uint8_t RX_BUFFER_INDEX = 0;

volatile uint16_t TX_HEAD = 0U;
volatile uint16_t TX_TAIL = 0U;

volatile uint8_t CB_INDEX = 0;

volatile uint8_t RX_DATA_READY = 0;


//interrupt handler for UART operations
void USART2_IRQHandler()
{
    // Reception handler
    if (USART2->SR & USART_SR_RXNE)
    {
        if(((RX_CIRCLE_INDEX + 1U) & UART_BUFFER_MASK) == RX_BUFFER_INDEX)
        {
            USART2->DR;
        }
        else
        {
            RX_CIRCLE_BUFFER[RX_CIRCLE_INDEX] = (uint8_t)USART2->DR;
            RX_CIRCLE_INDEX = (RX_CIRCLE_INDEX + 1U) & UART_BUFFER_MASK;
        }
    }

    //transmission handler
    if ((USART2->SR & USART_SR_TXE) && (USART2->CR1 & USART_CR1_TXEIE))
    {
        if (TX_TAIL != TX_HEAD)
        {
            USART2->DR = TX_BUFFER[TX_TAIL];

            TX_TAIL = (TX_TAIL + 1U) & UART_BUFFER_MASK;
        }
        else
        {
            USART2->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}

//moves data that are to be transmitted into transmission queue (TX_BUFFER) and enables TXEIE interrupt
void uart_send(const uint8_t *data, uint8_t length)
{
    if (data == NULL || length == 0U)
        return;

    uint16_t head = TX_HEAD;
    uint16_t tail = TX_TAIL;

    uint16_t used = (head - tail) & UART_BUFFER_MASK;
    uint16_t free_space = UART_MAX_BUFFER_SIZE - 1U - used;

    if (length > free_space)
        return;

    for (uint8_t i = 0U; i < length; i++)
    {
        TX_BUFFER[head] = data[i];
        head = (head + 1U) & UART_BUFFER_MASK;
    }

    TX_HEAD = head;

    USART2->CR1 |= USART_CR1_TXEIE;
}

//QoL function to send single character
void uart_send_char(uint8_t data)
{
    uart_send(&data, 1U);
}

//main function to process uart reception

//end of message is indicated by recieving '\r' symbol which sets RX_DATA_READY, 
//indicating that whole message has been recieved and buffer wont accept new input until RX_DATA_READY is cleared

//[!note] rx_buffer must be emptied before each new reception else new input will overlap with the old one
void uart_update(uint8_t* rx_buffer)
{
    uint8_t processed = 0U;

    while(!RX_DATA_READY && RX_BUFFER_INDEX != RX_CIRCLE_INDEX && processed < UART_MAX_UPDATE_BYTES)
    {
        uint8_t data;

        data = RX_CIRCLE_BUFFER[RX_BUFFER_INDEX];
        RX_BUFFER_INDEX = (RX_BUFFER_INDEX + 1U) & UART_BUFFER_MASK;

        processed++;

        uart_send_char(data);

        if(data == '\r')
        {
            rx_buffer[CB_INDEX] = '\0';
            uart_send_char('\n');

            RX_DATA_READY = 1;
            CB_INDEX = 0;
        }
        else if (CB_INDEX < UART_MAX_BUFFER_SIZE - 1U)
        {
            rx_buffer[CB_INDEX] = data;
            CB_INDEX++;
        }
        else
        {
            CB_INDEX = 0;
        }
    }
}

//initialization function for uart, must be called to enable and set uart peripheral
void uart_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    pin_setup(USART2_RX, ALTERNATE, PUSH_PULL, VERY_HIGH, PULL_UP);
    pin_set_alternate(USART2_RX, USART_2_);

    pin_setup(USART2_TX, ALTERNATE, PUSH_PULL, VERY_HIGH, PULL_UP);
    pin_set_alternate(USART2_TX, USART_2_);

    USART2->CR1 |= USART_CR1_OVER8;  //sets oversampling to 8
    USART2->BRR = 0x113;             //sets baudrate for 115200 bps for fpclk = 16MHz (17.375)

    USART2->CR1 |= USART_CR1_TE;     //enable transmission
    USART2->CR1 |= USART_CR1_RE;     //enable reception

    USART2->CR1 |= USART_CR1_RXNEIE;  //enable recieve interrupt (required for asynchronous reading)

    NVIC_EnableIRQ(USART2_IRQn);

    USART2->CR1 |= USART_CR1_UE;     //enable uart
}

#endif  //and thats all my friends