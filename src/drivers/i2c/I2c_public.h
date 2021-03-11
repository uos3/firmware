/**
 * @ingroup drivers
 * 
 * @file I2c_public.h
 * @author Duncan Hamill (dh2g16@soton.ac.uk/duncanrhamill@googlemail.com)
 *         Leon Galanakis (lg5g16@soton.ac.uk)
 * @brief I2C driver for the firmware.
 * 
 * Task ref: [UT_2.8.7]
 * 
 * I2C, like all other IO drivers, uses an internal state rather than the
 * DataPool for storing data. Users of the I2C driver are responsible for
 * handling I2C errors (either themselves or by propagating to FDIR system).
 * This is because many hardware endpoints can be connected to the I2C bus, so
 * errors in those endpoints must be handled by the component responsible for
 * them. The I2C driver itself won't understand what action can be taken if the
 * IMU doesn't respond, for example.
 * 
 * The I2C driver works with the concept of actions being performed on devices.
 * A device represents a slave connected to one of the available I2C modules,
 * of which on the TM4C123GPHM there are 4. Many devices can be connected to
 * the same module, so each device also has a separate address. While an action
 * is being performed on a device that device locks the module, to prevent
 * other devices interrupting the communications of the locking device.
 * 
 * Actions are raised and performed asynchronously from the point of view of
 * the user. The user first calls `I2c_init()` with the list of modules to
 * activate, and can then use either `I2c_device_send_bytes()` or
 * `I2c_device_recv_bytes()` to perform actions on the device. The user then
 * must poll the `EVT_I2C_ACTION_FINISHED` event, which occurs at least one
 * cycle later. Once the action is finished the status of the action shall be
 * read using `I2c_get_device_action_status()`, which will either be
 * `I2C_ACTION_STATUS_SUCCESS` or `I2C_ACTION_STATUS_FAILURE`.  
 * 
 * The root cause of a failed action may be found using
 * `I2c_get_device_action_failure_cause()`. The bytes read by a successful 
 * `recv` action can be retrieved using `I2c_get_device_recved_bytes()`.
 * 
 * Finally the user must call `I2c_clear_device_action()` once all necessary
 * data has been retrieved by the user. Failure to do so will result in the
 * I2C module associated with that device remaining locked, and therefore
 * unusable by that device or others. 
 * 
 * @version 0.1
 * @date 2020-11-09
 * 
 * @copyright Copyright (c) 2020
 * 
 * @defgroup i2c I2C
 * @{
 */

#ifndef H_I2C_PUBLIC_H
#define H_I2C_PUBLIC_H

/* -------------------------------------------------------------------------   
 * INCLUDES
 * ------------------------------------------------------------------------- */

/* Standard library includes */
#include <stdlib.h>
#include <stdint.h>

/* Internal includes */
#include "drivers/i2c/I2c_errors.h"

/* -------------------------------------------------------------------------   
 * ENUMS
 * ------------------------------------------------------------------------- */

/**
 * @brief The possible status of an action being performed by the I2C driver.
 * 
 */
typedef enum _I2c_ActionStatus {

    I2C_ACTION_STATUS_NO_ACTION = 0,
    
    I2C_ACTION_STATUS_IN_PROGRESS,

    I2C_ACTION_STATUS_SUCCESS,

    I2C_ACTION_STATUS_FAILURE

} I2c_ActionStatus;

/* -------------------------------------------------------------------------   
 * STRUCTS
 * ------------------------------------------------------------------------- */

/**
 * @brief Represents a single device connected to an I2C bus of the TM4C.
 */
typedef struct _I2c_Device {

    /**
     * @brief The TM4C's I2C module number for this device.
     * 
     * This is effectively the I2C bus as each module is assigned a different
     * set of output pins. Must be between 0 and 3.
     * 
     * Note: This is different from the concept of a software module, and is 
     *       used because this is the terminology in the TM4C manual.
     */
    uint32_t module;

    /**
     * @brief The address of the device on the I2C bus.
     */
    uint8_t address;
} I2c_Device;

/* -------------------------------------------------------------------------   
 * FUNCTIONS
 * ------------------------------------------------------------------------- */

/**
 * @brief Initialise the I2C driver.
 * 
 * This function calls the required TivaWare functions to initialise the
 * driver. 
 * 
 * Before calling this function the system clock will have been initialised.
 *
 * @return ErrorCode Return code.
 */
ErrorCode I2c_init(void);

/**
 * @brief Step the I2C driver, performing any queued actions.
 * 
 * @return ErrorCode Return code.
 */
ErrorCode I2c_step(void);

/**
 * @brief Send the given bytes to the I2C device.
 * 
 * This function will send bytes to the I2C device in single byte mode when
 * length_in == 1, and multi byte mode when length_in > 1.
 * 
 * The user must call I2c_clear_device_action for this device before attempting
 * another operation on the device.
 * 
 * @param p_device_in The device to send the data to.
 * @param p_data_in A pointer to an array of bytes to send.
 * @param length_in The length of the byte array.
 * @return ErrorCode Return code.
 */
ErrorCode I2c_device_send_bytes(
    I2c_Device *p_device_in, 
    uint8_t *p_data_in, 
    size_t length_in
);

/**
 * @brief Recieve bytes from the I2C device.
 * 
 * This function requests bytes stored in reg_in from the given device.
 * 
 * The user must call I2c_clear_device_action for this device before attempting
 * another operation on the device.
 * 
 * @param p_device_in The device to recieve from.
 * @param reg_in The register to read from the device.
 * @param length_in The number of bytes expected from the device.
 * @return ErrorCode Return code.
 */
ErrorCode I2c_device_recv_bytes(
    I2c_Device *p_device_in,
    uint8_t reg_in,
    size_t length_in
);

/**
 * @brief Get the bytes recieved from the I2C device.
 * 
 * The user must have previously called I2c_device_recv_bytes and verified the
 * success of this action with I2c_get_device_action_status.
 * 
 * @param p_device_in The device to retrieve bytes from.
 * @param pp_bytes_out A pointer to an array of bytes to write in. The array
 *        must be at least the size passed into I2c_device_recv_bytes.
 * @return ErrorCode Return code.
 */
ErrorCode I2c_get_device_recved_bytes(
    I2c_Device *p_device_in,
    uint8_t *p_bytes_out
);

/**
 * @brief Get the status of an action being performed on a device.
 * 
 * @param p_device_in The device to check.
 * @param p_status_out Pointer to a I2c_ActionStatus enum indicating the status
 *        of this device's action.
 * @return ErrorCode Return code.
 */
ErrorCode I2c_get_device_action_status(
    I2c_Device *p_device_in,
    I2c_ActionStatus *p_status_out
);

/**
 * @brief Get the error associated with a failed I2C action.
 * 
 * @param p_device_in The device to check.
 * @param p_error_out Pointer to an ErrorCode which will be filled with the
 *        error that caused the action to fail.
 * @return ErrorCode Return code.
 */
ErrorCode I2c_get_device_action_failure_cause(
    I2c_Device *p_device_in,
    ErrorCode *p_error_out
);

/**
 * @brief Clear the action for the given device.
 * 
 * This function should be called after the information stored in the action
 * (for instance received bytes) has been retrieved by the user. Failing to
 * call this function may result in too many actions remaining and an error in
 * the future.
 * 
 * @param p_device_in The device to clear.
 * @return ErrorCode Return code.
 */
ErrorCode I2c_clear_device_action(I2c_Device *p_device_in);

#endif /* H_I2C_PUBLIC_H */

/** @} */ /* End of I2C group */