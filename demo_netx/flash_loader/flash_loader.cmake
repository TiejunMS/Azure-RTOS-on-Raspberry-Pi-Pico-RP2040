set(PICO_FLASHLOADER_PATH ${CMAKE_CURRENT_LIST_DIR}/../../lib/pico-flashloader)
set(FLASH_LOADER_PATH ${CMAKE_CURRENT_LIST_DIR})

################################################################################
# Helper function
function(set_linker_script TARGET script)
    target_link_directories(${TARGET} PRIVATE ${FLASH_LOADER_PATH})
    target_link_directories(${TARGET} PRIVATE ${PICO_FLASHLOADER_PATH})
    target_include_directories(${TARGET} PRIVATE ${PICO_FLASHLOADER_PATH})
    pico_set_linker_script(${TARGET} ${FLASH_LOADER_PATH}/${script})

    # Add dependencies on the 'included' linker scripts so that the target gets
    # rebuilt if they are changed
    pico_add_link_depend(${TARGET} ${FLASH_LOADER_PATH}/memmap_defines.ld)
    pico_add_link_depend(${TARGET} ${FLASH_LOADER_PATH}/memmap_default.ld)
endfunction()

find_package (Python3 REQUIRED COMPONENTS Interpreter)
# Combine the flashloader and application into one flashable UF2 image
function(combine_uf2 TARGET)
    set(${TARGET} ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.combined.uf2)
    add_custom_command(OUTPUT ${${TARGET}} DEPENDS ${FLASHLOADER} ${TARGET}
            COMMENT "Building full UF2 image"
            COMMAND ${Python3_EXECUTABLE}
                    ${PICO_FLASHLOADER_PATH}/uf2tool.py
                    -o ${${TARGET}} ${FLASHLOADER_UF2} ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.uf2
            )

    add_custom_target(${PROJECT_NAME}_combined ALL DEPENDS ${${TARGET}} ${FLASHLOADER})
endfunction()
################################################################################
# Flashloader
set(FLASHLOADER pico-flashloader)
set(FLASHLOADER_UF2 ${CMAKE_CURRENT_BINARY_DIR}/../flash_loader/${FLASHLOADER}.uf2)