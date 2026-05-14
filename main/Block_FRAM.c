#include "Block_FRAM.h"
#include "esp_sleep.h" 

// Объявление переменной в RTC памяти
RTC_DATA_ATTR uint32_t current_point = 0;
bool archive_requested = false;
LogFrame_t data_to_log; // 10 сигналов (float/uint)
DBSled_t DBSled= {0};

#include "esp_attr.h"

// Эта функция будет выполняться из SRAM (максимальная скорость)
// void IRAM_ATTR critical_protection_loop(void) {
    // Логика защиты, которая не должна ждать чтения из Flash
// }

// Данные будут принудительно в SRAM
// DRAM_ATTR uint32_t fast_buffer[256];

#include "esp_cache.h"

// Пример логики (упрощенно):
// Блокировка адреса в кэше инструкций
// esp_cache_msync(addr, size, ESP_CACHE_MSYNC_FLAG_INVALIDATE);
// Для ESP32-C3 конкретные API управления кэшем могут зависеть от версии IDF

#include "esp_efuse.h"
#include "esp_efuse_table.h"

// void read_custom_efuse() {
//     uint8_t mac_custom[6];
//     // Чтение встроенного MAC или пользовательского поля
//     esp_efuse_read_field_blob(ESP_EFUSE_USER_DATA, &mac_custom, 48);
// }