cmake_minimum_required(VERSION 3.12)

set(ENV{PICO_CYW43_DRIVER_PATH} ${CMAKE_CURRENT_LIST_DIR}/lib/cyw43-driver)

include(pico_sdk_import.cmake)

project(Azure-RTOS-on-Raspberry-Pi-Pico-RP2040)

pico_sdk_init()

set(THREADX_ARCH "cortex_m0")
set(THREADX_TOOLCHAIN "gnu")
add_subdirectory(lib/threadx)
add_subdirectory(lib/netxduo)
add_subdirectory(demo_threadx)
add_subdirectory(demo_netx)