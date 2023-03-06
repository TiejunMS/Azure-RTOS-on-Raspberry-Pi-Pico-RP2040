project(demo_threadx)

add_executable(${PROJECT_NAME} ../main.c tx_initialize_low_level.S demo_threadx.c)

pico_enable_stdio_usb(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})

if (PICO_CYW43_SUPPORTED) # set by PICO_BOARD=pico_w
    target_link_libraries(${PROJECT_NAME} pico_cyw43_arch_none )
endif()
target_link_libraries(${PROJECT_NAME} pico_stdlib cmsis_core azrtos::threadx)