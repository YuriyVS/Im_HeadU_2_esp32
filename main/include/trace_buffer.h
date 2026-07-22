#ifndef TRACE_BUFFER_H
#define TRACE_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_err.h"
#include "mbcontroller.h"

#define TRACE_SAMPLES 1000
#define SLAVE_ADDR    1

// Структура одного сэмпла (8 байт)
typedef struct __attribute__((packed)) {
    float useti;
    float iakb;
} TracePoint_t;

// Структура кольцевого буфера (8008 байт)
typedef struct __attribute__((packed)) {
    volatile uint16_t head;         // Текущий индекс записи (0..999)
    volatile uint8_t  is_running;   // 1 - запись идет, 0 - остановлено
    volatile uint8_t  is_full;      // 1 - сделан хотя бы 1 полный круг
    volatile uint32_t sample_count; // Общий счетчик сэмплов с момента старта
    TracePoint_t      data[TRACE_SAMPLES];
} TraceBuffer_t;

// Прототипы функций
esp_err_t modbus_read_full_trace(TraceBuffer_t *out_trace);
esp_err_t modbus_write_trace_control(bool start);

#endif // TRACE_BUFFER_H