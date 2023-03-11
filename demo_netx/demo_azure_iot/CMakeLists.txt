project(demo_azure_iot)

include(../flash_loader/flash_loader.cmake)

set(MANUFACTURER "RaspberryPi")
set(MODEL "PicoW")
set(INITIAL_VERSION "1.0.0")
set(${PROJECT_NAME}_src
    ../../main.c
    ../tx_initialize_low_level.S
    demo_azure_iot.c
    nx_azure_iot_adu_agent_proxy_simulator_driver.c
    nx_azure_iot_adu_agent_driver_pico_w.c
    nx_azure_iot_cert.c
    nx_azure_iot_ciphersuites.c
    sample_azure_iot_embedded_sdk_adu.c
    sample_azure_iot_embedded_sdk_connect.c
    sample_azure_iot_embedded_sdk_pnp.c
    sample_device_identity.c)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_src})
target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE NX_ENABLE_DHCP
          SAMPLE_DEVICE_INSTALLED_CRITERIA=\"${INITIAL_VERSION}\"
          SAMPLE_DEVICE_MANUFACTURER=\"${MANUFACTURER}\"
          SAMPLE_DEVICE_MODEL=\"${MODEL}\")

# pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_add_extra_outputs(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} nx_driver_pico_w)
target_include_directories(${PROJECT_NAME} PRIVATE ${PICO_FLASHLOADER_PATH})
set_linker_script(${PROJECT_NAME} memmap_application.ld)
combine_uf2(${PROJECT_NAME})

# Build another demo with version 2.0.0
set(UPDATE_VERSION "2.0.0")
set(UPDATE_PROJECT_NAME ${PROJECT_NAME}_${UPDATE_VERSION})
add_executable(${UPDATE_PROJECT_NAME} ${${PROJECT_NAME}_src})
target_compile_definitions(
  ${UPDATE_PROJECT_NAME}
  PRIVATE NX_ENABLE_DHCP SAMPLE_DEVICE_INSTALLED_CRITERIA=\"${UPDATE_VERSION}\"
          SAMPLE_DEVICE_MANUFACTURER=\"${MANUFACTURER}\"
          SAMPLE_DEVICE_MODEL=\"${MODEL}\")

# pico_enable_stdio_usb(${UPDATE_PROJECT_NAME} 1)
pico_add_extra_outputs(${UPDATE_PROJECT_NAME})
target_link_libraries(${UPDATE_PROJECT_NAME} nx_driver_pico_w)
set_linker_script(${UPDATE_PROJECT_NAME} memmap_application.ld)

# Generate manifest file
set(ADU_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/tools/create-adu-import-manifest.sh)
set(MANIFEST_FILE ${UPDATE_PROJECT_NAME}.importmanifest.json)
add_custom_command(OUTPUT ${MANIFEST_FILE} DEPENDS ${UPDATE_PROJECT_NAME}
            COMMENT "Generate manifest file"
            COMMAND ${ADU_SCRIPT}  -c "manufacturer:RaspberryPi" -c "model:PicoW" -p "RaspberryPi" -n "PicoW" -v "2.0.0" -h "microsoft/swupdate:1" -r "installedCriteria:${UPDATE_VERSION}" ${CMAKE_CURRENT_BINARY_DIR}/${UPDATE_PROJECT_NAME}.bin>${CMAKE_CURRENT_BINARY_DIR}/${MANIFEST_FILE}
            )
add_custom_target(${MANIFEST_FILE}_gen ALL DEPENDS ${MANIFEST_FILE})
