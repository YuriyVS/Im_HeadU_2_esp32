#include "trace_buffer.h"

// =============================================================================
// 1. Чтение заголовка буфера (Адрес 3000, 4 регистра = 8 байт)
// ПРИМЕЧАНИЕ: Этот запрос триггерит создание Snapshot на STM32!
// =============================================================================
static esp_err_t modbus_read_trace_header(TraceBuffer_t *out_trace) 
{
    uint16_t rx_regs[4];
    mb_param_request_t req = {
        .slave_addr = SLAVE_ADDR,
        .command    = 0x03,   // Read Holding Registers
        .reg_start  = 3000,   // Адрес заголовка
        .reg_size   = 4       // 4 регистра (8 байт)
    };

    esp_err_t err = mbc_master_send_request(&req, rx_regs);
    if (err != ESP_OK) return err;

    // Reg 3000: head (uint16_t)
    out_trace->head       = rx_regs[0];

    // Reg 3001: High byte = is_running, Low byte = is_full
    out_trace->is_running = (rx_regs[1] >> 8) & 0xFF;
    out_trace->is_full    = rx_regs[1] & 0xFF;

    // Reg 3002..3003: sample_count (uint32_t)
    out_trace->sample_count = ((uint32_t)rx_regs[2] << 16) | rx_regs[3];

    return ESP_OK;
}

// =============================================================================
// 2. Вычитка пачки точек (До 30 точек = 120 регистров = 240 байт за 1 запрос)
// =============================================================================
static esp_err_t modbus_read_trace_chunk(uint16_t start_point, uint8_t req_points, TraceBuffer_t *out_trace) 
{
    if (req_points > 30) req_points = 30; // Лимит кадра Modbus (120 регистров)

    uint16_t rx_regs[120];
    mb_param_request_t req = {
        .slave_addr = SLAVE_ADDR,
        .command    = 0x03,
        .reg_start  = 3010 + (start_point * 4), // 4 регистра на точку (2x float)
        .reg_size   = req_points * 4
    };

    esp_err_t err = mbc_master_send_request(&req, rx_regs);
    if (err != ESP_OK) return err;

    // Заполняем массив data в структуре out_trace
    for (uint8_t i = 0; i < req_points; i++) {
        uint16_t *r = &rx_regs[i * 4];
        uint16_t global_idx = start_point + i;

        // 1. Useti (float)
        uint32_t u_raw = ((uint32_t)r[0] << 16) | r[1];
        memcpy((void*)&out_trace->data[global_idx].useti, &u_raw, sizeof(float));

        // 2. Iakb (float)
        uint32_t i_raw = ((uint32_t)r[2] << 16) | r[3];
        memcpy((void*)&out_trace->data[global_idx].iakb, &i_raw, sizeof(float));
    }

    return ESP_OK;
}

// =============================================================================
// 3. Главная функция: Заполнение структуры TraceBuffer_t (1000 точек)
// =============================================================================
esp_err_t modbus_read_full_trace(TraceBuffer_t *out_trace) 
{
    // Шаг 1: Заполняем заголовок структуры (head, is_running, is_full, sample_count)
    esp_err_t err = modbus_read_trace_header(out_trace);
    if (err != ESP_OK) return err;

    // Шаг 2: Вычитываем 1000 сэмплов в out_trace->data (34 запроса)
    uint16_t fetched = 0;
    while (fetched < TRACE_SAMPLES) {
        uint8_t points_to_read = (TRACE_SAMPLES - fetched > 30) ? 30 : (TRACE_SAMPLES - fetched);

        err = modbus_read_trace_chunk(fetched, points_to_read, out_trace);
        if (err != ESP_OK) return err;

        fetched += points_to_read;
    }

    return ESP_OK;
}

// =============================================================================
// 4. Отправка команды управления (Старт / Стоп)
// =============================================================================
esp_err_t modbus_write_trace_control(bool start) 
{
    uint16_t cmd_val = start ? 1 : 0;
    
    mb_param_request_t req = {
        .slave_addr = SLAVE_ADDR,
        .command    = 0x10,   // Write Multiple Registers
        .reg_start  = 3000,   // Адрес тумблера
        .reg_size   = 1
    };

    return mbc_master_send_request(&req, &cmd_val);
}