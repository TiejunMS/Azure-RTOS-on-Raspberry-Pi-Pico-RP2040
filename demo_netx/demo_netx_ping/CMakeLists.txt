project(demo_netx_ping)

add_executable(${PROJECT_NAME} ../../main.c ../tx_initialize_low_level.S demo_netx_ping.c)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        NX_ENABLE_DHCP
        )

# pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_add_extra_outputs(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} nx_driver_pico_w)
