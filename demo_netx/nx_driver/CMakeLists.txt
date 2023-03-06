add_library(nx_driver_pico_w nx_driver_pico_w.c)
target_link_libraries(nx_driver_pico_w pico_cyw43_arch_poll pico_stdlib
                      cmsis_core azrtos::netxduo)

target_compile_definitions(pico_cyw43_arch_poll INTERFACE -DCYW43_LWIP_DISABLE)

target_include_directories(nx_driver_pico_w PUBLIC .)
target_compile_definitions(
  nx_driver_pico_w PRIVATE WIFI_SSID=\"${WIFI_SSID}\"
                           WIFI_PASSWORD=\"${WIFI_PASSWORD}\")
