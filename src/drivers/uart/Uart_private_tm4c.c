/**
 * @ingroup uart
 * 
 * @file Uart_private_tm4c.c
 * @author Leon Galanakis (lg5g16@soton.ac.uk)
 * @brief Private UART driver for the firmware.
 * 
 * Implements the UART driver for the TM4C.
 * 
 * @version 0.1
 * @date 2021-02-16
 * 
 * @copyright Copyright (c) 2021
 */

/* -------------------------------------------------------------------------   
 * INCLUDES
 * ------------------------------------------------------------------------- */

/* Standard Lirary */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Internal */
#include "drivers/uart/Uart_public.h"
#include "drivers/uart/Uart_private.h"
#include "drivers/uart/Uart_errors.h"
#include "drivers/gpio/Gpio_public.h"

/* TI Library Includes */
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "driverlib/udma.h"

/* -------------------------------------------------------------------------   
 * GLOBALS
 * ------------------------------------------------------------------------- */

Uart_Device UART_DEVICES[UART_NUM_UARTS] = {
    {
        /* UART 1 (GNSS)
         * TODO: change this to "UART ID 0 (GNSS, UART0)"
         */
        SYSCTL_PERIPH_GPIOA,
        SYSCTL_PERIPH_UART0,
        GPIO_PORTA_BASE,
        UART0_BASE,
        GPIO_PA0_U0RX, /* TODO: Getting same error as I2C. Should still work */
        GPIO_PA1_U0TX,
        GPIO_PIN_0,
        GPIO_PIN_1,
        UDMA_CHANNEL_UART0TX,
        UDMA_CHANNEL_UART0RX,
        0x000,
        UDMA_MODE_STOP,
        false
    },

    {
        /* UART 2 (CAM) */
        SYSCTL_PERIPH_GPIOC,
        SYSCTL_PERIPH_UART3,
        GPIO_PORTC_BASE,
        UART3_BASE,
        GPIO_PC6_U3RX, /* TODO: Getting same error as I2C. Should still work */
        GPIO_PC7_U3TX,
        GPIO_PIN_6,
        GPIO_PIN_7,
        UDMA_CHANNEL_UART1TX,
        UDMA_CHANNEL_UART1RX,
        0x000,
        UDMA_MODE_STOP,
        false
    },

    {
        /* UART 3 (EPS) */
        SYSCTL_PERIPH_GPIOE,
        SYSCTL_PERIPH_UART7,
        GPIO_PORTE_BASE,
        UART7_BASE,
        GPIO_PE0_U7RX, /* TODO: Getting same error as I2C. Should still work */
        GPIO_PE1_U7TX,
        GPIO_PIN_0,
        GPIO_PIN_1,
        UDMA_CH1_UART2TX, /* TODO: Check TI channel assignments */
        UDMA_CH0_UART2RX,
        0x000,
        UDMA_MODE_STOP,
        false
    },

    {
        /* UART 4 (TEST) */
        SYSCTL_PERIPH_GPIOB,
        SYSCTL_PERIPH_UART0,
        GPIO_PORTB_BASE,
        UART0_BASE,
        GPIO_PCTL_PB0_U1RX,
        GPIO_PCTL_PB1_U1TX,
        GPIO_PINB0, /* use tivaware gpio not driver */
        GPIO_PINB1,
        UDMA_CHANNEL_UART0TX, /* TODO: Check TI channel assignments */
        UDMA_CHANNEL_UART0RX,
        0x000,
        UDMA_MODE_STOP,
        false
    },
};

/* -------------------------------------------------------------------------   
 * FUNCTIONS
 * ------------------------------------------------------------------------- */