#ifndef BLOCK_SPI_H
#define BLOCK_SPI_H

#include "driver/spi_master.h"
#include "driver/spi_common.h"
#include "DB_Parameters.h"
#include "DB_Main.h"

#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 7
#define PIN_NUM_CLK  6
#define PIN_NUM_CS   10

extern spi_device_handle_t stm32_spi_handle;
extern void init_spi_interface();

#endif