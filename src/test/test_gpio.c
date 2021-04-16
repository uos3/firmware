/**
 * @file test_gpio.c
 * @author Leon Galanakis (leongalanakis@gmail.com)
 * @brief GPIO test
 * 
 * 
 * 
 * @version 0.1
 * @date 2021-03-16
 * 
 * @copyright Copyright (c) 2021
 */

/* -------------------------------------------------------------------------   
 * INCLUDES
 * ------------------------------------------------------------------------- */

/* Standard includes */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* Internal includes */
#include "drivers/gpio/Gpio_public.h"
#include "drivers/uart/Uart_public.h"
#include "drivers/udma/Udma_public.h"
#include "components/led/Led_public.h"
#include "util/debug/Debug_public.h"
#include "system/kernel/Kernel_public.h"
#include "system/event_manager/EventManager_public.h"

/* -------------------------------------------------------------------------   
 * MAIN
 * ------------------------------------------------------------------------- */

int main(void) {
    int toggle_count;

    toggle_count = 0;

    Debug_exit(1);

    Kernel_init_critical_modules();

    DEBUG_INF("GPIO Test");

    if (Uart_init_specific(UART_DEVICE_ID_TEST) != ERROR_NONE) {
        Debug_exit(1);
    }

    if (Gpio_init(GPIO_PINF1, 1, GPIO_MODE_OUTPUT) != ERROR_NONE) {
        Debug_exit(1);
    }
    if (Gpio_init(GPIO_PINF0, 1, GPIO_MODE_INPUT) != ERROR_NONE) {
        Debug_exit(1);
    }

    Led_set(LED_LAUNCHPAD, true);

    Debug_exit(1);

    #if 0
    /* Initialise the LED. */
    if (Gpio_init((int *)GPIO_PINF1, 1, GPIO_MODE_OUTPUT) != ERROR_NONE) {
        DEBUG_ERR("Failed to initialise LED GPIO pin");
        Debug_exit(1);
        return 1;
    }
    else {
        DEBUG_INF("LED initialised");
    }

    /* Initialise the switch input GPIO pin. */
    if (Gpio_init(GPIO_PINF0, 1, GPIO_MODE_INPUT) != ERROR_NONE) {
        DEBUG_ERR("Failed to initialise switch GPIO pin");
        Debug_exit(1);
        return 1;
    }
    else {
        DEBUG_INF("Switch initialised");
    }
    #endif

    /* Return 0 if no errors occured up to this point. */
    return 0;
}
