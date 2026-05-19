#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "esp_task_wdt.h"

#include "Block_GPIO.h"
#include "DB_Parameters.h"
#include "DB_Main.h"
#include "Block_FRAM.h"
#include "Block_SPI.h"
#include "Block_Modbus.h"

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


void vTaskSPI(void *pvParameters);
void vTaskModbus(void *pvParameters);
void vTaskNetwork(void *pvParameters);

void create_system_tasks(void);

//#include "freertos/semphr.h" 

SemaphoreHandle_t xDataMutex = NULL;

SemaphoreHandle_t spi_sem = NULL;

int16_t count_takt = 0;
#define COUNT_LED_MAX 5

void app_main(void)
{
    // 1. Сначала системные объекты синхронизации (семафоры, очереди, мьютексы)
    spi_sem = xSemaphoreCreateBinary();
    xDataMutex = xSemaphoreCreateMutex(); // Если будете использовать для работы с DBMain и DBParameters

    // 2. Инициализация СТРУКТУР ДАННЫХ (очень важно сделать ДО задач!)
    DBMain = DBMainInit;
    DBParameters = DBParametersFactory;
    
    // 3. Инициализация ПЕРИФЕРИИ (но без включения прерываний, если можно)
    init_int_to_stm32_signal();
    init_led();
    //init_spi_interface();
    init_modbus_master();

    // 4. ЗАПУСК ЗАДАЧ
    // Теперь задачи созданы и "спят" в ожидании ресурсов или событий
    create_system_tasks();   

    // 5. Основной цикл app_main (работает на приоритете 1)
    while (true) {
        // В ESP-IDF лучше использовать vTaskDelay вместо sleep()
        // для чистоты стиля FreeRTOS
        ESP_LOGI("MAIN", "System is running...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
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
    //esp_task_wdt_add(NULL);
    while (1) {
       
        // Задача засыпает здесь и НЕ потребляет ресурсы процессора.
        // Она будет ждать вечно (portMAX_DELAY), пока семафор не "дадут".
        // if (xSemaphoreTake(spi_sem, portMAX_DELAY) == pdTRUE) {
        //     // Как только мы здесь — значит данные от STM32 ПРИШЛИ!
        //     ; //process_spi_data(); // Быстро обрабатываем
        //     // Попытка взять мьютекс. Ждем максимум 10 мс (pdMS_TO_TICKS(10))
        //     if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        //         // 2. Сбрасываем таймер сразу после пробуждения
        //         esp_task_wdt_reset();
                
        //         // КРИТИЧЕСКАЯ СЕКЦИЯ: Здесь работаем с DBMain
        //         float new_Useti_from_spi = 0;
        //         DBMain.f50.Useti = new_Useti_from_spi;
                
        //         // Обязательно "отдаем" мьютекс обратно!
        //         xSemaphoreGive(xDataMutex);
        //     } else {
        //         // Если за 10 мс мьютекс не освободился — это повод для лога ошибки
        //         ESP_LOGW("DATA", "Не удалось получить доступ к DBMain (Timeout)");
        //     }
        // }
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}

// Пример реализации критической задачи
// void vTaskModbus(void *pvParameters) {
//     esp_task_wdt_add(NULL);
//     mb_event_group_t event_mask;
//     //const TickType_t xTicksToWait = portMAX_DELAY; // Ждем события вечно, не потребляя CPU
//     //init_modbus_slave();
//     while (1) {
//         // Ожидание события (аналог обработки прерывания в RTOS)
//         // Блокируется до тех пор, пока не придет запрос от Master (STM32)
//         event_mask = mbc_slave_check_event(MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD);
//         esp_task_wdt_reset();
//         if (event_mask & MB_EVENT_HOLDING_REG_WR) {
//             // Если STM32 что-то записал в ESP32
//             ESP_LOGI("MODBUS", "STM32 обновил параметры управления");
//             // Здесь можно добавить логику реакции на новые команды
//         }

//         if (event_mask & MB_EVENT_HOLDING_REG_RD) {
//             // Если STM32 просто считал данные
//             // Обновляем наш Heartbeat для следующего цикла
//             //slave_data.heartbeat++; 
//         }
//         vTaskDelay(pdMS_TO_TICKS(10)); // Задача ушла спать на 10 мс
//     }
// }

void vTaskModbus(void *pvParameters) {
    //float voltage_value = 0;
    //uint8_t type; // Добавляем переменную для типа, которую требует функция
    //const mb_parameter_descriptor_t* param_descr = NULL; // Теперь будем использовать
        
    while (1) {
        // Запрашиваем параметр. Переменная type заполнится автоматически 
        // значением из таблицы дескрипторов (например, PARAM_TYPE_FLOAT)
        // esp_err_t err = mbc_master_get_parameter(PARAM_VOLTAGE_ID, "Voltage", (uint8_t*)&voltage_value, &type);
        
        // if (err == ESP_OK) {
        //     DBMain.f50.Useti = voltage_value;
        // } else {
        //     // Если произошла ошибка, получаем информацию о параметре из таблицы по его CID
        //     if (mbc_master_get_cid_info(PARAM_VOLTAGE_ID, &param_descr) == ESP_OK) {
        //         ESP_LOGE("MODBUS", "Ошибка параметра [%s] (Slave ID: %d, Addr: 0x%04X): %s", 
        //                  param_descr->param_key, 
        //                  param_descr->mb_slave_addr, 
        //                  param_descr->mb_reg_start,
        //                  esp_err_to_name(err));
        //     }
        // }

        //read_DB_Main_block_number(0);
        // read_DB_Main_single(GET_MB_ADDR(DBMain.f50.GenFreq));
        // read_DB_Main_single(GET_MB_ADDR(DBMain.b32));
        // read_DB_Main_start_size(GET_MB_ADDR(DBMain.f50.GenFreq), 10);

        count_takt ++;        
        if(count_takt > COUNT_LED_MAX) {
            // Считываем текущее состояние, инвертируем знаком '!' и записываем обратно
            gpio_set_level(BLINK_GPIO, !gpio_get_level(BLINK_GPIO));
            count_takt = 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); 
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
        ESP_LOGI("NET", "Ожидание сетевых событий...");
        
        // Задержка 1 секунд для тестов
        vTaskDelay(pdMS_TO_TICKS(3000)); // Вочдог не сработает во время сна!
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