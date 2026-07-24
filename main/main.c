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
#include "Block_Network.h"
#include "web_server.h"
#include "event_log.h"
#include "trace_buffer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_http_client.h"

// Access Token из вашего кабинета ThingsBoard
#define TB_ACCESS_TOKEN "fjvB6ijKGxSdO26AkjCZ"
#define TB_URL "http://thingsboard.cloud/api/v1/" TB_ACCESS_TOKEN "/telemetry"

// Учетные данные вашего роутера для выхода в интернет
#define ROUTER_WIFI_SSID "A05sYVS"
#define ROUTER_WIFI_PASS "6afv942dwnxim5s"

// Глобальный флаг успешного получения IP от роутера
static bool s_sta_connected = false;
static bool s_sta_has_ip = false;

// Определения приоритетов
#define PRIORITY_SPI_COMM        10  // Критическая связь с STM32
#define PRIORITY_MODBUS_UART     24  // Обработка протокола Modbus
#define PRIORITY_NETWORK_APP      7  // Логика WiFi/Bluetooth
#define PRIORITY_LOG_TASK         1  // Фоновое логирование

// Размеры стеков (в байтах)
#define STACK_SIZE_SPI           4096
#define STACK_SIZE_MODBUS        4096
#define STACK_SIZE_NETWORK       8192 // WiFi требует больше стека


void vTaskSPI(void *pvParameters);
void vTaskModbus(void *pvParameters);
void vTaskNetwork(void *pvParameters);
void vTaskThingsBoard(void *pvParameters);

void create_system_tasks(void);

//#include "freertos/semphr.h" 

SemaphoreHandle_t xDataMutex = NULL;

SemaphoreHandle_t spi_sem = NULL;

int16_t count_takt = 0;
#define COUNT_LED_MAX 10

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

    // 4. Задача отправки телеметрии в ThingsBoard Cloud
    // xTaskCreate(vTaskThingsBoard, 
    //             "TB_Task", 
    //             6144, 
    //             NULL, 
    //             3, 
    //             NULL); 

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

        // read_DB_Main_block_number(count_takt);

        // read_DB_Main_single(GET_MB_ADDR(DBMain.f50.GenFreq));
        // read_DB_Main_single(GET_MB_ADDR(DBMain.b32));
        // read_DB_Main_start_size(GET_MB_ADDR(DBMain.f50.GenFreq), 10);
        //read_DB_Main_single(GET_MB_ADDR(DBMain.f50.UsetiV));
        //read_DB_Main_single(GET_MB_ADDR(DBMain.f50.IakbA));

        count_takt ++;        
        if(count_takt >= COUNT_LED_MAX) {
            // Считываем текущее состояние, инвертируем знаком '!' и записываем обратно
            gpio_set_level(BLINK_GPIO, !gpio_get_level(BLINK_GPIO));
            count_takt = 0;
            ESP_LOGI("MODBUS", "Напряжение сети: %.2f В", DBMain.f50.UsetiV);
            ESP_LOGI("MODBUS", "Iakb: %.2f A", DBMain.f50.IakbA);
            ESP_LOGI("MODBUS", "b32 raw: 0x%08X", (unsigned int)DBMain.b32.all);
            ESP_LOGI("MODBUS", "b64 raw: 0x%08X", (unsigned int)DBMain.b64.all);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

// Реализация задачи для сетевых интерфейсов (WiFi + Bluetooth)
// void vTaskNetwork(void *pvParameters) {
//     esp_task_wdt_add(NULL);
//     ESP_LOGI("NET", "Задача Network запущена");

//     /* Здесь в будущем будет инициализация:
//        1. nvs_flash_init()
//        2. esp_netif_init()
//        3. esp_event_loop_create_default()
//        4. Инициализация WiFi или BLE
//     */

//     while (1) {
//         // Пока здесь просто заглушка, чтобы задача не завершалась
//         // и не тратила ресурсы процессора
//         esp_task_wdt_reset();
//         ESP_LOGI("NET", "Ожидание сетевых событий...");
        
//         // Задержка 1 секунд для тестов
//         vTaskDelay(pdMS_TO_TICKS(3000)); // Вочдог не сработает во время сна!
//     }
// }
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

// Обработчик событий: пишет в лог, когда кто-то подключается или отключается от ESP32
// static void wifi_event_handler(void* arg, esp_event_base_t event_base,
//                                int32_t event_id, void* event_data) {
//     // --- События SoftAP (Точка доступа) ---
//     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
//         wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
//         ESP_LOGI("NET", "Смартфон подключился к AP, MAC: " MACSTR ", AID=%d",
//                  MAC2STR(event->mac), event->aid);
//     } 
//     else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
//         wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
//         ESP_LOGI("NET", "Смартфон отключился от AP, MAC: " MACSTR ", AID=%d",
//                  MAC2STR(event->mac), event->aid);
//     }
    
//     // --- События STA (Клиент роутера) ---
//     else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
//         ESP_LOGI("NET", "Wi-Fi STA запущен, пробуем подключиться к роутеру...");
//         esp_wifi_connect();
//     } 
//     else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
//         s_sta_connected = false;
//         s_sta_has_ip = false;
//         ESP_LOGW("NET", "Соединение с роутером отсутствует. Следующая попытка по таймеру.");
//     } 
//     else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
//         ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
//         ESP_LOGI("NET", "Получен IP от роутера: " IPSTR, IP2STR(&event->ip_info.ip));
//         s_sta_connected = true;
//         s_sta_has_ip = true;
//     }
// }

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI("NET", "Устройство подключилось, MAC: " MACSTR ", AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI("NET", "Устройство отключилось, MAC: " MACSTR ", AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

// void vTaskNetwork(void *pvParameters) {
//     ESP_LOGI("NET", "Задача Network запущена. Инициализация Wi-Fi AP+STA...");

//     // 1. Инициализация NVS флеш-памяти
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

//     // 2. Инициализация LwIP и системных петлей событий
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());

//     esp_netif_create_default_wifi_ap();
//     esp_netif_create_default_wifi_sta();

//     // 3. Конфигурация Wi-Fi стека
//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_wifi_init(&cfg));

//     // 4. Регистрация обработчиков событий
//     ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
//                                                         ESP_EVENT_ANY_ID,
//                                                         &wifi_event_handler,
//                                                         NULL,
//                                                         NULL));
//     ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
//                                                         IP_EVENT_STA_GOT_IP,
//                                                         &wifi_event_handler,
//                                                         NULL,
//                                                         NULL));

//     // 5. Конфигурация AP (Отлаженная настройка SoftAP)
//     wifi_config_t ap_config = {
//         .ap = {
//             .ssid = WIFI_AP_SSID,
//             .ssid_len = strlen(WIFI_AP_SSID),
//             .channel = WIFI_AP_CHANNEL,
//             .password = WIFI_AP_PASS,
//             .max_connection = MAX_STA_CONN,
//             .authmode = WIFI_AUTH_WPA2_PSK,
//             .pmf_cfg = {
//                 .required = false,
//             },
//         },
//     };
//     if (strlen(WIFI_AP_PASS) == 0) {
//         ap_config.ap.authmode = WIFI_AUTH_OPEN; 
//     }

//     // 6. Конфигурация STA (Подключение к роутеру)
//     wifi_config_t sta_config = {
//         .sta = {
//             .ssid = ROUTER_WIFI_SSID,
//             .password = ROUTER_WIFI_PASS,
//             .threshold.authmode = WIFI_AUTH_WPA2_PSK,
//         },
//     };

//     // 7. Установка совмещенного режима APSTA и старт
//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
//     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
//     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
//     ESP_ERROR_CHECK(esp_wifi_start());

//     // Фиксация ширины канала 20 МГц для Точки Доступа
//     ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20));

//     configure_ap_ip(); 
//     ESP_LOGI("NET", "Точка доступа поднята с IP 192.168.10.1 и SSID: %s", WIFI_AP_SSID);

//     start_webserver();

//     // Подключение вочдога после инициализации
//     esp_task_wdt_add(NULL);

//     uint32_t reconnect_counter = 0;

//     while (1) {
//         esp_task_wdt_reset();

//         // Если связь с роутером потеряна, делаем попытку реконнекта раз в 20 секунд
//         if (!s_sta_connected) {
//             reconnect_counter++;
//             if (reconnect_counter >= 20) {
//                 reconnect_counter = 0;
//                 ESP_LOGI("NET", "Попытка повторного подключения к роутеру...");
//                 esp_wifi_connect();
//             }
//         } else {
//             reconnect_counter = 0;
//         }

//         vTaskDelay(pdMS_TO_TICKS(1000)); 
//     }
// }

void vTaskNetwork(void *pvParameters) {
    ESP_LOGI("NET", "Задача Network запущена. Старт инициализации Wi-Fi SoftAP...");

    // 1. Инициализация NVS флеш-памяти (ОБЯЗАТЕЛЬНО для Wi-Fi, там хранятся калибровки радио)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Инициализация базового сетевого интерфейса (LwIP)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    // 3. Конфигурация Wi-Fi стека
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 4. Регистрация обработчика событий (чтобы видеть подключения в логах)
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    // 5. Настройка параметров нашей точки доступа
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .channel = WIFI_AP_CHANNEL,
            .password = WIFI_AP_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK, // Защита WPA2
            .pmf_cfg = {
                .required = false,
            },
        },
    };
    
    // Если пароль не задан, делаем сеть открытой
    if (strlen(WIFI_AP_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN; 
    }

    // 6. Установка режима AP и запуск
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // ДОБАВИТЬ СТРОКУ ТУТ: Принудительно ставим ширину канала 20 МГц для Точки Доступа
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20));

    //ESP_LOGI("NET", "Точка доступа успешно поднята! SSID: %s", WIFI_AP_SSID);
    // Вставляем настройку IP "на лету"
    configure_ap_ip(); 

    // Логируем результат
    ESP_LOGI("NET", "Точка доступа поднята с кастомным IP и SSID: %s", WIFI_AP_SSID);

    start_webserver();

    // ПОДКЛЮЧАЕМ ВОЧДОГ ЗДЕСЬ: инициализация радио завершена, теперь можно следить за циклом
    esp_task_wdt_add(NULL);

    while (1) {
        // Сброс вочдога
        esp_task_wdt_reset();
        
        // Используем LOGD (Debug), чтобы не спамить в консоль каждую секунду
        //ESP_LOGD("NET", "Ожидание сетевых событий...");
        
        // Задержка 
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}

void vTaskThingsBoard(void *pvParameters) {
    ESP_LOGI("TB_HTTP", "Задача ThingsBoard Cloud запущена.");

    char json_payload[256];

    esp_http_client_config_t config = {
        .url = TB_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    while (1) {
        // Отправляем телеметрию только если получен IP от роутера
        if (s_sta_has_ip) {
            if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                snprintf(json_payload, sizeof(json_payload),
                    "{"
                      "\"UsetiV\":%.2f,"
                      "\"IakbA\":%.2f,"
                      "\"GenFreq\":%.2f,"
                      "\"b32\":%u"
                    "}",
                    DBMain.f50.UsetiV,
                    DBMain.f50.IakbA,
                    DBMain.f50.GenFreq,
                    (unsigned int)DBMain.b32.all
                );
                xSemaphoreGive(xDataMutex);

                esp_http_client_set_post_field(client, json_payload, strlen(json_payload));
                esp_err_t err = esp_http_client_perform(client);

                if (err == ESP_OK) {
                    int status_code = esp_http_client_get_status_code(client);
                    if (status_code == 200) {
                        ESP_LOGI("TB_HTTP", "Успешно отправлено в ThingsBoard [%d]: %s", status_code, json_payload);
                    } else {
                        ESP_LOGW("TB_HTTP", "Отклик сервера: HTTP %d", status_code);
                    }
                } else {
                    ESP_LOGE("TB_HTTP", "Ошибка HTTP: %s", esp_err_to_name(err));
                }
            }
        }

        // Интервал отправки — 10 секунд
        vTaskDelay(pdMS_TO_TICKS(10000));
    }

    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}