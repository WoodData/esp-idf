idf_build_get_property(target IDF_TARGET)

set(requires soc)
set(priv_requires efuse spi_flash bootloader_support)

set(srcs "compare_set.c" "cpu.c" "esp_memory_utils.c")
if(NOT BOOTLOADER_BUILD)
    list(APPEND srcs "esp_async_memcpy.c"
                     "esp_clk.c"
                     "clk_ctrl_os.c"
                     "hw_random.c"
                     "intr_alloc.c"
                     "mac_addr.c"
                     "periph_ctrl.c"
                     "rtc_module.c"
                     "sleep_modes.c"
                     "sleep_gpio.c"
                     "sleep_mac_bb.c"
                     "regi2c_ctrl.c")
    if(NOT CONFIG_IDF_TARGET_ESP32 AND NOT CONFIG_IDF_TARGET_ESP32S2)
        list(APPEND srcs "sleep_retention.c")
    endif()

    # [refactor-todo]: requires "driver" for GPIO and RTC  (by sleep_gpio and sleep_modes)
    list(APPEND priv_requires driver esp_timer)

    if(CONFIG_IDF_TARGET_ESP32 OR CONFIG_IDF_TARGET_ESP32S2)
        list(APPEND srcs "rtc_wdt.c")
    endif()

else()
    # Requires "_esp_error_check_failed()" function
    list(APPEND priv_requires "esp_system")
endif()

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS include include/soc include/soc/${target}
                       PRIV_INCLUDE_DIRS port/include include/esp_private
                       REQUIRES ${requires}
                       PRIV_REQUIRES "${priv_requires}"
                       LDFRAGMENTS linker.lf)

idf_build_get_property(target IDF_TARGET)
add_subdirectory(port/${target})

if(NOT BOOTLOADER_BUILD)
    if(CONFIG_SPIRAM)
        idf_component_optional_requires(PRIVATE esp_psram)
    endif()
endif()
