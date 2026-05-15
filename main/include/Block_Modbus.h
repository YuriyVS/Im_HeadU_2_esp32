#ifndef BLOCK_MODBUS_H
#define BLOCK_MODBUS_H

#include "driver/uart.h"
#include "mbcontroller.h"
#include "DB_Parameters.h"
#include "DB_Main.h"

#define UART_PORT_NUM      UART_NUM_0
#define UART_BAUD_RATE     115200
#define UART_TX_PIN        21
#define UART_RX_PIN        20

extern void init_uart_communication();

#endif