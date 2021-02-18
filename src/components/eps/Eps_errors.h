/**
 * @file Eps_errors.h
 * @author Duncan Hamill (dh2g16@soton.ac.uk/duncanrhamill@googlemail.com)
 * @brief EPS Component Module error definitions
 * 
 * Task ref: [UT_2.10.7]
 * 
 * @version 0.1
 * @date 2021-02-17
 * 
 * @copyright Copyright (c) 2021
 */

#ifndef H_EPS_ERRORS_H
#define H_EPS_ERRORS_H

/* -------------------------------------------------------------------------   
 * INCLUDES
 * ------------------------------------------------------------------------- */

/* Standard includes */
#include <stdint.h>

/* Internal includes */
#include "system/kernel/Kernel_errors.h"
#include "system/kernel/Kernel_module_ids.h"

/* -------------------------------------------------------------------------   
 * ERRORS
 * ------------------------------------------------------------------------- */

/**
 * @brief An Eps function was called while the component is not iniailised.
 */
#define EPS_ERROR_NOT_INITIALISED ((ErrorCode)(MOD_ID_EPS | 1))

/**
 * @brief The Eps module state is invalid, i.e. not one of the `EPS_STATE_x`
 * enums. 
 */
#define EPS_ERROR_INVALID_STATE ((ErrorCode)(MOD_ID_EPS | 2))

/**
 * @brief Attempted to send a command to the EPS when the Eps module wasn't in
 * the IDLE state.
 */
#define EPS_ERROR_SEND_TC_WHEN_NOT_IDLE ((ErrorCode)(MOD_ID_EPS | 3))


#endif /* H_EPS_ERRORS_H */