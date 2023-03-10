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

#include "nx_azure_iot_adu_agent.h"
#include "demo_update_bin.h"

#define ADU_AGENT_DRIVER nx_azure_iot_adu_agent_driver_pico_w
extern void ADU_AGENT_DRIVER(NX_AZURE_IOT_ADU_AGENT_DRIVER *driver_req_ptr);

#define VERSION "6.1"

/* Define main entry point.  */
int demo_threadx(void)
{
NX_AZURE_IOT_ADU_AGENT_DRIVER driver_request;
UCHAR *data_ptr;
UINT data_size;
UINT received_firmware_size;

    printf("\r\nADU driver testing %s!\r\n", VERSION);
    
    if (demo_update_bin_len == 0)
    {
        return(0);
    }
   
    /* Call the driver to initialize the hardware.  */
    driver_request.nx_azure_iot_adu_agent_driver_command = NX_AZURE_IOT_ADU_AGENT_DRIVER_INITIALIZE;
    ADU_AGENT_DRIVER(&driver_request);

    /* Check status.  */
    if (driver_request.nx_azure_iot_adu_agent_driver_status)
    {
        printf("Driver status error!\r\n");
        return(1);
    }
        
    /* Send the preprocess request to the driver.   */
    driver_request.nx_azure_iot_adu_agent_driver_command = NX_AZURE_IOT_ADU_AGENT_DRIVER_PREPROCESS;
    driver_request.nx_azure_iot_adu_agent_driver_firmware_size = demo_update_bin_len;
    ADU_AGENT_DRIVER(&driver_request);

    /* Check status.  */
    if (driver_request.nx_azure_iot_adu_agent_driver_status)
    {
        printf("Driver status error!\r\n");
        return(1);
    }
    
    /* Initialize the firmware pointer and size.  */
    data_ptr = (UCHAR *)demo_update_bin;
    data_size = 0;
    received_firmware_size = 0;
    
    while (received_firmware_size < demo_update_bin_len)
    {
        
        /* Random data size to simulate the message from network.  */
        data_size = (rand() % 1500) + 1;
        if (received_firmware_size + data_size > demo_update_bin_len)
        {
            data_size = demo_update_bin_len - received_firmware_size;
        }
        
        /* Send the firmware write request to the driver.   */
        driver_request.nx_azure_iot_adu_agent_driver_command = NX_AZURE_IOT_ADU_AGENT_DRIVER_WRITE;
        driver_request.nx_azure_iot_adu_agent_driver_firmware_data_offset = received_firmware_size;
        driver_request.nx_azure_iot_adu_agent_driver_firmware_data_ptr = data_ptr;
        driver_request.nx_azure_iot_adu_agent_driver_firmware_data_size = data_size;
        ADU_AGENT_DRIVER(&driver_request);

        /* Check status.  */
        if (driver_request.nx_azure_iot_adu_agent_driver_status)
        {
            printf("Driver write error!\r\n");
            return(1);
        }
        
        received_firmware_size += data_size;
        data_ptr += data_size;
    }

    /* Send the firmware install request to the driver.   */
    driver_request.nx_azure_iot_adu_agent_driver_command = NX_AZURE_IOT_ADU_AGENT_DRIVER_INSTALL;
    ADU_AGENT_DRIVER(&driver_request);

    /* Check status.  */
    if (driver_request.nx_azure_iot_adu_agent_driver_status)
    {
        printf("Driver status error!\r\n");
        return(1);
    }

    /* Send the firmware apply request to the driver.   */
    driver_request.nx_azure_iot_adu_agent_driver_command = NX_AZURE_IOT_ADU_AGENT_DRIVER_APPLY;
    ADU_AGENT_DRIVER(&driver_request);

    /* Check status.  */
    if (driver_request.nx_azure_iot_adu_agent_driver_status)
    {
        printf("Driver status error!\r\n");
        return(1);
    }
    
    return(0);
}