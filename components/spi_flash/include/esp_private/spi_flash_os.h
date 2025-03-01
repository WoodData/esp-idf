/*
 * SPDX-FileCopyrightText: 2019-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * System level MSPI APIs (private)
 */
/**
 * Currently the MSPI timing tuning related APIs are designed to be private.
 * Because:
 * 1. now we don't split SPI0 and SPI1, we don't have a component for SPI0, including PSRAM, Cache, etc..
 * 2. SPI0 and SPI1 are strongly coupling.
 *
 * In the future, we may consider creating a component for SPI0, and spi_flash component will only work on SPI1 (and it
 * can rely on SPI0). Therefore, we can put these APIs there.
 *
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "sdkconfig.h"
#include "esp_rom_spiflash.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "hal/spi_flash_hal.h"
#include "spi_flash_override.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief To setup Flash chip
 */
esp_err_t spi_flash_init_chip_state(void);

/**
 * @brief Make MSPI work under 20Mhz, remove the timing tuning required delays.
 * @param control_spi1  Select whether to control SPI1. For tuning, we need to use SPI1. After tuning (during startup stage), let the flash driver to control SPI1
 */
void spi_timing_enter_mspi_low_speed_mode(bool control_spi1);

/**
 * @brief Make MSPI work under the frequency as users set, may add certain delays to MSPI RX direction to meet timing requirements.
 * @param control_spi1  Select whether to control SPI1. For tuning, we need to use SPI1. After tuning (during startup stage), let the flash driver to control SPI1
 */
void spi_timing_enter_mspi_high_speed_mode(bool control_spi1);

/**
 * @brief Switch MSPI into low speed mode / high speed mode.
 * @note This API is cache safe, it will freeze both D$ and I$ and restore them after MSPI is switched
 * @note For some of the MSPI high frequency settings (e.g. 80M DDR mode Flash or PSRAM), timing tuning is required.
 *       Certain delays will be added to the MSPI RX direction. When CPU clock switches from PLL to XTAL, should call
 *       this API first to enter MSPI low speed mode to remove the delays, and vice versa.
 */
void spi_timing_change_speed_mode_cache_safe(bool switch_down);

/**
 * @brief Tune MSPI flash timing to make it work under high frequency
 */
void spi_timing_flash_tuning(void);

/**
 * @brief Tune MSPI psram timing to make it work under high frequency
 */
void spi_timing_psram_tuning(void);

/**
 * @brief To initislize the MSPI pins
 */
void esp_mspi_pin_init(void);

/**
 * @brief Set SPI1 registers to make ROM functions work
 * @note This function is used for setting SPI1 registers to the state that ROM SPI functions work
 */
void spi_flash_set_rom_required_regs(void);

/**
 * @brief Initialize main flash
 * @param chip Pointer to main SPI flash(SPI1 CS0) chip to use..
 */
esp_err_t esp_flash_init_main(esp_flash_t *chip);

/**
 * @brief Should be only used by SPI1 Flash driver to know the necessary timing registers
 * @param out_timing_config Pointer to timing_tuning parameters.
 */
void spi_timing_get_flash_timing_param(spi_flash_hal_timing_config_t *out_timing_config);

/**
 * @brief Get the knowledge if the MSPI timing is tuned or not
 */
bool spi_timing_is_tuned(void);

/**
 * @brief Set Flash chip specifically required MSPI register settings here
 */
void spi_flash_set_vendor_required_regs(void);

/**
 * @brief Judge whether need to reset flash when brownout.
 *        Set` flash_brownout_needs_reset` inside the function if really need reset.
 */
void spi_flash_needs_reset_check(void);

/**
 * @brief Set flag to reset flash. set when erase chip or program chip
 *
 * @param bool status. True if flash is eraing. False if flash is not erasing.
 *
 * @return None.
 */
void spi_flash_set_erasing_flag(bool status);

/**
 * @brief Judge whether need to reset flash when brownout.
 *
 * @return true if need reset, otherwise false.
 */
bool spi_flash_brownout_need_reset(void);

/**
 * @brief Enable SPI flash high performance mode.
 *
 * @return ESP_OK if success.
 */
esp_err_t spi_flash_enable_high_performance_mode(void);

/**
 * @brief Get the flash dummy through this function
 *        This can be used when one flash has several dummy configurations to enable the high performance mode.
 * @note Don't forget to subtract one when assign to the register of mspi e.g. if the value you get is 4, (4-1=3) should be assigned to the register.
 *
 * @return Pointer to bootlaoder_flash_dummy_conf_t.
 */
const spi_flash_hpm_dummy_conf_t *spi_flash_get_dummy(void);

#ifdef __cplusplus
}
#endif
