/**
 * @file MemStoreManager_dp_struct.h
 * @author Duncan Hamill (dh2g16@soton.ac.uk/duncanrhamill@googlemail.com)
 * @brief MemStoreManager datapool structure
 * @version 0.1
 * @date 2020-12-16
 * 
 * @copyright Copyright (c) UoS3 2020
 */

#ifndef H_MEMSTOREMANAGER_DP_STRUCT_H
#define H_MEMSTOREMANAGER_DP_STRUCT_H

/* -------------------------------------------------------------------------   
 * INCLUDES
 * ------------------------------------------------------------------------- */

/* Internal includes */
#include "system/kernel/Kernel_errors.h"
#include "system/mem_store_manager/MemStoreManager_public.h"

/* -------------------------------------------------------------------------   
 * STRUCTS
 * ------------------------------------------------------------------------- */

typedef struct _MemStoreManager_Dp {

    /**
     * @brief Flag indicating whether or not the MemStoreManager is
     * initialised. 
     * 
     * @dp 1
     */
    bool INITIALISED;

    /**
     * @brief Error code for the MemStoreManager
     * 
     * @dp 2
     */
    ErrorCode ERROR_CODE;

    /**
     * @brief Error code from the EEPROM driver.
     * 
     * @dp 3
     */
    ErrorCode EEPROM_ERROR_CODE;

    /**
     * @brief Indicates if the first configuration file is OK.
     * 
     * @dp 4
     */
    bool CFG_FILE_1_OK;

    /**
     * @brief Indicates if the second configuration file is OK.
     * 
     * @dp 5
     */
    bool CFG_FILE_2_OK;

    /**
     * @brief Indicates if the third configuration file is OK.
     * 
     * @dp 6
     */
    bool CFG_FILE_3_OK;

    /**
     * @brief Flag which can be set during the boot process if the EEPROM is
     * not functioning, and therefore we must use the redundent config stored
     * as apart of the software image.
     * 
     * @dp 7
     */
    bool USE_BACKUP_CFG;

    /**
     * @brief Flag which indicates that the persistent data has been modified
     * since the previous call to MemStoreManager_step, and should be written
     * to the EEPROM.
     * 
     * @dp 8
     */
    bool PERS_DATA_DIRTY;

    /**
     * @brief Indicates if the first persistent file is OK.
     * 
     * @dp 9
     */
    bool PERS_FILE_1_OK;

    /**
     * @brief Indicates if the second persistent file is OK.
     * 
     * @dp 10
     */
    bool PERS_FILE_2_OK;

    /**
     * @brief Indicates if the third persistent file is OK.
     * 
     * @dp 11
     */
    bool PERS_FILE_3_OK;

} MemStoreManager_Dp;

#endif /* H_MEMSTOREMANAGER_DP_STRUCT_H */