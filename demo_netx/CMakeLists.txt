cmake_minimum_required(VERSION 3.12)

if (PICO_CYW43_SUPPORTED) # set by PICO_BOARD=pico_w
    if (NOT TARGET pico_cyw43_arch)
        message("Skipping Pico W examples as support is not available")
    else()

        if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
            set(WIFI_SSID $ENV{WIFI_SSID})
            message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
        endif()

        if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
            set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
            message("Using WIFI_PASSWORD from environment")
        endif()

        set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
        set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")

        set(NXD_ENABLE_FILE_SERVERS OFF CACHE INTERNAL "Includes a dependency on FileX to support 'server' protocol handlers")
        set(NXD_ENABLE_AZURE_IOT ON CACHE INTERNAL "Enable Azure IoT")
        add_subdirectory(../lib/netxduo netxduo)
        add_subdirectory(nx_driver)
        add_subdirectory(demo_netx_ping)
        add_subdirectory(demo_azure_iot)

        target_compile_definitions(netxduo INTERFACE
                                   -DNXD_MQTT_CLOUD_ENABLE
                                   -DNX_SECURE_ENABLE
                                   -DNX_ENABLE_IP_PACKET_FILTER
                                   -DNX_ENABLE_EXTENDED_NOTIFY_SUPPORT
                                   -DSAMPLE_NETWORK_DRIVER=nx_driver_pico_w
                                   -DSAMPLE_DEVICE_MANUFACTURER=\"RaspberryPi\"
                                   -DSAMPLE_DEVICE_MODEL=\"PicoW\"
                                   )
    endif()
endif()
