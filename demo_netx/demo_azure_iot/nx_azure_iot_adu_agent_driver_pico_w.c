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
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hardware/flash.h"
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"
#include "flashloader.h"

#ifndef FLASH_BUFFER_SIZE
#define FLASH_BUFFER_SIZE 256 /* must be multiple of 256 */
#endif /* FLASH_BUFFER_SIZE */

// Offset within flash of the new app image to be flashed by the flashloader
static const UINT FLASH_IMAGE_OFFSET = 1024 * 1024;
static UINT flash_offset;
static UCHAR flash_header_buffer[256]; /* Due to CRC stored at beginning, the content must be stored. */
static UCHAR flash_buffer[FLASH_BUFFER_SIZE];
static UINT flash_total_size;
static UINT flash_crc;

void nx_azure_iot_adu_agent_driver_pico_w(NX_AZURE_IOT_ADU_AGENT_DRIVER *driver_req_ptr);

static UINT crc32(UCHAR *data, UINT len, UINT crc)
{
INT bit;

    while (len--)
    {
        crc ^= (*data++ << 24);

        for (bit = 0; bit < 8; bit++)
        {
            if (crc & (1L << 31))
                crc = (crc << 1) ^ 0x04C11DB7;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}

/****** DRIVER SPECIFIC ******/
void nx_azure_iot_adu_agent_driver_pico_w(NX_AZURE_IOT_ADU_AGENT_DRIVER *driver_req_ptr)
{
    UINT erase_length;
    UINT copy_size;
    UINT data_size;
    UINT remaining_size;
    UCHAR *data_ptr;
    tFlashHeader *flash_header;
    TX_INTERRUPT_SAVE_AREA

    /* Default to successful return.  */
    driver_req_ptr -> nx_azure_iot_adu_agent_driver_status = NX_AZURE_IOT_SUCCESS;
        
    /* Process according to the driver request type.  */
    switch (driver_req_ptr -> nx_azure_iot_adu_agent_driver_command)
    {

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_INITIALIZE:
        {

            /* Process initialize requests.  */
            printf("Driver initialized successfully.\r\n");
            break;
        }

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_UPDATE_CHECK:
        {

            /* Compare the installed_criteria (such as: version) to check if the update is installed or not.
               If installed, return NX_TRUE, else return NX_FALSE.  */
            *(driver_req_ptr -> nx_azure_iot_adu_agent_driver_return_ptr) = NX_FALSE;

            printf("Proxy driver update check successfully.\r\n");
            break;
        }

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_PREPROCESS:
        {
        
            /* Process firmware preprocess requests before writing firmware.
               Such as: erase the flash at once to improve the speed.  
               Round erase length up to next 4096 byte boundary. */
            erase_length = (driver_req_ptr -> nx_azure_iot_adu_agent_driver_firmware_size + 
                            sizeof(tFlashHeader) + 0xFFF) & 0xFFFFF000;
            TX_DISABLE
            flash_range_erase(FLASH_IMAGE_OFFSET, erase_length);
            TX_RESTORE
            flash_header = (tFlashHeader *)flash_header_buffer;
            flash_header -> magic1 = FLASH_MAGIC1;
            flash_header -> magic2 = FLASH_MAGIC2;
            flash_header -> length = driver_req_ptr -> nx_azure_iot_adu_agent_driver_firmware_size;
            flash_offset = sizeof(tFlashHeader);
            flash_total_size = sizeof(tFlashHeader);
            flash_crc = 0xFFFFFFFF;
            printf("Driver flash erased successfully.\r\n");
    
            break;
        }

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_WRITE:
        {
        
            /* Process firmware write requests.  */
            
            /* Write firmware contents.
               1. This function must be able to figure out which bank it should write to.
               2. Write firmware contents into new bank.
               3. Decrypt and authenticate the firmware itself if needed.
            */
            printf("Driver firmware writing...\r\n");
            data_ptr = driver_req_ptr->nx_azure_iot_adu_agent_driver_firmware_data_ptr;
            data_size = driver_req_ptr->nx_azure_iot_adu_agent_driver_firmware_data_size;
            flash_crc = crc32(data_ptr, data_size, flash_crc);
            if (flash_total_size < sizeof(flash_header_buffer))
            {
                /* flash_header_buffer is not full. */
                remaining_size = sizeof(flash_header_buffer) - flash_offset;
                if (remaining_size >= driver_req_ptr->nx_azure_iot_adu_agent_driver_firmware_data_size)
                {

                    /* all data can be filled in flash_header_buffer. */
                    copy_size = driver_req_ptr -> nx_azure_iot_adu_agent_driver_firmware_data_size;
                }
                else
                {
                        
                    /* flash_header_buffer is full. */
                    copy_size = remaining_size;
                }
                memcpy(flash_header_buffer + flash_offset, data_ptr, copy_size);
                flash_offset += copy_size;
                flash_total_size = flash_offset;
                data_size -= copy_size;
                data_ptr += copy_size;

                /* Check if flash_header_buffer is full. */
                if (flash_offset == sizeof(flash_header_buffer))
                {
                    flash_offset = 0;
                }

                if (data_size == 0)
                {
                    return;
                }
            }

            if (flash_offset > 0)
            {

                /* Handle remaining data in flash_buffer */
                remaining_size = FLASH_BUFFER_SIZE - flash_offset;
                if (data_size < remaining_size)
                {
                    memcpy(&flash_buffer[flash_offset], data_ptr, data_size);
                    flash_offset += data_size;
                    return;
                }
                else
                {
                    memcpy(&flash_buffer[flash_offset], data_ptr, remaining_size);
                    TX_DISABLE
                    flash_range_program(FLASH_IMAGE_OFFSET+ flash_total_size, flash_buffer, FLASH_BUFFER_SIZE);
                    TX_RESTORE
                    flash_total_size += FLASH_BUFFER_SIZE;
                    flash_offset = 0;
                    data_ptr += remaining_size;
                    data_size -= remaining_size;
                }
            }

            /* Write data_ptr to flash directly. */
            while (data_size >= FLASH_BUFFER_SIZE)
            {
                memcpy(flash_buffer, data_ptr, FLASH_BUFFER_SIZE);
                TX_DISABLE
                flash_range_program(FLASH_IMAGE_OFFSET + flash_total_size, flash_buffer, FLASH_BUFFER_SIZE);
                TX_RESTORE
                flash_total_size += FLASH_BUFFER_SIZE;
                data_ptr += FLASH_BUFFER_SIZE;
                data_size -= FLASH_BUFFER_SIZE;
            }

            if (data_size > 0)
            {
                memcpy(flash_buffer, data_ptr, data_size);
                flash_offset = data_size;
            }
            else
            {
                flash_offset = 0;
            }
            
            break;
        }

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_INSTALL:
        {

            /* Flash remaining data. */
            if (flash_offset > 0)
            {
                TX_DISABLE
                flash_range_program(FLASH_IMAGE_OFFSET + flash_total_size, flash_buffer, flash_offset);
                TX_RESTORE
            }

            /* Update crc and flash header. */
            flash_header = (tFlashHeader *)flash_header_buffer;
            flash_header -> crc32 = flash_crc;
            flash_range_program(FLASH_IMAGE_OFFSET, flash_header_buffer, sizeof(flash_header_buffer));

            /* Set the new firmware for next boot.  */
            printf("Driver firmware installed successfully.\r\n");

            break;
        }

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_APPLY:
        {

            /* Apply the new firmware, and reboot device from that.*/
            printf("Driver firmware apply successfully.\r\n");
            
            /* Set up watchdog scratch registers so that the flashloader knows 
               what to do after the reset */
            watchdog_hw->scratch[0] = FLASH_MAGIC1;
            watchdog_hw->scratch[1] = XIP_BASE + FLASH_IMAGE_OFFSET;
            watchdog_reboot(0x00000000, 0x00000000, 1000);
            for (;;);
        
            break;
        }
        default:
        {

            /* Invalid driver request.  */

            /* Default to successful return.  */
            driver_req_ptr -> nx_azure_iot_adu_agent_driver_status =  NX_AZURE_IOT_FAILURE;
        }
    }
}
