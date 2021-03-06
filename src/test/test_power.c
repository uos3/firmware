/**
 * @file test_power.c
 * @author Duncan Hamill (dh2g16@soton.ac.uk/duncanrhamill@googlemail.com)
 * @brief Power Application main test executable
 * 
 * Task ref: [UT_2.11.6]
 * 
 * @version 0.1
 * @date 2021-02-26
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
#include "util/debug/Debug_public.h"
#include "drivers/rtc/Rtc_public.h"
#include "drivers/board/Board_public.h"
#include "drivers/eeprom/Eeprom_public.h"
#include "drivers/timer/Timer_public.h"
#include "drivers/udma/Udma_public.h"
#include "drivers/uart/Uart_public.h"
#include "system/kernel/Kernel_public.h"
#include "system/data_pool/DataPool_public.h"
#include "system/event_manager/EventManager_public.h"
#include "system/mem_store_manager/MemStoreManager_public.h"
#include "system/opmode_manager/OpModeManager_public.h"
#include "components/eps/Eps_public.h"
#include "applications/power/Power_public.h"

/* -------------------------------------------------------------------------   
 * MAIN
 * ------------------------------------------------------------------------- */

int main(void) {
    ErrorCode error = ERROR_NONE;
    bool run_loop = true;
    uint8_t test_step = 0;
    Event timer_event = EVT_NONE;
    double ocp_duration_s = 30.0;
    double timeout_duration_s = CFG.POWER_TASK_TIMER_DURATION_S * 1.1;
    bool timeout_passed;
    bool check_wfi = true;

    /* Init system critical modules */
    Kernel_init_critical_modules();
    if (Rtc_init() != ERROR_NONE) {
        Debug_exit(1);
    }
    if (Eeprom_init() != ERROR_NONE) {
        Debug_exit(1);
    }
    if (!MemStoreManager_init()) {
        Debug_exit(1);
    }
    error = Timer_init();
    if (error != ERROR_NONE) {
        DEBUG_ERR("Timer init error 0x%04X", error);
        Debug_exit(1);
    }

    DEBUG_INF("---- Power Test ----");
    DEBUG_INF("Basic initialisation complete");

    /* Init UDMA and UART */
    if (Udma_init() != ERROR_NONE) {
        DEBUG_ERR("Couldn't init uDMA");
        Debug_exit(1);
    }
    if (Uart_init() != ERROR_NONE) {
        DEBUG_ERR("Couldn't init Uart");
        Debug_exit(1);
    }

    /* Init the EPS */
    if (!Eps_init()) {
        Debug_exit(1);
    }
    DEBUG_INF("Eps initialised");

    /* Init the opmode manager */
    if (!OpModeManager_init()) {
        Debug_exit(1);
    }
    DEBUG_INF("OpModeManager initialised");

    /* Init the power app */
    if (!Power_init()) {
        Debug_exit(1);
    }
    DEBUG_INF("Power initialised");

    /* Start timer for opmode transitions */
    if (Timer_start_periodic(
            ocp_duration_s, 
            &timer_event
        ) 
        != 
        ERROR_NONE
    ) {
        DEBUG_ERR("Couldn't start OCP timer");
        Debug_exit(1);
    }

    DEBUG_INF("---- TEST 1: OPMODE TRANSITIONS ----");

    /* Enter main exec loop */
    while (run_loop) {

        /* ---- DRIVERS ---- */
        
        /* Removed: pending changes to UART driver */
        #if 0
        error = Uart_step_tx();
        if (error != ERROR_NONE) {
            DEBUG_ERR("Uart_step_tx() failed! Error = 0x%04X", error);
            Debug_exit(1);
        }
        error = Uart_step_rx();
        if (error != ERROR_NONE) {
            DEBUG_ERR("Uart_step_rx() failed! Error = 0x%04X", error);
            Debug_exit(1);
        }
        #endif

        /* ---- COMPONENTS ---- */
        
        if (!Eps_step()) {
            char error_chain[64] = {0};
            Kernel_error_to_string(&DP.EPS.ERROR, error_chain);
            DEBUG_ERR(
                "Eps_step() failed! Error chain = %s", 
                error_chain
            );

            /* If the root cause is a timeout we will actually not exit here,
             * since Power is supposed to handle these */
            if (DP.EPS.ERROR.code != EPS_ERROR_COMMAND_TIMEOUT) {
                Debug_exit(1);
            }
            else {
                DEBUG_INF("Continuing to allow Power to handle timeout");
            }
        }

        /* ---- APPLICATIONS ---- */

        if (!Power_step()) {
            char error_chain[64] = {0};
            Kernel_error_to_string(&DP.POWER.ERROR, error_chain);
            DEBUG_ERR(
                "Power_step() failed! Error chain = %s",
                error_chain
            );
            Debug_exit(1);
        }

        if (!OpModeManager_step()) {
            char error_chain[64] = {0};
            Kernel_error_to_string(&DP.OPMODEMANAGER.ERROR, error_chain);
            DEBUG_ERR(
                "OpModeManager_step() failed! Error chain = %s",
                error_chain
            );
            Debug_exit(1);
        }

        /* Cycle clean up */
        EventManager_cleanup_events();

        /* ---- TEST SEQUENCE ---- */
        switch (test_step) {
            /* Stepping through different opmodes */
            case 0:
                /* If the OCP change has timed out, of if we haven't started
                 * one yet */
                if (EventManager_is_event_raised(timer_event)
                    ||
                    timer_event == EVT_NONE
                ) {
                    if (DP.OPMODEMANAGER.OPMODE 
                        < OPMODEMANAGER_OPMODE_PICTURE_TAKING
                    ) {                
                        
                        /* Trigger OpMode transition if not already in one */
                        if (DP.OPMODEMANAGER.NEXT_OPMODE 
                            == DP.OPMODEMANAGER.OPMODE
                        ) {
                            DP.OPMODEMANAGER.NEXT_OPMODE 
                                = DP.OPMODEMANAGER.OPMODE + 1;
                            DP.OPMODEMANAGER.TC_REQUEST_NEW_OPMODE = true;

                            DEBUG_INF(
                                "No events, setting OPMODE to %d", 
                                DP.OPMODEMANAGER.NEXT_OPMODE
                            );
                        }
                    }
                    else {
                        DEBUG_INF("Final OPMODE reached, moving to next step");
                        /* Disable the wfi check until we get back to nominal
                         * operation */
                        check_wfi = false;
                        test_step++;
                    }
                }
                break;
            /* Transition back to NF mode */
            case 1:
                DEBUG_INF("Transition back to NF for next test");
                DP.OPMODEMANAGER.NEXT_OPMODE 
                    = OPMODEMANAGER_OPMODE_NOMINAL_FUNCTIONING;
                DP.OPMODEMANAGER.TC_REQUEST_NEW_OPMODE = true;
                test_step++;
                break;
            case 2:
                if (DP.OPMODEMANAGER.OPMODE 
                    == 
                    OPMODEMANAGER_OPMODE_NOMINAL_FUNCTIONING
                ) {
                    test_step++;
                    DEBUG_INF("---- TEST 2: HK DATA COLLECTION ----");
                    DEBUG_INF("Waiting for POWER to collect EPS HK data");
                    check_wfi = true;
                    
                    /* Record the current time so we have a timeout on the data
                     * collection */
                    if (Timer_start_one_shot(
                        timeout_duration_s,
                        &timer_event
                    ) != ERROR_NONE) {
                        DEBUG_ERR("Couldn't start timeout timer");
                        Debug_exit(1);
                    }
                }
                break;
            /* Wait for EPS HK data */
            case 3:
                /* Check timeout of collection */
                if (EventManager_poll_event(timer_event)) {
                    DEBUG_ERR("HK collection timeout passed");
                    Debug_exit(1);
                }

                /* Check for collection starting */
                if (EventManager_is_event_raised(EVT_EPS_NEW_HK_DATA)) {
                    DEBUG_INF("New HK data collected");
                    check_wfi = false;
                    test_step++;

                    if (Timer_disable(timer_event) != ERROR_NONE) {
                        DEBUG_ERR("Couldn't disable timeout timer");
                        Debug_exit(1);
                    }
                }
                break;
            case 4:
            /* End of test */
            default:
                DEBUG_INF("---- ALL TESTS PASSED ----");
                run_loop = false;
        }

        Kernel_disable_interrupts();
        if (DP.EVENTMANAGER.NUM_RAISED_EVENTS == 0 && check_wfi) {
            /* Wait for interrupts if no events raised on the TM4C */
            #ifdef TARGET_TM4C
            DEBUG_INF("No events, waiting for interrupt...");
            __asm("WFI");
            #endif
        }
        Kernel_enable_interrupts();
    }

    /* Cleanup */
    EventManager_destroy();

    return EXIT_SUCCESS;
}