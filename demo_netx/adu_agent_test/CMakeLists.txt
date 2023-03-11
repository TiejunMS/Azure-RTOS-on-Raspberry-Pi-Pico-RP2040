project(adu_agent_test)
set(DEMO_UPDATE demo_update)

include(../flash_loader/flash_loader.cmake)

# build demo_update
add_executable(${DEMO_UPDATE} ../../main.c ../tx_initialize_low_level.S  ../../demo_threadx/demo_threadx.c)
pico_add_extra_outputs(${DEMO_UPDATE})
if (PICO_CYW43_SUPPORTED) # set by PICO_BOARD=pico_w
    target_link_libraries(${DEMO_UPDATE} pico_cyw43_arch_none )
endif()
target_link_libraries(${DEMO_UPDATE} pico_stdlib cmsis_core azrtos::threadx)
set_linker_script(${DEMO_UPDATE} memmap_application.ld)

# generate binary file in c array format
set(DEMO_UPDATE_FILE ${CMAKE_CURRENT_LIST_DIR}/${DEMO_UPDATE}_bin.h)
add_custom_command(OUTPUT ${DEMO_UPDATE_FILE} DEPENDS ${DEMO_UPDATE}
            COMMENT "Generate binary file in c array format"
            COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && 
                    xxd -i ${DEMO_UPDATE}.bin ${DEMO_UPDATE_FILE} && 
                    sed -i 's/unsigned/const unsigned/g' ${DEMO_UPDATE_FILE}
            )
add_custom_target(${DEMO_UPDATE}_bin.h_gen ALL DEPENDS ${DEMO_UPDATE_FILE})

# build adu_agent_test
add_executable(${PROJECT_NAME} ../../main.c adu_agent_test.c ../demo_azure_iot/nx_azure_iot_adu_agent_driver_pico_w.c)
target_link_libraries(${PROJECT_NAME} nx_driver_pico_w)
pico_add_extra_outputs(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${PICO_FLASHLOADER_PATH})
set_linker_script(${PROJECT_NAME} memmap_application.ld)
combine_uf2(${PROJECT_NAME})