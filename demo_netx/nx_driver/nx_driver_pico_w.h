/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   NetX driver for Raspberry Pi Pico W                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef NX_DRIVER_PICO_W_H
#define NX_DRIVER_PICO_W_H

#ifdef   __cplusplus
extern   "C" {
#endif

#include "nx_api.h"

#define SAMPLE_NETWORK_DRIVER nx_driver_pico_w

/* Public API */
void nx_driver_pico_w(NX_IP_DRIVER *driver_req_ptr);


#ifdef   __cplusplus
    }
#endif

#endif