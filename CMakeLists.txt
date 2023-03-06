cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(Azure-RTOS-on-Raspberry-Pi-Pico-RP2040)

pico_sdk_init()

set(THREADX_ARCH "cortex_m0")
set(THREADX_TOOLCHAIN "gnu")
add_subdirectory(threadx)
add_subdirectory(demo_threadx)