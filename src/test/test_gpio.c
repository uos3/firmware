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
#include "drivers/gpio/Gpio_errors.h"
#include "drivers/gpio/Gpio_private.h"
#include "drivers/gpio/Gpio_public.h"
#include "components/led/Led_errors.h"
#include "components/led/Led_private.h"
#include "components/led/Led_public.h"
#include "util/debug/Debug_public.h"

/* -------------------------------------------------------------------------   
 * MAIN
 * ------------------------------------------------------------------------- */

int main(void) {
    /* TODO: Unfinished */

    /* Initialise the LED. */
    if (Gpio_init(LED_LEDS[LED_TEMP_1].gpio_pin, 1, GPIO_MODE_INPUT) != ERROR_NONE) {
        DEBUG_ERR("Failed to initialise GPIO pin");
    }

    /* Set the rising interrupt to toggle the LED state when an interrupt is
     * detected. */
    if (Gpio_set_rising_interrupt(LED_LEDS[LED_TEMP_1].gpio_pin, Led_toggle(LED_LEDS[LED_TEMP_1].gpio_pin)) != ERROR_NONE) {
        DEBUG_ERR("Failed to set rising interrupt on GPIO pin");
    }

    while (1) {
        /* Keep looping, so that the switch on the tm4c launchpad can be used
         * at any time to set the rising interrupt, and the LED should toggle
         * with each press of the switch. */
    }

    /* Return 0 if no errors occured up to this point. */
    return 0;
}