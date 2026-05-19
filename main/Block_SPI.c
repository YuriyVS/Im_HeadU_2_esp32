#include "Block_SPI.h"

spi_device_handle_t stm32_spi_handle= NULL;
//Инициализация SPI (GPIO 2, 6, 7, 10)
void init_spi_interface() {
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
        .flags = SPICOMMON_BUSFLAG_IOMUX_PINS // Принудительно используем быстрый путь // Если пины не родные (не 6, 2, 7), замените на 0
    };

    // Инициализация шины SPI
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Если ESP32 — ведущий (Master), настраиваем параметры устройства
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000, // 10 MHz
        .mode = 0,                          // SPI Mode 0
        .spics_io_num = PIN_NUM_CS,         // Hardware CS
        .queue_size = 7,
    };

    //spi_device_handle_t spi;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &stm32_spi_handle));
}