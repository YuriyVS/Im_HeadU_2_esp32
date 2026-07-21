#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <stdint.h>
#include <stdbool.h>
#include "DB_Parameters.h"
#include "DB_Main.h"

// Емкость кольцевого буфера в SRAM1 (256 записей = 4096 байт)
#define LOG_MAX_ENTRIES  256

// Уровни важности событий (Severity)
typedef enum {
    LOG_SEV_INFO    = 0,
    LOG_SEV_WARNING = 1,
    LOG_SEV_ALARM   = 2
} LogSeverity_t;

// Коды типовых событий
#define LOG_EV_SYS_INIT        1000  // Старт системы
#define LOG_EV_PARAM_CHANGED   1001  // Изменена уставка
#define LOG_EV_OVERVOLTAGE     2001  // Перенапряжение
#define LOG_EV_OVERCURRENT     2002  // Перегрузка по току
#define LOG_EV_COMM_FAULT      3001  // Ошибка связи Modbus

// Структура единичной записи журнала (16 байт)
typedef struct __attribute__((packed)) {
    uint32_t timestamp;   // Время события (HAL_GetTick или RTC)
    uint16_t event_id;    // Код события
    uint8_t  severity;    // Уровень (0=INFO, 1=WARN, 2=ALARM)
    uint8_t  flags;       // Резерв / Флаги
    float    value;       // Значение параметра
    uint32_t param_id;    // Индекс / дополнительный код
} LogEntry_t;

// Структура кольцевого буфера журнала
typedef struct {
    uint32_t head;                     // Индекс для следующей записи (0..255)
    uint32_t count;                    // Текущее количество записей в буфере (max 256)
    uint32_t total_recorded;           // Всего зарегистрировано событий от старта
    LogEntry_t entries[LOG_MAX_ENTRIES]; // Массив записей в SRAM1
} LogBuffer_t;

// Внешний доступ к буферу журнала
extern volatile LogBuffer_t EventLogBuffer;

#endif


