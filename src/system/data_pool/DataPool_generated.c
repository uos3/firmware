
/**
 * @ingroup data_pool
 * 
 * @file DataPool_generated.c
 * @author Generated by DataPool_generate.py
 * @brief Generated code to assist with DataPool usage.
 * 
 * This file was generated from DataPool_struct.h by DataPool_generate.py.
 * 
 * @version Generated from DataPool_struct.h version 0.1
 * @date 2020-11-18
 * 
 * @copyright Copyright (c) UoS3 2020
 */

/* -------------------------------------------------------------------------   
 * INCLUDES
 * ------------------------------------------------------------------------- */

/* Standard library includes */
#include <stddef.h>

/* Internal includes */
#include "util/debug/Debug_public.h"
#include "system/data_pool/DataPool_public.h"
#include "system/data_pool/DataPool_generated.h"

/* -------------------------------------------------------------------------   
 * FUNCTIONS
 * ------------------------------------------------------------------------- */

bool DataPool_get(
    DataPool_Id id_in,
    void **pp_data_out,
    DataPool_DataType *p_data_type_out,
    size_t *p_data_size_out
) {

    switch (id_in) {
    
    
    /* DP.INITIALISED */
    case 0x0001:
        *pp_data_out = &DP.INITIALISED;
        *p_data_type_out = DATAPOOL_DATATYPE_BOOL;
        *p_data_size_out = sizeof(bool);
        return true;


    /* DP.BOARD_INITIALISED */
    case 0x0002:
        *pp_data_out = &DP.BOARD_INITIALISED;
        *p_data_type_out = DATAPOOL_DATATYPE_BOOL;
        *p_data_size_out = sizeof(bool);
        return true;


    /* DP.EVENTMANAGER.INITIALISED */
    case 0x0201:
        *pp_data_out = &DP.EVENTMANAGER.INITIALISED;
        *p_data_type_out = DATAPOOL_DATATYPE_BOOL;
        *p_data_size_out = sizeof(bool);
        return true;


    /* DP.EVENTMANAGER.ERROR_CODE */
    case 0x0202:
        *pp_data_out = &DP.EVENTMANAGER.ERROR_CODE;
        *p_data_type_out = DATAPOOL_DATATYPE_EVENTMANAGER_ERRORCODE;
        *p_data_size_out = sizeof(EventManager_ErrorCode);
        return true;


    /* DP.EVENTMANAGER.MAX_EVENTS_REACHED */
    case 0x0203:
        *pp_data_out = &DP.EVENTMANAGER.MAX_EVENTS_REACHED;
        *p_data_type_out = DATAPOOL_DATATYPE_BOOL;
        *p_data_size_out = sizeof(bool);
        return true;


    /* DP.EVENTMANAGER.NUM_RAISED_EVENTS */
    case 0x0204:
        *pp_data_out = &DP.EVENTMANAGER.NUM_RAISED_EVENTS;
        *p_data_type_out = DATAPOOL_DATATYPE_UINT8_T;
        *p_data_size_out = sizeof(uint8_t);
        return true;


    /* DP.EVENTMANAGER.EVENT_LIST_SIZE */
    case 0x0205:
        *pp_data_out = &DP.EVENTMANAGER.EVENT_LIST_SIZE;
        *p_data_type_out = DATAPOOL_DATATYPE_SIZE_T;
        *p_data_size_out = sizeof(size_t);
        return true;


    /* DP.IMU.INITIALISED */
    case 0x0401:
        *pp_data_out = &DP.IMU.INITIALISED;
        *p_data_type_out = DATAPOOL_DATATYPE_BOOL;
        *p_data_size_out = sizeof(bool);
        return true;


    /* DP.IMU.ERROR */
    case 0x0402:
        *pp_data_out = &DP.IMU.ERROR;
        *p_data_type_out = DATAPOOL_DATATYPE_IMU_ERRORCODE;
        *p_data_size_out = sizeof(Imu_ErrorCode);
        return true;


    /* DP.IMU.STATE */
    case 0x0403:
        *pp_data_out = &DP.IMU.STATE;
        *p_data_type_out = DATAPOOL_DATATYPE_IMU_STATE;
        *p_data_size_out = sizeof(Imu_State);
        return true;


    /* DP.IMU.SUBSTATE */
    case 0x0404:
        *pp_data_out = &DP.IMU.SUBSTATE;
        *p_data_type_out = DATAPOOL_DATATYPE_IMU_SUBSTATE;
        *p_data_size_out = sizeof(Imu_SubState);
        return true;


    /* DP.IMU.COMMAND */
    case 0x0405:
        *pp_data_out = &DP.IMU.COMMAND;
        *p_data_type_out = DATAPOOL_DATATYPE_IMU_COMMAND;
        *p_data_size_out = sizeof(Imu_Command);
        return true;


    /* DP.IMU.GYROSCOPE_DATA */
    case 0x0406:
        *pp_data_out = &DP.IMU.GYROSCOPE_DATA;
        *p_data_type_out = DATAPOOL_DATATYPE_IMU_VECINT16;
        *p_data_size_out = sizeof(Imu_VecInt16);
        return true;


    /* DP.IMU.GYROSCOPE_DATA_VALID */
    case 0x0407:
        *pp_data_out = &DP.IMU.GYROSCOPE_DATA_VALID;
        *p_data_type_out = DATAPOOL_DATATYPE_BOOL;
        *p_data_size_out = sizeof(bool);
        return true;


    /* DP.IMU.MAGNETOMETER_DATA */
    case 0x0408:
        *pp_data_out = &DP.IMU.MAGNETOMETER_DATA;
        *p_data_type_out = DATAPOOL_DATATYPE_IMU_VECINT16;
        *p_data_size_out = sizeof(Imu_VecInt16);
        return true;


    /* DP.IMU.MAGNE_SENSE_ADJUST_DATA */
    case 0x0409:
        *pp_data_out = &DP.IMU.MAGNE_SENSE_ADJUST_DATA;
        *p_data_type_out = DATAPOOL_DATATYPE_IMU_VECINT16;
        *p_data_size_out = sizeof(Imu_VecInt16);
        return true;


    /* DP.IMU.MAGNETOMETER_DATA_VALID */
    case 0x040a:
        *pp_data_out = &DP.IMU.MAGNETOMETER_DATA_VALID;
        *p_data_type_out = DATAPOOL_DATATYPE_BOOL;
        *p_data_size_out = sizeof(bool);
        return true;


    /* DP.IMU.TEMPERATURE_DATA */
    case 0x040b:
        *pp_data_out = &DP.IMU.TEMPERATURE_DATA;
        *p_data_type_out = DATAPOOL_DATATYPE_INT16_T;
        *p_data_size_out = sizeof(int16_t);
        return true;

    
    default:
        DEBUG_ERR("Invalid DP ID: %d", id_in);
        return false;
    }
}

bool DataPool_get_symbol_str(
    DataPool_Id id_in, 
    char **pp_symbol_str_out
) {

    switch (id_in) {
    
    
    /* DP.INITIALISED */
    case 0x0001:
        *pp_symbol_str_out = strdup("DP.INITIALISED");
        return true;


    /* DP.BOARD_INITIALISED */
    case 0x0002:
        *pp_symbol_str_out = strdup("DP.BOARD_INITIALISED");
        return true;


    /* DP.EVENTMANAGER.INITIALISED */
    case 0x0201:
        *pp_symbol_str_out = strdup("DP.EVENTMANAGER.INITIALISED");
        return true;


    /* DP.EVENTMANAGER.ERROR_CODE */
    case 0x0202:
        *pp_symbol_str_out = strdup("DP.EVENTMANAGER.ERROR_CODE");
        return true;


    /* DP.EVENTMANAGER.MAX_EVENTS_REACHED */
    case 0x0203:
        *pp_symbol_str_out = strdup("DP.EVENTMANAGER.MAX_EVENTS_REACHED");
        return true;


    /* DP.EVENTMANAGER.NUM_RAISED_EVENTS */
    case 0x0204:
        *pp_symbol_str_out = strdup("DP.EVENTMANAGER.NUM_RAISED_EVENTS");
        return true;


    /* DP.EVENTMANAGER.EVENT_LIST_SIZE */
    case 0x0205:
        *pp_symbol_str_out = strdup("DP.EVENTMANAGER.EVENT_LIST_SIZE");
        return true;


    /* DP.IMU.INITIALISED */
    case 0x0401:
        *pp_symbol_str_out = strdup("DP.IMU.INITIALISED");
        return true;


    /* DP.IMU.ERROR */
    case 0x0402:
        *pp_symbol_str_out = strdup("DP.IMU.ERROR");
        return true;


    /* DP.IMU.STATE */
    case 0x0403:
        *pp_symbol_str_out = strdup("DP.IMU.STATE");
        return true;


    /* DP.IMU.SUBSTATE */
    case 0x0404:
        *pp_symbol_str_out = strdup("DP.IMU.SUBSTATE");
        return true;


    /* DP.IMU.COMMAND */
    case 0x0405:
        *pp_symbol_str_out = strdup("DP.IMU.COMMAND");
        return true;


    /* DP.IMU.GYROSCOPE_DATA */
    case 0x0406:
        *pp_symbol_str_out = strdup("DP.IMU.GYROSCOPE_DATA");
        return true;


    /* DP.IMU.GYROSCOPE_DATA_VALID */
    case 0x0407:
        *pp_symbol_str_out = strdup("DP.IMU.GYROSCOPE_DATA_VALID");
        return true;


    /* DP.IMU.MAGNETOMETER_DATA */
    case 0x0408:
        *pp_symbol_str_out = strdup("DP.IMU.MAGNETOMETER_DATA");
        return true;


    /* DP.IMU.MAGNE_SENSE_ADJUST_DATA */
    case 0x0409:
        *pp_symbol_str_out = strdup("DP.IMU.MAGNE_SENSE_ADJUST_DATA");
        return true;


    /* DP.IMU.MAGNETOMETER_DATA_VALID */
    case 0x040a:
        *pp_symbol_str_out = strdup("DP.IMU.MAGNETOMETER_DATA_VALID");
        return true;


    /* DP.IMU.TEMPERATURE_DATA */
    case 0x040b:
        *pp_symbol_str_out = strdup("DP.IMU.TEMPERATURE_DATA");
        return true;


    default:
        DEBUG_ERR("Invalid DP ID: %d", id_in);
        return false;
    }
}
