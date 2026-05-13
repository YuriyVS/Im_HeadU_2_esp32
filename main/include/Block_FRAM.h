#ifndef BLOCK_FRAM_H
#define BLOCK_FRAM_H

#include <stdint.h>
#include <stdbool.h>

#include "DB_Main.h"
#include "DB_Parameters.h"


typedef struct __attribute__((packed)) {
    float signals_f[5];    // 5 сигналов Float (20 байт)
    uint32_t signals_u[5]; // 5 сигналов UInt32 (20 байт)
} LogFrame_t;

extern LogFrame_t data_to_log;

typedef struct __attribute__((packed)) {
    uint32_t Archive_Point_Idx;  // Смещение 0 (Регистр 2000)
    uint32_t Archive_Cmd;        // Смещение 2 (Регистр 2002)
    LogFrame_t Archive_Window;   // Смещение 4 (Регистры 2004-2023)
} DBSled_t;

extern DBSled_t DBSled;

#define FRAM_TOTAL_SIZE     0x40000     // 256 KB
#define LOG_RECORD_SIZE     40          // 10 * 4 bytes
#define LOG_POINTS_COUNT    2000
#define BUFFER_SIZE         (LOG_RECORD_SIZE * LOG_POINTS_COUNT) // 80,000 bytes

#define ADDR_RING_BUFFER    0x00000     // Начало кольцевого буфера
#define ADDR_ARCHIVE        0x20000     // Начало архива (смещение 128KB)
#define ADDR_METADATA       0x3FF00     // Хранение текущего индекса

extern uint32_t current_point;
extern bool archive_requested;



#endif
