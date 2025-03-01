idf_build_get_property(target IDF_TARGET)

set(include_dirs "include" "include/${target}")

set(private_required_comp "")

set(sources "")

if(target STREQUAL "linux")
    list(APPEND sources "${target}/esp_rom_sys.c"
                        "${target}/esp_rom_crc.c"
                        "${target}/esp_rom_md5.c"
                        "${target}/esp_rom_efuse.c")
else()
    list(APPEND include_dirs "${target}")
    list(APPEND sources "patches/esp_rom_crc.c"
                        "patches/esp_rom_sys.c"
                        "patches/esp_rom_uart.c"
                        "patches/esp_rom_spiflash.c"
                        "patches/esp_rom_tjpgd.c"
                        "patches/esp_rom_regi2c.c")
    list(APPEND private_required_comp soc hal)
endif()

if(CONFIG_IDF_TARGET_ARCH_XTENSA)
    list(APPEND sources "patches/esp_rom_longjmp.S")
endif()

idf_component_register(SRCS ${sources}
                       INCLUDE_DIRS ${include_dirs}
                       PRIV_REQUIRES ${private_required_comp}
                       LDFRAGMENTS linker.lf)

if(target STREQUAL "esp32h2")
    if(CONFIG_IDF_TARGET_ESP32H2_BETA_VERSION_1)
        set(ld_folder "ld/rev1")
    elseif(CONFIG_IDF_TARGET_ESP32H2_BETA_VERSION_2)
        set(ld_folder "ld/rev2")
    endif()
else()
    set(ld_folder "ld")
endif()

# Append a target linker script at the target-specific path,
# only the 'name' part is different for each script
function(rom_linker_script name)
    target_linker_script(${COMPONENT_LIB} INTERFACE "${target}/${ld_folder}/${target}.rom.${name}.ld")
endfunction()

if(target STREQUAL "linux")
    # We need to disable some warnings due to the ROM code's printf implementation
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND ${CMAKE_CXX_COMPILER_VERSION} GREATER "7.0.0")
        target_compile_options(${COMPONENT_LIB} PRIVATE -Wimplicit-fallthrough=0 -Wno-shift-count-overflow)
    endif()
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")  # Clang or AppleClang
        target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-integer-overflow -Wno-shift-count-overflow)
    endif()
else()
    target_linker_script(${COMPONENT_LIB} INTERFACE "${target}/${ld_folder}/${target}.rom.ld")
    rom_linker_script("api")
    if(CONFIG_COMPILER_FLOAT_LIB_FROM_GCCLIB)
        rom_linker_script("libgcc")
    else()
        rom_linker_script("rvfp")
    endif()
endif()

idf_build_get_property(time_t_size TIME_T_SIZE)

if(BOOTLOADER_BUILD)
    if(target STREQUAL "esp32")
        rom_linker_script("newlib-funcs")
        if(NOT CONFIG_SPI_FLASH_ROM_DRIVER_PATCH)
            rom_linker_script("spiflash")
        endif()
        if(CONFIG_ESP32_REV_MIN_3)
            rom_linker_script("eco3")
        endif()

    elseif(target STREQUAL "esp32s2")
        rom_linker_script("newlib-funcs")
        rom_linker_script("spiflash")

    elseif(target STREQUAL "esp32s3")
        rom_linker_script("newlib")

    elseif(target STREQUAL "esp32c3")
        rom_linker_script("newlib")

    elseif(target STREQUAL "esp32h2")
        rom_linker_script("newlib")

    elseif(target STREQUAL "esp32c2")
        rom_linker_script("newlib")
        rom_linker_script("mbedtls")
    endif()

else() # Regular app build
    if(target STREQUAL "esp32")
        rom_linker_script("newlib-data")
        rom_linker_script("syscalls")

        if(NOT CONFIG_SPIRAM_CACHE_WORKAROUND)
            # ESP32 only: these ROM functions may only be used if PSRAM cache workaround is disabled.
            # Otherwise we need to link to a multilib version of libc compiled with PSRAM workaround.
            rom_linker_script("newlib-funcs")

            if(time_t_size EQUAL 4)
                # The ROM functions listed in this linker script depend on sizeof(time_t).
                # Since ROM for ESP32 was compiled for 32-bit time_t, only link these functions
                # if the toolchain is also using 32-bit time_t.
                rom_linker_script("newlib-time")

                if(CONFIG_NEWLIB_NANO_FORMAT)
                    # nano formatting functions in ROM are also built for 32-bit time_t.
                    rom_linker_script("newlib-nano")
                endif()
            endif()
        endif()

        if(NOT CONFIG_SPI_FLASH_ROM_DRIVER_PATCH)
            rom_linker_script("spiflash")
        endif()

        if(CONFIG_ESP32_REV_MIN_3)
            rom_linker_script("eco3")
        endif()

    elseif(target STREQUAL "esp32s2")
        rom_linker_script("newlib-funcs")
        rom_linker_script("newlib-data")
        rom_linker_script("spiflash")

        if(time_t_size EQUAL 4)
            # The ROM functions listed in this linker script depend on sizeof(time_t).
            # Since ROM for ESP32-S2 was compiled for 32-bit time_t, only link these functions
            # if the toolchain is also using 32-bit time_t.
            rom_linker_script("newlib-time")

            if(CONFIG_NEWLIB_NANO_FORMAT)
                # nano formatting functions in ROM are also built for 32-bit time_t.
                rom_linker_script("newlib-nano")
            endif()
        endif()

    elseif(target STREQUAL "esp32s3")
        rom_linker_script("newlib")
        rom_linker_script("version")

        if(time_t_size EQUAL 4)
            # The ROM functions listed in this linker script depend on sizeof(time_t).
            # Since ROM for ESP32-S3 was compiled for 32-bit time_t, only link these functions
            # if the toolchain is also using 32-bit time_t.
            rom_linker_script("newlib-time")

            if(CONFIG_NEWLIB_NANO_FORMAT)
                # nano formatting functions in ROM are also built for 32-bit time_t.
                rom_linker_script("newlib-nano")
            endif()
        endif()

    elseif(target STREQUAL "esp32c3")
        rom_linker_script("newlib")
        rom_linker_script("version")

        if(time_t_size EQUAL 4)
            # The ROM functions listed in this linker script depend on sizeof(time_t).
            # Since ROM for ESP32-C3 was compiled for 32-bit time_t, only link these functions
            # if the toolchain is also using 32-bit time_t.
            rom_linker_script("newlib-time")

            if(CONFIG_NEWLIB_NANO_FORMAT)
                # nano formatting functions in ROM are also built for 32-bit time_t.
                rom_linker_script("newlib-nano")
            endif()
        endif()

        if(CONFIG_ESP32C3_REV_MIN_3 OR CONFIG_ESP32C3_REV_MIN_4)
            rom_linker_script("eco3")
        endif()

    elseif(target STREQUAL "esp32h2")
        rom_linker_script("newlib")
        rom_linker_script("version")

        if(CONFIG_NEWLIB_NANO_FORMAT AND time_t_size EQUAL 4)
            # nano formatting functions in ROM are built for 32-bit time_t,
            # only link them if the toolchain is also using 32-bit time_t and nano formatting was requested.
            rom_linker_script("newlib-nano")
        endif()

    elseif(target STREQUAL "esp32c2")
        rom_linker_script("newlib")
        rom_linker_script("version")
        rom_linker_script("mbedtls")

        if(time_t_size EQUAL 8)
            # The ROM functions listed in this linker script depend on sizeof(time_t).
            # Since ROM for ESP32-C2 was compiled for 64-bit time_t, only link these functions
            # if the toolchain is also using 64-bit time_t.
            rom_linker_script("newlib-time")

            if(CONFIG_NEWLIB_NANO_FORMAT)
                # nano formatting functions in ROM are also built for 64-bit time_t.
                rom_linker_script("newlib-nano")
            endif()
        endif()

        if(CONFIG_HEAP_TLSF_USE_ROM_IMPL)
            rom_linker_script("heap")
        endif()
    endif()

    if(CONFIG_IDF_TARGET_ARCH_XTENSA)
        target_link_libraries(${COMPONENT_LIB} INTERFACE "-Wl,--wrap=longjmp")
    endif()
endif()

if(target STREQUAL "esp32s2")
    target_sources(${COMPONENT_LIB} PRIVATE "esp32s2/usb_descriptors.c")
endif()
