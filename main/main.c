#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "esp_task_wdt.h"

#include "mbcontroller.h"

#include "driver/spi_master.h"
#include "driver/spi_common.h"
#include "driver/gpio.h"

#include "driver/uart.h"

#include "DB_Parameters.h"
#include "DB_Main.h"
#include "Block_FRAM.h"

#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 7
#define PIN_NUM_CLK  6
#define PIN_NUM_CS   10

void init_spi_interface();

#define UART_PORT_NUM      UART_NUM_0
#define UART_BAUD_RATE     115200
#define UART_TX_PIN        21
#define UART_RX_PIN        20

void init_uart_communication();

#define GPIO_INT_TO_STM32 3

void init_interrupt_signal();

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Определения приоритетов
#define PRIORITY_SPI_COMM        15  // Критическая связь с STM32
#define PRIORITY_MODBUS_UART     10  // Обработка протокола Modbus
#define PRIORITY_NETWORK_APP      7  // Логика WiFi/Bluetooth
#define PRIORITY_LOG_TASK         1  // Фоновое логирование

// Размеры стеков (в байтах)
#define STACK_SIZE_SPI           4096
#define STACK_SIZE_MODBUS        4096
#define STACK_SIZE_NETWORK       8192 // WiFi требует больше стека

// Заглушки функций задач
void vTaskSPI(void *pvParameters);
void vTaskModbus(void *pvParameters);
void vTaskNetwork(void *pvParameters);

void create_system_tasks(void);

//#include "freertos/semphr.h" 

SemaphoreHandle_t xDataMutex = NULL;

SemaphoreHandle_t spi_sem = NULL;

void app_main(void)
{
    // 1. Сначала системные объекты синхронизации (семафоры, очереди, мьютексы)
    spi_sem = xSemaphoreCreateBinary();
    xDataMutex = xSemaphoreCreateMutex(); // Если будете использовать для работы с DBMain и DBParameters

    // 2. Инициализация СТРУКТУР ДАННЫХ (очень важно сделать ДО задач!)
    DBMain = DBMainInit;
    DBParameters = DBParametersFactory;
    
    // 3. Инициализация ПЕРИФЕРИИ (но без включения прерываний, если можно)
    init_spi_interface();
    init_uart_communication();

    // 4. ЗАПУСК ЗАДАЧ
    // Теперь задачи созданы и "спят" в ожидании ресурсов или событий
    create_system_tasks();

    // 5. ВКЛЮЧЕНИЕ ПРЕРЫВАНИЙ
    // Теперь, если придет прерывание, всё готово к его обработке
    init_interrupt_signal();

    // 6. Основной цикл app_main (работает на приоритете 1)
    while (true) {
        // В ESP-IDF лучше использовать vTaskDelay вместо sleep()
        // для чистоты стиля FreeRTOS
        ESP_LOGI("MAIN", "System is running...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
//Инициализация SPI (GPIO 2, 6, 7, 10)
void init_spi_interface() {
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
        .flags = SPICOMMON_BUSFLAG_IOMUX_PINS // Принудительно используем быстрый путь
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

    spi_device_handle_t spi;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));
}
//Инициализация UART (GPIO 20, 21)
void init_uart_communication() {
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Установка параметров и драйвера
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, 1024 * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    
    // Назначение пинов (Native для UART0)
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, 
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}
//Настройка прерывания для STM32 (GPIO 3)
void init_interrupt_signal() {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,    // Это выход
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << GPIO_INT_TO_STM32),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&io_conf);
    
    // По умолчанию держим в логическом нуле
    gpio_set_level(GPIO_INT_TO_STM32, 0);
}

void create_system_tasks(void) {
    // 1. Задача SPI (Связь с STM32)
    // Используем xTaskCreate. На одноядерном C3 ядро всегда 0.
    xTaskCreate(vTaskSPI, // Функция задачи
                "SPI_Task", // Имя для отладки
                STACK_SIZE_SPI, // Используем 4096 байт
                NULL, // Параметры не передаем
                PRIORITY_SPI_COMM, // Используем приоритет 15
                NULL); // Хендл не сохраняем

    // 2. Задача Modbus RTU
    xTaskCreate(vTaskModbus, 
                "Modbus_Task", 
                STACK_SIZE_MODBUS, 
                NULL, 
                PRIORITY_MODBUS_UART, 
                NULL);

    // 3. Задача сетевых интерфейсов (WiFi + BLE)
    xTaskCreate(vTaskNetwork, 
                "Network_Task", 
                STACK_SIZE_NETWORK, 
                NULL, 
                PRIORITY_NETWORK_APP, 
                NULL);
    
    ESP_LOGI("SYS", "Все задачи системы инициализированы.");
}

// Пример реализации критической задачи
void vTaskSPI(void *pvParameters) {
    // 1. Регистрируем ТЕКУЩУЮ задачу в вочдоге
    esp_task_wdt_add(NULL);
    while (1) {
       
        // Задача засыпает здесь и НЕ потребляет ресурсы процессора.
        // Она будет ждать вечно (portMAX_DELAY), пока семафор не "дадут".
        if (xSemaphoreTake(spi_sem, portMAX_DELAY) == pdTRUE) {
            // Как только мы здесь — значит данные от STM32 ПРИШЛИ!
            ; //process_spi_data(); // Быстро обрабатываем
            // Попытка взять мьютекс. Ждем максимум 10 мс (pdMS_TO_TICKS(10))
            if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                // 2. Сбрасываем таймер сразу после пробуждения
                esp_task_wdt_reset();
                
                // КРИТИЧЕСКАЯ СЕКЦИЯ: Здесь работаем с DBMain
                float new_Useti_from_spi = 0;
                DBMain.f50.Useti = new_Useti_from_spi;
                
                // Обязательно "отдаем" мьютекс обратно!
                xSemaphoreGive(xDataMutex);
            } else {
                // Если за 10 мс мьютекс не освободился — это повод для лога ошибки
                ESP_LOGW("DATA", "Не удалось получить доступ к DBMain (Timeout)");
            }
        }
        //vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

// Пример реализации критической задачи
void vTaskModbus(void *pvParameters) {
    esp_task_wdt_add(NULL);
    mb_event_group_t event_mask;
    //const TickType_t xTicksToWait = portMAX_DELAY; // Ждем события вечно, не потребляя CPU
    //init_modbus_slave();
    while (1) {
        // Ожидание события (аналог обработки прерывания в RTOS)
        // Блокируется до тех пор, пока не придет запрос от Master (STM32)
        event_mask = mbc_slave_check_event(MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD);
        esp_task_wdt_reset();
        if (event_mask & MB_EVENT_HOLDING_REG_WR) {
            // Если STM32 что-то записал в ESP32
            ESP_LOGI("MODBUS", "STM32 обновил параметры управления");
            // Здесь можно добавить логику реакции на новые команды
        }

        if (event_mask & MB_EVENT_HOLDING_REG_RD) {
            // Если STM32 просто считал данные
            // Обновляем наш Heartbeat для следующего цикла
            //slave_data.heartbeat++; 
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Задача ушла спать на 10 мс
    }
}

// Реализация задачи для сетевых интерфейсов (WiFi + Bluetooth)
void vTaskNetwork(void *pvParameters) {
    esp_task_wdt_add(NULL);
    ESP_LOGI("NET", "Задача Network запущена");

    /* Здесь в будущем будет инициализация:
       1. nvs_flash_init()
       2. esp_netif_init()
       3. esp_event_loop_create_default()
       4. Инициализация WiFi или BLE
    */

    while (1) {
        // Пока здесь просто заглушка, чтобы задача не завершалась
        // и не тратила ресурсы процессора
        esp_task_wdt_reset();
        ESP_LOGD("NET", "Ожидание сетевых событий...");
        
        // Задержка 5 секунд для тестов
        vTaskDelay(pdMS_TO_TICKS(5000)); // Вочдог не сработает во время сна!
    }
}
// Обработчик прерывания
// Добавляем IRAM_ATTR, чтобы код жил в RAM и работал максимально быстро
void IRAM_ATTR spi_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // Даем семафор, чтобы разбудить задачу
    xSemaphoreGiveFromISR(spi_sem, &xHigherPriorityTaskWoken);
    
    // Если разбуженная задача имеет более высокий приоритет — 
    // переключаемся на неё мгновенно, не дожидаясь конца тика системы.
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}