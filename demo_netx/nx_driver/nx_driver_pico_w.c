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

#define NX_DRIVER_SOURCE

#include "pico/cyw43_arch.h"
#include "nx_driver_pico_w.h"

#ifdef NX_DRIVER_CONFIG
#include NX_DRIVER_CONFIG
#endif

#include "nx_driver_framework.c"

#ifndef WIFI_SSID
#error The symbol WIFI_SSID should be defined
#endif /* WIFI_SSID  */

#ifndef WIFI_PASSWORD
#error The symbol WIFI_PASSWORD should be defined
#endif /* WIFI_PASSWORD  */

#ifndef WIFI_AUTH_TYPE
#define WIFI_AUTH_TYPE CYW43_AUTH_WPA2_AES_PSK
#endif /* WIFI_AUTH_TYPE */

#ifndef WIFI_CONNECT_TIMEOUT
#define WIFI_CONNECT_TIMEOUT 10000 /* tmieout in ms */
#endif                             /* WIFI_CONNECT_TIMEOUT */

#ifndef NX_DRIVER_STACK_SIZE
#define NX_DRIVER_STACK_SIZE 1024
#endif /* NX_DRIVER_STACK_SIZE  */

/* Interval to receive packets when there is no packet. The default value is 10 ticks which is 100ms.  */
#ifndef NX_DRIVER_THREAD_INTERVAL
#define NX_DRIVER_THREAD_INTERVAL (NX_IP_PERIODIC_RATE / 10)
#endif /* NX_DRIVER_THREAD_INTERVAL */

/* Interval to blink the LED. The default value is once per 10 cyw43_arch_poll. Define 0 to disable blink. */
#ifndef NX_DRIVER_BLINK_RATE
#define NX_DRIVER_BLINK_RATE 10
#endif /* NX_DRIVER_BLINK_RATE  */

static UINT _nx_driver_pico_w_initialize(NX_IP_DRIVER *driver_req_ptr);
static UINT _nx_driver_pico_w_enable(NX_IP_DRIVER *driver_req_ptr);
static UINT _nx_driver_pico_w_disable(NX_IP_DRIVER *driver_req_ptr);
static UINT _nx_driver_pico_w_packet_send(NX_PACKET *packet_ptr);
static VOID _nx_driver_pico_w_thread_entry(ULONG thread_input);

static TX_THREAD nx_driver_pico_w_thread;
static UCHAR nx_driver_pico_w_thread_stack[NX_DRIVER_STACK_SIZE];
static UCHAR nx_driver_pico_w_link_up;

static UCHAR _nx_driver_buffer[1514];

void nx_driver_pico_w(NX_IP_DRIVER *driver_req_ptr)
{
  static UCHAR start = NX_FALSE;
  if (!start)
  {
    nx_driver_hardware_initialize = _nx_driver_pico_w_initialize;
    nx_driver_hardware_enable = _nx_driver_pico_w_enable;
    nx_driver_hardware_disable = _nx_driver_pico_w_disable;
    nx_driver_hardware_packet_send = _nx_driver_pico_w_packet_send;

    start = NX_TRUE;
  }

  nx_driver_framework_entry_default(driver_req_ptr);
}

struct pbuf;
uint16_t pbuf_copy_partial(const struct pbuf *p, void *dataptr, uint16_t len, uint16_t offset)
{

  /* Stub function as it will never hit. */
  return 0;
}

static VOID _nx_driver_pico_w_thread_entry(ULONG thread_input)
{
  NX_IP *ip_ptr = nx_driver_information.nx_driver_information_ip_ptr;
#if NX_DRIVER_BLINK_RATE > 0
  UCHAR led_state = 0;
  UINT blink_count = 0;
#endif /* NX_DRIVER_BLINK_RATE  */

  NX_PARAMETER_NOT_USED(thread_input);

  for (;;)
  {
    if (nx_driver_information.nx_driver_information_ip_ptr->nx_ip_driver_link_up)
    {

      /* Obtain the IP internal mutex before processing the IP event.  */
      tx_mutex_get(&(ip_ptr->nx_ip_protection), TX_WAIT_FOREVER);

      /* Poll WIFI events. */
      cyw43_arch_poll();

      /* Release the IP internal mutex before processing the IP event.  */
      tx_mutex_put(&(ip_ptr->nx_ip_protection));
    }

    /* Sleep some ticks to next loop.  */
    tx_thread_sleep(NX_DRIVER_THREAD_INTERVAL);

#if NX_DRIVER_BLINK_RATE > 0
    if (++blink_count < NX_DRIVER_BLINK_RATE)
      continue;
    led_state = 1 - led_state;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
    blink_count = 0;
#endif /* NX_DRIVER_BLINK_RATE  */
  }
}

int cyw43_tcpip_link_status(cyw43_t *self, int itf)
{
  if (nx_driver_pico_w_link_up)
    return CYW43_LINK_UP;
  else
    return cyw43_wifi_link_status(self, itf);
}

void cyw43_cb_tcpip_init(cyw43_t *self, int itf)
{
  if (!nx_driver_pico_w_link_up)
    return;
  nx_driver_information.nx_driver_information_ip_ptr->nx_ip_driver_link_up = NX_TRUE;
  _nx_ip_driver_link_status_event(nx_driver_information.nx_driver_information_ip_ptr,
                                  nx_driver_information.nx_driver_information_interface->nx_interface_index);
}
void cyw43_cb_tcpip_deinit(cyw43_t *self, int itf)
{
  if (!nx_driver_pico_w_link_up)
    return;
  nx_driver_information.nx_driver_information_ip_ptr->nx_ip_driver_link_up = NX_FALSE;
  _nx_ip_driver_link_status_event(nx_driver_information.nx_driver_information_ip_ptr,
                                  nx_driver_information.nx_driver_information_interface->nx_interface_index);
}
void cyw43_cb_tcpip_set_link_up(cyw43_t *self, int itf)
{
  nx_driver_pico_w_link_up = NX_TRUE;
}
void cyw43_cb_tcpip_set_link_down(cyw43_t *self, int itf)
{
  nx_driver_pico_w_link_up = NX_FALSE;
}
void cyw43_cb_process_ethernet(void *cb_data, int itf, size_t len, const uint8_t *buf)
{
  NX_PACKET *packet_ptr;
  if (!nx_driver_pico_w_link_up)
    return; /* interface down */

  /* Allocate a packet to receive data. */
  if (nx_packet_allocate(nx_driver_information.nx_driver_information_packet_pool_ptr,
                         &packet_ptr, NX_RECEIVE_PACKET, NX_NO_WAIT))
  {

    /* No packet available. Just return. */
    return;
  }

  /* Adjust packet starting address to align IP header to 4 bytes boundary*/
  packet_ptr->nx_packet_prepend_ptr += 2;
  packet_ptr->nx_packet_append_ptr += 2;

  /* Append data to packet_ptr. */
  if (nx_packet_data_append(packet_ptr, (VOID *)buf, len,
                            nx_driver_information.nx_driver_information_packet_pool_ptr, NX_NO_WAIT))
  {

    /* Error, release the packet. */
    nx_packet_release(packet_ptr);
    return;
  }

  /* Everything is OK, transfer the packet to NetX.  */
  nx_driver_transfer_to_netx(nx_driver_information.nx_driver_information_ip_ptr, packet_ptr);
}

UINT _nx_driver_pico_w_initialize(NX_IP_DRIVER *driver_req_ptr)
{
  UCHAR mac[6];
  UINT priority = 0;

  if (cyw43_arch_init())
  {
    printf("failed to initialise\n");
    return NX_DRIVER_ERROR;
  }

  cyw43_arch_enable_sta_mode();

  cyw43_hal_get_mac(0, mac);
  nx_driver_update_hardware_address(mac);

  /* Get priority of IP thread.  */
  tx_thread_info_get(tx_thread_identify(), NX_NULL, NX_NULL, NX_NULL, &priority,
                     NX_NULL, NX_NULL, NX_NULL, NX_NULL);

  /* Create the driver thread.  */
  /* The priority of network thread is lower than IP thread.  */
  tx_thread_create(&nx_driver_pico_w_thread, "Driver Thread", _nx_driver_pico_w_thread_entry, 0,
                   nx_driver_pico_w_thread_stack, NX_DRIVER_STACK_SIZE,
                   priority + 1, priority + 1,
                   TX_NO_TIME_SLICE, TX_DONT_START);

  return NX_SUCCESS;
}

UINT _nx_driver_pico_w_enable(NX_IP_DRIVER *driver_req_ptr)
{
  printf("Connecting to Wi-Fi...\n");
  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                         WIFI_AUTH_TYPE, WIFI_CONNECT_TIMEOUT))
  {
    printf("failed to connect.\n");
    return NX_DRIVER_ERROR;
  }
  else
  {
    printf("Connected to %s.\n", WIFI_SSID);
  }

  tx_thread_reset(&nx_driver_pico_w_thread);
  tx_thread_resume(&nx_driver_pico_w_thread);

  return NX_SUCCESS;
}

UINT _nx_driver_pico_w_disable(NX_IP_DRIVER *driver_req_ptr)
{
  tx_thread_suspend(&nx_driver_pico_w_thread);
  tx_thread_terminate(&nx_driver_pico_w_thread);
  return NX_SUCCESS;
}

UINT _nx_driver_pico_w_packet_send(NX_PACKET *packet_ptr)
{
  int ret;
  ULONG bytes_copied;
  if (packet_ptr->nx_packet_next)
  {
    if (packet_ptr->nx_packet_length > sizeof(_nx_driver_buffer))
    {

      /* Buffer too small to hold packet, release the packet. */
      NX_DRIVER_PHYSICAL_HEADER_REMOVE(packet_ptr);
      nx_packet_transmit_release(packet_ptr);
      return NX_DRIVER_ERROR;
    }

    /* Extract data from packet to contigous buffer. */
    if (nx_packet_data_retrieve(packet_ptr, _nx_driver_buffer, &bytes_copied))
    {

      /* Error, release the packet. */
      nx_packet_release(packet_ptr);
      return NX_DRIVER_ERROR;
    }
    ret = cyw43_send_ethernet(&cyw43_state, 0, bytes_copied, _nx_driver_buffer, false);
  }
  else
  {
    ret = cyw43_send_ethernet(&cyw43_state, 0, packet_ptr->nx_packet_length, packet_ptr->nx_packet_prepend_ptr, false);
  }
  NX_DRIVER_PHYSICAL_HEADER_REMOVE(packet_ptr);
  nx_packet_transmit_release(packet_ptr);
  if (ret)
  {
    return NX_DRIVER_ERROR;
  }

  return NX_SUCCESS;
}