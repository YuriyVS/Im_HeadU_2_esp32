#ifndef BLOCK_MODBUS_H
#define BLOCK_MODBUS_H

#include "driver/uart.h"
#include "mbcontroller.h"
#include "DB_Parameters.h"
#include "DB_Main.h"
#include "event_log.h"


#define UART_BAUD_RATE     115200
// #define UART_PORT_NUM      UART_NUM_0
// #define UART_TX_PIN        21
// #define UART_RX_PIN        20

#define UART_PORT_NUM      UART_NUM_1  // Используем второй UART
#define UART_TX_PIN        4           // Подключается к RX платы STM32
#define UART_RX_PIN        5           // Подключается к TX платы STM32
#define SLAVE_ADDR         1
// Макрос для получения Modbus-адреса любого поля структуры DBMain
#define GET_MB_ADDR(member) (uint16_t)(1000 + ((uint8_t*)&(member) - (uint8_t*)&DBMain) / 2)

extern void init_uart_communication();
extern void init_modbus_slave_all_in_one();
extern void init_modbus_master();
extern void read_all_floats_block();
extern void read_DB_Main_block_number(uint16_t num);
extern esp_err_t modbus_read_log_page(uint16_t start_depth, uint8_t req_count, LogEntry_t *out_logs, uint8_t *out_fetched); // Вычитка страницы журнала за 1 запрос Modbus
extern void read_DB_Main_start_size(uint16_t start_addr, uint16_t num_vars);
extern esp_err_t read_DB_Main_single(uint16_t addr);
extern esp_err_t modbus_write_param_32(uint16_t reg_start, uint32_t val32);
extern const mb_parameter_descriptor_t device_parameters[];


enum {
    PARAM_VOLTAGE_ID = 0,    // ID для напряжения
    PARAM_CURRENT_ID,        // ID для тока (автоматически станет 1)
    PARAM_FREQ_ID,           // ID для частоты (2)
    PARAM_MAX_COUNT          // Общее количество параметров
};

#endif