project(demo_azure_iot)

add_executable(${PROJECT_NAME} ../../main.c ../tx_initialize_low_level.S 
                               demo_azure_iot.c
                               nx_azure_iot_adu_agent_proxy_simulator_driver.c
                               nx_azure_iot_adu_agent_simulator_driver.c
                               nx_azure_iot_cert.c
                               nx_azure_iot_ciphersuites.c
                               sample_azure_iot_embedded_sdk_adu.c
                               sample_azure_iot_embedded_sdk_connect.c
                               sample_azure_iot_embedded_sdk_pnp.c
                               sample_device_identity.c)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        NX_ENABLE_DHCP
        )

# pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_add_extra_outputs(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} nx_driver_pico_w)
