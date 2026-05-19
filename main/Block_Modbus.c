#include "Block_Modbus.h"

//Инициализация UART (GPIO 20, 21)
// void init_uart_communication() {
//     uart_config_t uart_config = {
//         .baud_rate = UART_BAUD_RATE,
//         .data_bits = UART_DATA_8_BITS,
//         .parity    = UART_PARITY_DISABLE,
//         .stop_bits = UART_STOP_BITS_1,
//         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//         .source_clk = UART_SCLK_DEFAULT,
//     };

//     // 1. Сначала настраиваем параметры
//     ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));

//     // 2. Назначаем пины (21-TX, 20-RX)
//     ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, 
//                                  UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

//     // 3. Устанавливаем драйвер. 
//     // Буфер 2КБ хорош, но для Modbus проверьте, не приходят ли пакеты длиннее.
//     ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, 1024 * 2, 0, 0, NULL, 0));

//     // 4. Если это Modbus RS485, обязательно добавьте:
//     ESP_ERROR_CHECK(uart_set_mode(UART_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX));                                 
// }

//Инициализация UART (GPIO 4, 5)
void init_uart_communication() {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // 1. Применяем параметры
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    
    // 2. Назначаем пины
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, 
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // 3. Устанавливаем драйвер (ОБЯЗАТЕЛЬНО ПЕРЕД set_mode)
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, 1024 * 2, 0, 0, NULL, 0));

    // 4. Устанавливаем режим RS485 для Modbus
    ESP_ERROR_CHECK(uart_set_mode(UART_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX));
}

void init_modbus_slave_all_in_one() {
    // 1. Инициализация контроллера
    void* slave_handler = NULL;
    ESP_ERROR_CHECK(mbc_slave_init(MB_PORT_SERIAL_SLAVE, &slave_handler));

    // 2. Настройка параметров связи (заменяет вашу структуру uart_config)
    mb_communication_info_t comm_info = {
        .mode = MB_MODE_RTU,          // RTU режим
        .port = UART_NUM_1,       // Номер порта
        .baudrate = 115200,           // Скорость
        .parity = MB_PARITY_NONE,     // Четность
     };
    
    // Этот вызов сам установит драйвер UART внутри библиотеки
    ESP_ERROR_CHECK(mbc_slave_setup((void*)&comm_info));

    // 3. А вот здесь мы назначаем пины 
    // Библиотека создала драйвер, теперь мы привязываем его к физике
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, 
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // 4. Опционально: если нужно явно подтвердить режим RS485 (для RTS)
    ESP_ERROR_CHECK(uart_set_mode(UART_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX));

    // 5. Регистрация областей данных (Holding Registers и т.д.)
    // ... здесь код привязки структур данных ...

    // 6. Старт
    ESP_ERROR_CHECK(mbc_slave_start());
}


void init_modbus_master() {
    void* master_handler = NULL;
    // 1. Инициализация контроллера для MASTER
    ESP_ERROR_CHECK(mbc_master_init(MB_PORT_SERIAL_MASTER, &master_handler));

    // 2. Настройка параметров связи
    mb_communication_info_t comm_info = {
        .mode = MB_MODE_RTU,
        .port = UART_PORT_NUM,
        .baudrate = 115200,
        .parity = MB_PARITY_NONE
    };

    ESP_ERROR_CHECK(mbc_master_setup((void*)&comm_info));

    // 3. Назначение пинов (как и раньше)
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, 
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // 4. Старт мастера
    ESP_ERROR_CHECK(mbc_master_start());
}

const mb_parameter_descriptor_t device_parameters[] = {
    // { CID, Key, Units, Slave_Addr, Modbus_Func, Reg_Start, Reg_Size, Instance_Offset, Data_Type, Data_Size, Options, Access }
    
    // Параметр 0: Напряжение (Адрес 0x0002, занимает 2 регистра)
    { 
        .cid = PARAM_VOLTAGE_ID, .param_key = "Voltage", .param_units = "V",
        .mb_slave_addr = 1, .mb_param_type = MB_PARAM_HOLDING, .mb_reg_start = 0x0002, .mb_size = 2,
        .param_offset = 0, .param_type = PARAM_TYPE_FLOAT_CDAB, .param_size = PARAM_SIZE_FLOAT,
        .param_opts = { .opt1 = 0 }, .access = PAR_PERMS_READ 
    },

    // Параметр 1: Ток (Адрес 0x0004, занимает следующие 2 регистра)
    { 
        .cid = PARAM_CURRENT_ID, .param_key = "Current", .param_units = "A",
        .mb_slave_addr = 1, .mb_param_type = MB_PARAM_HOLDING, .mb_reg_start = 0x0004, .mb_size = PARAM_SIZE_FLOAT,
        .param_offset = 0, .param_type = PARAM_TYPE_FLOAT_CDAB, .param_size = 4,
        .param_opts = { .opt1 = 0 }, .access = PAR_PERMS_READ 
    },

    // Параметр 2: Частота (Адрес 0x0006, занимает следующие 2 регистра)
    { 
        .cid = PARAM_FREQ_ID, .param_key = "Frequency", .param_units = "Hz",
        .mb_slave_addr = 1, .mb_param_type = MB_PARAM_HOLDING, .mb_reg_start = 0x0006, .mb_size = PARAM_SIZE_FLOAT,
        .param_offset = 0, .param_type = PARAM_TYPE_FLOAT_CDAB, .param_size = 4,
        .param_opts = { .opt1 = 0 }, .access = PAR_PERMS_READ 
    }
};

void read_all_floats_block() {
    uint16_t temp_buffer[100];
    mb_param_request_t request = {
        .slave_addr = SLAVE_ADDR,
        .command    = 0x03,       // Чтение Holding Registers
        .reg_start  = 1000,       // Начало DBMain
        .reg_size   = 100         // 50 float = 100 регистров
    };

    // Читаем сразу в массив вашей локальной копии DBMain
    esp_err_t err = mbc_master_send_request(&request, (void*)temp_buffer);

    if (err == ESP_OK) {
        // Данные получены, но внимание: mbc_master_send_request 
        // НЕ делает автоматический SWAP байтов (CDAB)! 
        // Вам нужно будет пройтись циклом и переставить слова, 
        // либо использовать mbc_master_get_parameter для отдельных значений.
        // 2. Вручную исправляем порядок слов для каждого float (CDAB -> ABCD)
        // Пример безопасного копирования со Swap
        for (int i = 0; i < 50; i++) {
            uint16_t low = temp_buffer[i*2];
            uint16_t high = temp_buffer[i*2 + 1];
            
            // Собираем float правильно, учитывая порядок слов STM32
            uint32_t combined = (high << 16) | low; 
            // Мы явно говорим компилятору: "Я знаю, что делаю"
            memcpy((void*)(uintptr_t)&DBMain.f50.as_array, &combined, sizeof(combined));
        }
    }
}

void read_DB_Main_block_number(uint16_t num) {
    uint16_t temp_buffer[100]; // Максимальный размер блока — 100 регистров
    mb_param_request_t request = {
        .slave_addr = SLAVE_ADDR,
        .command = 0x03 // Используем функцию 03 (Holding Registers), как в STM32
    };

    void* dest_ptr = NULL;
    int elements_count = 0;

    switch (num) {
        case 0: // f50
            request.reg_start = 1000;
            request.reg_size = 100;
            dest_ptr = (void*)(uintptr_t)&DBMain.f50.as_array;
            elements_count = 50;
            break;

        case 2: // f100
            request.reg_start = 1100;
            request.reg_size = 100;
            dest_ptr = (void*)(uintptr_t)&DBMain.f100.as_array;
            elements_count = 50;
            break;

        case 3: // i50
            request.reg_start = 1200;
            request.reg_size = 100;
            dest_ptr = (void*)(uintptr_t)&DBMain.i50.as_array;
            elements_count = 50;
            break;

        case 4: // u50
            request.reg_start = 1300;
            request.reg_size = 100;
            dest_ptr = (void*)(uintptr_t)&DBMain.u50.as_array;
            elements_count = 50;
            break;

        case 5: // b32, b64, b96 (читаем все 3 блока сразу)
            request.reg_start = 1400;
            request.reg_size = 6; // 3 блока по 32 бита = 6 регистров
            dest_ptr = (void*)(uintptr_t)&DBMain.b32; // Начало цепочки bool-блоков
            elements_count = 3;
            break;

        default:
            ESP_LOGW("MODBUS", "Неизвестный номер блока: %d", num);
            return;
    }

    // Выполняем запрос
    esp_err_t err = mbc_master_send_request(&request, (void*)temp_buffer);

    if (err == ESP_OK) {
        // Общий цикл для копирования и исправления порядка слов (CDAB -> ABCD)
        for (int i = 0; i < elements_count; i++) {
            uint16_t low = temp_buffer[i * 2];
            uint16_t high = temp_buffer[i * 2 + 1];

            // Собираем 32-битное значение (переставляем слова для STM32)
            uint32_t combined = (high << 16) | low;

            // Безопасно копируем в DBMain по смещению
            // Используем арифметику указателей (uint32_t*), чтобы сдвигаться на 4 байта за шаг
            memcpy((void*)((uint32_t*)dest_ptr + i), &combined, 4);
        }
        ESP_LOGI("MODBUS", "Блок %d успешно обновлен", num);
    } else {
        ESP_LOGE("MODBUS", "Ошибка чтения блока %d: %s", num, esp_err_to_name(err));
    }
}

/**
 * @brief Чтение группы 32-битных переменных из DBMain
 * @param start_addr Modbus-адрес (через GET_MB_ADDR)
 * @param num_vars Количество 32-битных переменных (float/int32)
 */
void read_DB_Main_start_size(uint16_t start_addr, uint16_t num_vars) {
    if (num_vars > 50) num_vars = 50; // Ограничение буфера (50 * 2 = 100 регистров)

    uint16_t temp_buf[100];
    mb_param_request_t request = {
        .slave_addr = SLAVE_ADDR,
        .command = 0x03,
        .reg_start = start_addr,
        .reg_size = num_vars * 2  // Автоматический перевод в регистры
    };

    if (mbc_master_send_request(&request, (void*)temp_buf) == ESP_OK) {
        // Базовое байтовое смещение от начала DBMain
        uint32_t base_offset = (start_addr - 1000) * 2;
        
        for (int i = 0; i < num_vars; i++) {
            // Сборка 32-битного значения (CDAB -> ABCD)
            uint16_t low = temp_buf[i * 2];
            uint16_t high = temp_buf[i * 2 + 1];
            uint32_t combined = (high << 16) | low;

            // Вычисляем точный адрес записи для i-й переменной
            uint8_t* dest_ptr = (uint8_t*)&DBMain + base_offset + (i * 4);

            // Безопасное копирование в упакованную структуру
            memcpy((void*)(uintptr_t)dest_ptr, &combined, 4);
        }
        ESP_LOGI("MB_SYNC", "Обновлено переменных: %d, Адрес: %d", num_vars, start_addr);
    } else {
        ESP_LOGE("MB_SYNC", "Ошибка связи при чтении %d переменных", num_vars);
    }
}

esp_err_t read_DB_Main_single(uint16_t addr) {
    // Проверка: адрес должен быть не меньше 1000 (база DBMain в STM32)
    if (addr < 1000) {
        ESP_LOGE("MB_SINGLE", "Ошибка: Адрес %d вне диапазона DBMain (1000+)", addr);
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t temp_buf[2]; // Буфер для одного 32-битного параметра (2 регистра)
    mb_param_request_t request = {
        .slave_addr = SLAVE_ADDR,
        .command = 0x03,     // Holding Register
        .reg_start = addr,
        .reg_size = 2        // Всегда читаем 32 бита (2 регистра)
    };

    esp_err_t err = mbc_master_send_request(&request, (void*)temp_buf);

    if (err == ESP_OK) {
        // 1. Вычисляем байтовое смещение внутри DBMain.
        // Каждый инкремент Modbus-адреса — это 2 байта.
        uint32_t byte_offset = (addr - 1000) * 2;

        // 2. Проверка на выход за границы структуры (опционально, размер DB_Main ~412 байт)
        if (byte_offset + 4 > sizeof(DBMain)) {
            ESP_LOGE("MB_SINGLE", "Смещение %d выходит за размер структуры!", byte_offset);
            return ESP_ERR_INVALID_SIZE;
        }

        // 3. Пост-обработка: перестановка слов (CDAB -> ABCD)
        // STM32 при доступе по uint16 сначала отдает младшее слово.
        uint16_t low = temp_buf[0];
        uint16_t high = temp_buf[1];
        uint32_t combined = (high << 16) | low;

        // 4. Безопасная запись в волатильную упакованную структуру по смещению
        // Используем memcpy, чтобы избежать проблем с выравниванием (Alignment)
        uint8_t* dest_addr = (uint8_t*)&DBMain + byte_offset;
        memcpy((void*)(uintptr_t)dest_addr, &combined, 4);

        ESP_LOGI("MB_SINGLE", "Адрес %d обновлен. Смещение: %d, Значение (HEX): 0x%08X", 
                  addr, byte_offset, combined);
    } else {
        ESP_LOGE("MB_SINGLE", "Ошибка чтения адреса %d: %s", addr, esp_err_to_name(err));
    }

    return err;
}