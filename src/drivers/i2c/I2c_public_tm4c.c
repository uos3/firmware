/**
 * @ingroup i2c
 * 
 * @file I2c_public_tm4c.h
 * @author Duncan Hamill (dh2g16@soton.ac.uk/duncanrhamill@googlemail.com)
 *         Leon Galanakis (lg5g16@soton.ac.uk)
 * @brief I2C driver for the firmware.
 * 
 * This file implements the I2C driver for TM4C targets.
 * 
 * See I2c_public.h for more information.
 * 
 * @version 0.1
 * @date 2020-11-11
 * 
 * @copyright Copyright (c) 2020
 */

/* -------------------------------------------------------------------------   
 * INCLUDES
 * ------------------------------------------------------------------------- */

/* Standard library includes */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* External includes */
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/i2c.h"
#include "driverlib/gpio.h"

#include "inc/hw_memmap.h"

/* Internal includes */
#include "util/debug/Debug_public.h"
#include "system/event_manager/EventManager_public.h"
#include "drivers/i2c/I2c_public.h"
#include "drivers/i2c/I2c_private.h"

/* -------------------------------------------------------------------------   
 * FUNCTIONS
 * ------------------------------------------------------------------------- */

ErrorCode I2c_init(void) {

    /* Loop through each module */
    for (size_t i = 0; i < I2C_NUM_ENABLED_MODULES; ++i) {

        /* Get a reference to the module to init */
        I2c_Module *p_module = &I2C_MODULES[I2C_ENABLED_MODULES[i]];

        /* If the module is already initialised issue warning but don't do
         * anything 
         */
        if (p_module->initialised) {
            DEBUG_WRN(
                "I2c_init() called on module %d when already initialised.", 
                p_modules_in[i]
            );

            /* Go to the next module in the list */
            continue;
        }
        
        /* If the peripheral is not ready reset and enable it
         * TODO: The reset here in the old code reset the GPIO, this should
         * probably be I2C instead as GPIO is handled later, so just check this
         * is correct.
         */
        if (!SysCtlPeripheralReady(p_module->peripheral_i2c)) {
            SysCtlPeripheralReset(p_module->peripheral_i2c);
            SysCtlPeripheralEnable(p_module->peripheral_i2c);

            /* Attempt to initialise the I2C peripheral */
            bool enabled = false; 
            for (
                int attempt = 0;
                attempt < I2C_MAX_NUM_I2C_PERIPH_READY_CHECKS; 
                ++attempt
            ) {
                
                if (SysCtlPeripheralReady(p_module->peripheral_i2c)) {
                    enabled = true;
                    break;
                }
            }

            /* If the peripheral wasn't enabled before the max attempts was
             * reached return an error 
             */
            if (!enabled) {
                /* TODO: Mark the module as disabled? */
                DEBUG_ERR("Could not enable I2C module %d", p_modules_in[i]);
                return I2C_ERROR_I2C_PERIPH_ENABLE_FAILED;
            }
        }

        /* If the peripheral is not ready reset and enable it */
        if (!SysCtlPeripheralReady(p_module->peripheral_gpio)) {
            SysCtlPeripheralReset(p_module->peripheral_gpio);
            SysCtlPeripheralEnable(p_module->peripheral_gpio);

            /* Attempt to initialise the GPIO peripheral */
            bool enabled = false; 
            for (
                int attempt = 0;
                attempt < I2C_MAX_NUM_GPIO_PERIPH_READY_CHECKS; 
                ++attempt
            ) {
                
                if (SysCtlPeripheralReady(p_module->peripheral_gpio)) {
                    enabled = true;
                    break;
                }
            }

            /* If the peripheral wasn't enabled before the max attempts was
             * reached return an error 
             */
            if (!enabled) {
                /* TODO: Mark the module as disabled? */
                DEBUG_ERR(
                    "Could not enable GPIO module for I2C module %d", 
                    p_modules_in[i]
                );
                return I2C_ERROR_GPIO_PERIPH_ENABLE_FAILED;
            }
        }

        /* Configure the GPIO pins for their I2C functions */
        GPIOPinConfigure(p_module->pin_scl_function);
        GPIOPinConfigure(p_module->pin_sda_function);

        /* Assign pins to the I2C peripheral */
        GPIOPinTypeI2CSCL(p_module->base_gpio, p_module->pin_scl);
        GPIOPinTypeI2C(p_module->base_gpio, p_module->pin_sda);

        /* Set the I2C peripheral clock rate */
        I2CMasterInitExpClk(
            p_module->base_i2c, 
            SysCtlClockGet(), 
            p_module->full_speed
        );

        /* Enable the master mode */
        I2CMasterEnable(p_module->base_i2c);

        /* 
         * Note: In the previous code a FIFO flush was done here, however the
         * TM4C123s have no FIFOs on the I2C, so this was removed. 
         */

        /* Set the module to be initialised */
        p_module->initialised = true;
    }

    /* Mark the I2C driver as initialised */
    I2C.initialised = true;

    /* Return success */
    return ERROR_NONE;
}

ErrorCode I2c_step(void) {
    /* 
     * In the step we poll for an I2C_NEW_ACTION event, and iterate through 
     * each action and call the appropriate function.
     */

    /* Check the I2C is initialised */
    if (!I2C.initialised) {
        DEBUG_ERR("Attempted to step I2C when it is not initialised.");
        return I2C_ERROR_NOT_INITIALISED;
    }

    /* Poll for new action event. We actually don't care if this is raised or
     * not since actions can be waiting for interrupts to happen, so we check
     * need to step their functions each time. The reason the new action event
     * is raised at all is that a raised event at the end of the cycle will
     * trigger the next cycle to be run immediately, rather than putting the
     * system to sleep. */
    bool new_action = false;
    if (!EventManager_poll_event(EVT_I2C_NEW_ACTION, &new_action)) {
        DEBUG_ERR("Could not poll for EVT_I2C_NEW_ACTION event");
        return I2C_ERROR_EVENTMANAGER_ERROR;
    }

    /* Loop through all actions and run the ones that are not NONE */
    for (size_t i = 0; i < I2C_MAX_NUM_ACTIONS; ++i) {
        /* Get the action type */
        I2c_ActionType type = I2C.action_types[i];

        /* Error return code for the stepped action functions */
        ErrorCode error = ERROR_NONE;

        switch(type) {
            case I2C_ACTION_TYPE_SINGLE_SEND:
                /* Call single send function */
                error = I2c_action_single_send(&I2C.u_actions[i].single_send);

                /* If there was an error log where it came from and return it */
                if (error != ERROR_NONE) {
                    DEBUG_ERR(
                        "Error performing single send action for device (%d, %d)",
                        I2C.u_actions[i].single_send.device.module,
                        I2C.u_actions[i].single_send.device.address
                    );
                    return error;
                }
                break;

            case I2C_ACTION_TYPE_SINGLE_RECV:
                /* Call single recv function */
                error = I2c_action_single_recv(&I2C.u_actions[i].single_recv);

                /* If there was an error log where it came from and return it */
                if (error != ERROR_NONE) {
                    DEBUG_ERR(
                        "Error performing single receive action for device (%d, %d)",
                        I2C.u_actions[i].single_recv.device.module,
                        I2C.u_actions[i].single_recv.device.address
                    );
                    return error;
                }
                break;

            case I2C_ACTION_TYPE_BURST_SEND:
                /* Call burst send function */
                error = I2c_action_burst_send(&I2C.u_actions[i].burst_send);

                /* If there was an error log where it came from and return it */
                if (error != ERROR_NONE) {
                    DEBUG_ERR(
                        "Error performing burst send action for device (%d, %d)",
                        I2C.u_actions[i].burst_send.device.module,
                        I2C.u_actions[i].burst_send.device.address
                    );
                    return error;
                }
                break;

            case I2C_ACTION_TYPE_BURST_RECV:
                /* Call burst recv function */
                error = I2c_action_burst_recv(&I2C.u_actions[i].burst_recv);

                /* If there was an error log where it came from and return it */
                if (error != ERROR_NONE) {
                    DEBUG_ERR(
                        "Error performing burst receive action for device (%d, %d)",
                        I2C.u_actions[i].burst_recv.device.module,
                        I2C.u_actions[i].burst_recv.device.address
                    );
                    return error;
                }
                break;

            case I2C_ACTION_TYPE_NONE:
                /* Nothing to do for no action */
                break;
            default:
                /* Log an unregocnised action */
                DEBUG_WRN("Unrecognised I2c_ActionType: %d", type);
                break;
        }
    }

    return ERROR_NONE;
}

ErrorCode I2c_device_send_bytes(
    I2c_Device *p_device_in,
    uint8_t *p_data_in,
    size_t length_in
) {
    /* Check the I2C is initialised */
    if (!I2C.initialised) {
        DEBUG_ERR(
            "Attempted to send byte(s) to I2C when it is not initialised."
        );
        return I2C_ERROR_NOT_INITIALISED;
    }

    /* Attempt to get the module lock */
    if (I2c_lock_module(p_device_in) != ERROR_NONE) {
        return I2C_ERROR_MODULE_LOCKED_BY_ANOTHER_DEVICE;
    }

    /* Raise an error if the length of the data is zero, as we cannot
     * send zero bytes. */
    if (length_in == 0) {
        DEBUG_ERR("Attempted to send zero bytes.");
        return I2C_ERROR_ZERO_LENGTH_SEND;
    }

    /* Will be true if an empty space in the action array is found and the
     * action was queued for execution. */
    bool action_queued = false;

    /* Loop through searching for an empty action in the list */
    for (int i = 0; i < I2C_MAX_NUM_ACTIONS; ++i) {
        if (I2C.action_types[i] == I2C_ACTION_TYPE_NONE) {
            /* Send bytes in single byte mode if length_in == 1 */
            if (length_in == 1) {
                I2C.action_types[i] = I2C_ACTION_TYPE_SINGLE_SEND;
                I2C.u_actions[i].single_send.step = 0;
                I2C.u_actions[i].single_send.error = ERROR_NONE;
                I2C.u_actions[i].single_send.status
                    = I2C_ACTION_STATUS_IN_PROGRESS;
                I2C.u_actions[i].single_send.device = *p_device_in;
                I2C.u_actions[i].single_send.byte = *p_data_in;
                I2C.u_actions[i].single_send.num_master_busy_major_checks = 0;
            }

            /* Send bytes in multi byte mode if length_in > 1 */
            else {
                I2C.action_types[i] = I2C_ACTION_TYPE_BURST_SEND;
                I2C.u_actions[i].burst_send.step = 0;
                I2C.u_actions[i].burst_send.error = ERROR_NONE;
                I2C.u_actions[i].burst_send.status
                    = I2C_ACTION_STATUS_IN_PROGRESS;
                I2C.u_actions[i].burst_send.device = *p_device_in;
                I2C.u_actions[i].burst_send.length = length_in;

                /* Allocate memory for the bytes */
                I2C.u_actions[i].burst_send.p_bytes 
                    = (uint8_t *)malloc(length_in);

                /* Check that memory allocation was successful */
                if (I2C.u_actions[i].burst_send.p_bytes == NULL) {
                    DEBUG_ERR(
                        "Failed to allocate memory for burst send action."
                    );
                    return I2C_ERROR_MEMORY_ALLOC_FOR_SEND_FAILED;
                }

                /* Copy the input data into the action structure */
                memcpy(
                    (void *)I2C.u_actions[i].burst_send.p_bytes,
                    (void *)p_data_in,
                    length_in
                );
            }

            /* Set to true if an empty space is found in the action array,
             * allowing an action to be queued */
            action_queued = true;

            break;
        }
    }

    #ifdef DEBUG_MODE
    /* Alloc string to print bytes in hex, 2 chars ber pyte, + spaces, + null */
    char *p_hex_string = (char *)malloc(sizeof(char) * 3 * length_in);
    char buf[4] = {0};

    /* Print bytes except last into the string */
    for (int i = 0; i < length_in - 1; ++i) {
        sprintf((char *)buf, "%02X ", p_data_in[i]);
        strcat(p_hex_string, (char *)buf);
    }

    sprintf((char *)buf, "%02X", p_data_in[length_in - 1]);
    strcat(p_hex_string, (char *)buf);

    DEBUG_DBG(
        "I2C send (%02X, %02X): %s", 
        p_device_in->module, 
        p_device_in->address,
        p_hex_string
    );

    free(p_hex_string);
    #endif

    /* If the action was not queued raise an error */
    if (!action_queued) {
        DEBUG_ERR(
            "Failed to queue a send action as the maximum number of actions\
             has been reached."
        );
        return I2C_ERROR_MAX_ACTIONS_REACHED;
    }

    /* The action has been queued, raise the new action event */
    if (!EventManager_raise_event(EVT_I2C_NEW_ACTION)) {
        DEBUG_ERR("Could not raise EVT_I2C_NEW_ACTION");
        return I2C_ERROR_EVENTMANAGER_ERROR;
    }

    return ERROR_NONE;
}

ErrorCode I2c_device_recv_bytes(
    I2c_Device *p_device_in,
    uint8_t reg_in,
    size_t length_in
) {
    /* Check the I2C is initialised */
    if (!I2C.initialised) {
        DEBUG_ERR(
            "Attempted to send byte(s) to I2C when it is not initialised."
        );
        return I2C_ERROR_NOT_INITIALISED;
    }

    /* Attempt to get the module lock */
    if (I2c_lock_module(p_device_in) != ERROR_NONE) {
        return I2C_ERROR_MODULE_LOCKED_BY_ANOTHER_DEVICE;
    }

    /* Raise an error if the length of the data is zero, as we cannot
     * receive zero bytes. */
    if (length_in == 0) {
        DEBUG_ERR("Attempted to receive zero bytes.");
        return I2C_ERROR_ZERO_LENGTH_RECEIVE;
    }

    /* Will be true if an empty space in the action array is found and the
     * action was queued for execution. */
    bool action_queued = false;

    /* Loop through searching for an empty action in the list */
    for (int i = 0; i < I2C_MAX_NUM_ACTIONS; ++i) {
        if (I2C.action_types[i] == I2C_ACTION_TYPE_NONE) {
            /* Receive bytes in single byte mode if length_in == 1 */
            if (length_in == 1) {
                I2C.action_types[i] = I2C_ACTION_TYPE_SINGLE_RECV;
                I2C.u_actions[i].single_recv.step = 0;
                I2C.u_actions[i].single_recv.error = ERROR_NONE;
                I2C.u_actions[i].single_recv.status
                    = I2C_ACTION_STATUS_IN_PROGRESS;
                I2C.u_actions[i].single_recv.device = *p_device_in;
                I2C.u_actions[i].single_recv.reg = reg_in;
            }

            /* Send bytes in multi byte mode if length_in > 1 */
            else {
                I2C.action_types[i] = I2C_ACTION_TYPE_BURST_RECV;
                I2C.u_actions[i].burst_recv.step = 0;
                I2C.u_actions[i].burst_recv.error = ERROR_NONE;
                I2C.u_actions[i].burst_recv.status
                    = I2C_ACTION_STATUS_IN_PROGRESS;
                I2C.u_actions[i].burst_recv.device = *p_device_in;
                I2C.u_actions[i].burst_recv.reg = reg_in;
                I2C.u_actions[i].burst_recv.length = length_in;

                /* Allocate memory for the bytes */
                I2C.u_actions[i].burst_recv.p_bytes 
                    = (uint8_t *)malloc(length_in);

                /* Check that memory allocation was successful */
                if (I2C.u_actions[i].burst_recv.p_bytes == NULL) {
                    DEBUG_ERR(
                        "Failed to allocate memory for burst receive action."
                    );
                    return I2C_ERROR_MEMORY_ALLOC_FOR_RECV_FAILED;
                }
            }

            /* Set to true if an empty space is found in the action array,
             * allowing an action to be queued */
            action_queued = true;

            break;
        }
    }

    /* If the action was not queued raise an error */
    if (!action_queued) {
        DEBUG_ERR(
            "Failed to queue a receive action as the maximum number of actions\
             has been reached."
        );
        return I2C_ERROR_MAX_ACTIONS_REACHED;
    }

    /* The action has been queued, raise the new action event */
    if (!EventManager_raise_event(EVT_I2C_NEW_ACTION)) {
        DEBUG_ERR("Could not raise EVT_I2C_NEW_ACTION");
        return I2C_ERROR_EVENTMANAGER_ERROR;
    }

    return ERROR_NONE;
}

ErrorCode I2c_get_device_recved_bytes(
    I2c_Device *p_device_in,
    uint8_t *p_bytes_out
) {
    /* Loop through the actions that are not NONE and find this device */
    for (size_t i = 0; i < I2C_MAX_NUM_ACTIONS; ++i) {
        /* Get the action type */
        I2c_ActionType type = I2C.action_types[i];

        switch (type) {
            case I2C_ACTION_TYPE_SINGLE_SEND: ;
            case I2C_ACTION_TYPE_BURST_SEND: ;
                /* Get the device for this action */
                I2c_Device *p_action_device;
                if (type == I2C_ACTION_TYPE_SINGLE_SEND) {
                    p_action_device = &I2C.u_actions[i].single_send.device;
                }
                else if (type == I2C_ACTION_TYPE_BURST_SEND) {
                    p_action_device = &I2C.u_actions[i].burst_send.device;
                }
                /* No need to check for other types as the contianing switch
                 * statement will prevent other types getting to this point. */

                /* If the devices are the same */
                if (I2c_devices_equal(p_action_device, p_device_in)) {
                    /* Return an error since we can't recieve bytes on a send
                     * action */
                    DEBUG_ERR(
                        "Attempted to get recieved bytes from a send action on\
                         device (%d, %d).",
                        p_device_in->module,
                        p_device_in->address
                    );
                    return I2C_ERROR_GET_RECVED_BYTES_FROM_SEND;
                }
                break;
            case I2C_ACTION_TYPE_SINGLE_RECV: ;
                /* Have to use sr_action here as there's also burst recieve,
                 * and switch cases do not have their own scope so redeclaring 
                 * a variable with the same name is not allowed */
                I2c_ActionSingleRecv *p_sr_action 
                    = &I2C.u_actions[i].single_recv;

                /* If the devices are the same */
                if (I2c_devices_equal(&p_sr_action->device, p_device_in)) {
                    /* If the status is success get the byte */
                    if (p_sr_action->status == I2C_ACTION_STATUS_SUCCESS) {
                        p_bytes_out[0] = p_sr_action->byte;
                        return ERROR_NONE;
                    }
                    /* If the status is not success return an error */
                    else {
                        DEBUG_ERR(
                            "Tried to get the recieved bytes on a non\
                            successful action for device (%d, %d)",
                            p_device_in->module,
                            p_device_in->address
                        );
                        return I2C_ERROR_GET_RECVED_BYTES_ON_NON_SUCCESS_ACTION;
                    }
                }
                break;
            case I2C_ACTION_TYPE_BURST_RECV: ;
                /* Have to use br_action here as there's also single recieve,
                 * and switch cases do not have their own scope so redeclaring 
                 * a variable with the same name is not allowed */
                I2c_ActionBurstRecv *p_br_action 
                    = &I2C.u_actions[i].burst_recv;

                /* If the devices are the same */
                if (I2c_devices_equal(&p_br_action->device, p_device_in)) {
                    /* If the status is success get the byte */
                    if (p_br_action->status == I2C_ACTION_STATUS_SUCCESS) {
                        /* 
                         * ---- NUMERICAL PROTECTION ----
                         * No check is made here to ensure that the output
                         * pointer can hold the number of bytes stored in the
                         * action. It is up to the user to ensure that they
                         * allocate enough space for the number of bytes they
                         * requested.
                         */
                        memcpy(
                            (void *)(p_bytes_out), 
                            (void *)(p_br_action->p_bytes),
                            p_br_action->length
                        );
                        return ERROR_NONE;
                    }
                    /* If the status is not success return an error */
                    else {
                        DEBUG_ERR(
                            "Tried to get the recieved bytes on a non\
                             successful action for device (%d, %d)",
                            p_device_in->module,
                            p_device_in->address
                        );
                        return I2C_ERROR_GET_RECVED_BYTES_ON_NON_SUCCESS_ACTION;
                    }
                }
                break;
            case I2C_ACTION_TYPE_NONE:
                /* Nothing to do for no action */
                break;
            default:
                /* Log an unregocnised action */
                DEBUG_WRN("Unrecognised I2c_ActionType: %d", type);
                break;
        }
    }

    /* If we have got to this point with no device found return an error */
    DEBUG_ERR(
        "Device (%d, %d) not found in the list of I2C actions.",
        p_device_in->module,
        p_device_in->address    
    );
    return I2C_ERROR_NO_ACTION_FOR_DEVICE;
}

ErrorCode I2c_get_device_action_failure_cause(
    I2c_Device *p_device_in,
    ErrorCode *p_error_out
) {
    /* Loop through the actions that are not NONE and find this device */
    for (size_t i = 0; i < I2C_MAX_NUM_ACTIONS; ++i) {
        /* Get the action type */
        I2c_ActionType type = I2C.action_types[i];
        I2c_Device *p_action_device = NULL;
        ErrorCode *p_error = NULL;

        switch (type) {
            case I2C_ACTION_TYPE_SINGLE_SEND:
                /* Get pointers to the device and error code for this action */
                p_action_device = &I2C.u_actions[i].single_send.device;
                p_error = &I2C.u_actions[i].single_send.error;
                break;
            case I2C_ACTION_TYPE_SINGLE_RECV:
                /* Get pointers to the device and error code for this action */
                p_action_device = &I2C.u_actions[i].single_recv.device;
                p_error = &I2C.u_actions[i].single_recv.error;
                break;
            case I2C_ACTION_TYPE_BURST_SEND:
                /* Get pointers to the device and error code for this action */
                p_action_device = &I2C.u_actions[i].burst_send.device;
                p_error = &I2C.u_actions[i].burst_send.error;
                break;
            case I2C_ACTION_TYPE_BURST_RECV:
                /* Get pointers to the device and error code for this action */
                p_action_device = &I2C.u_actions[i].burst_recv.device;
                p_error = &I2C.u_actions[i].burst_recv.error;
                break;
            case I2C_ACTION_TYPE_NONE:
                /* Nothing to do for no action */
                break;
            default:
                /* Log an unregocnised action */
                DEBUG_WRN("Unrecognised I2c_ActionType: %d", type);
                break;
        }

        /* If device is null skip to the next one (empty action) */
        if (p_action_device == NULL) {
            continue;
        }

        /* Check the device code */
        if (I2c_devices_equal(p_action_device, p_device_in)) {
            /* Set the error code and return */
            *p_error_out = *p_error;
            return ERROR_NONE;
        }
        /* If devices not equal continue to the next action */
    }

    /* If we have got to this point with no device found return an error */
    DEBUG_ERR("Device (%d, %d) not found in the list of I2C actions.");
    return I2C_ERROR_NO_ACTION_FOR_DEVICE;
}

ErrorCode I2c_get_device_action_status(
    I2c_Device *p_device_in,
    I2c_ActionStatus *p_status_out
) {
    
    /* Check the I2C is initialised */
    if (!I2C.initialised) {
        DEBUG_ERR(
            "Attempted to get device action status when I2C has not been\
             initialised."
        );
        return I2C_ERROR_NOT_INITIALISED;
    }
    
    /* Loop through all actions which are not NONE */
    for (size_t i = 0; i < I2C_MAX_NUM_ACTIONS; ++i) {
        
        /* Get the action type */
        I2c_ActionType type = I2C.action_types[i];
        
        /* Initialise pointers */
        I2c_Device *p_action_device = NULL;
        I2c_ActionStatus *p_status = NULL;

        switch(type) {
            case I2C_ACTION_TYPE_SINGLE_SEND:
                /* Get pointers for the device and associated action status
                 * for the single send action. */
                p_action_device = &I2C.u_actions[i].single_send.device;
                p_status = &I2C.u_actions[i].single_send.status;
                break;
            case I2C_ACTION_TYPE_BURST_SEND:
                /* Get pointers for the device and associated action status
                 * for the burst send action. */
                p_action_device = &I2C.u_actions[i].burst_send.device;
                p_status = &I2C.u_actions[i].burst_send.status;
                break;
            case I2C_ACTION_TYPE_SINGLE_RECV:
                /* Get pointers for the device and associated action status
                 * for the single receive action. */
                p_action_device = &I2C.u_actions[i].single_recv.device;
                p_status = &I2C.u_actions[i].single_recv.status;
                break;
            case I2C_ACTION_TYPE_BURST_RECV:
                /* Get pointers for the device and associated action status
                 * for the burst receive action. */
                p_action_device = &I2C.u_actions[i].burst_recv.device;
                p_status = &I2C.u_actions[i].burst_recv.status;
                break;
            case I2C_ACTION_TYPE_NONE:
                /* If there is no action, there is no associated status, so
                 * no need to do anything. */
                break;
            default:
                /* If the action type is anything other than the above listed
                 * action types, return an error. */
                DEBUG_ERR("Unexpected action type: %d", type);
                return I2C_ERROR_NO_ACTION_FOR_DEVICE;
                break;
        }

        /* If either pointer is null the action is none so skip to next 
         * action */
        if (p_action_device == NULL || p_status == NULL) {
            continue;
        }

        /* Check the device code */
        if (I2c_devices_equal(p_action_device, p_device_in)) {
            /* Set the status code and return */
            *p_status_out = *p_status;
            return ERROR_NONE;
        }
        /* If devices are not equal, continue */
    }

    /* If the loop has completed without a device found, give an error */
    DEBUG_ERR(
        "Device (%d, %d) not found in the list of I2C actions.",
        p_device_in->address,
        p_device_in->module
    );
    return I2C_ERROR_NO_ACTION_FOR_DEVICE;
}

ErrorCode I2c_clear_device_action(I2c_Device *p_device_in) {
    
    /* Check the I2C is initialised */
    if (!I2C.initialised) {
        DEBUG_ERR(
            "Attempted to clear device action when I2C has not been\
             initialised."
        );
        return I2C_ERROR_NOT_INITIALISED;
    }

    /* Loop through the actions */
    for (size_t i = 0; i < I2C_MAX_NUM_ACTIONS; ++i) {
        I2c_ActionType type = I2C.action_types[i];
        I2c_Device *p_action_device = NULL;

        switch(type) {
            case I2C_ACTION_TYPE_SINGLE_SEND:
                /* Check the device */
                p_action_device = &I2C.u_actions[i].single_send.device;
                
                break;

            case I2C_ACTION_TYPE_BURST_SEND:
                /* Check the device */
                p_action_device = &I2C.u_actions[i].burst_send.device;

                break;

            case I2C_ACTION_TYPE_SINGLE_RECV:
                /* Check the device */
                p_action_device = &I2C.u_actions[i].single_recv.device;

                break;
                
            case I2C_ACTION_TYPE_BURST_RECV:
                /* Check the device */
                p_action_device = &I2C.u_actions[i].burst_recv.device;

                break;
            
            case I2C_ACTION_TYPE_NONE:
                /* If there is no action, there is no associated status, so
                 * no need to do anything. */
                break;

            default:
                /* If the action type is anything other than the above listed
                 * action types, return an error. */
                DEBUG_ERR("Unexpected action type: %d", type);
                return I2C_ERROR_NO_ACTION_FOR_DEVICE;
                break;
        }

        /* If the device wasn't found go to the next action */
        if (p_action_device == NULL) {
            continue;
        }

        /* Check the device code */
        if (I2c_devices_equal(p_action_device, p_device_in)) {

            /* Set the data in the action to zero, effectively clearing it */
            memset(
                &I2C.u_actions[i],
                0,
                sizeof(I2c_Action)
            );

            /* Set the action type to none, again clearing it */
            I2C.action_types[i] = I2C_ACTION_TYPE_NONE;

            /* Unlock the bus */
            I2C.module_locked[p_device_in->module] = false;

            return ERROR_NONE;
        }
        /* If devices are not equal, continue */
    }

    /* If the loop has completed without a device found, give an error */
    DEBUG_ERR(
        "Device (%d, %d) not found in the list of I2C actions.",
        p_device_in->address,
        p_device_in->module
    );
    return I2C_ERROR_NO_ACTION_FOR_DEVICE;
}