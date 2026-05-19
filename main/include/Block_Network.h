#ifndef BLOCK_NETWORK_H
#define BLOCK_NETWORK_H

#include <string.h>
#include "esp_wifi.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"

// Настройки вашей точки доступа
#define WIFI_AP_SSID      "IM_HEADU_2_Setup" // Имя сети
#define WIFI_AP_PASS      "12345678"       // Пароль (минимум 8 символов!)
#define WIFI_AP_CHANNEL   1                // Wi-Fi канал
#define MAX_STA_CONN      4                // Максимум подключенных устройств




#endif