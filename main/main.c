#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

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

void app_main(void)
{
    init_spi_interface();
    init_uart_communication();
    init_interrupt_signal();
    create_system_tasks();
    DBMain = DBMainInit;
    DBParameters = DBParametersFactory;
    while (true) {
        printf("Hello from app_main!\n");
        sleep(1);
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
    xTaskCreate(vTaskSPI, 
                "SPI_Task", 
                STACK_SIZE_SPI, 
                NULL, 
                PRIORITY_SPI_COMM, 
                NULL);

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
    for (;;) {
        // Выполняем быстрый обмен данными
        // ... логика SPI ...

        // Обязательная блокировка, чтобы дать время другим задачам
        // Если SPI работает по прерыванию, здесь можно использовать семафор
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

// Пример реализации критической задачи
void vTaskModbus(void *pvParameters) {
    mb_event_group_t event_mask;
    
    //init_modbus_slave();
    while (1) {
        // Ожидание события (аналог обработки прерывания в RTOS)
        // Блокируется до тех пор, пока не придет запрос от Master (STM32)
        event_mask = mbc_slave_check_event(MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD);

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
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

// Реализация задачи для сетевых интерфейсов (WiFi + Bluetooth)
void vTaskNetwork(void *pvParameters) {
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
        
        ESP_LOGD("NET", "Ожидание сетевых событий...");
        
        // Задержка 5 секунд для тестов
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
