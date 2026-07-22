#include "web_server.h"

// 1. Сама веб-страница (HTML + CSS + JS)
// const char *index_html = 
// "<!DOCTYPE html><html><head><meta charset='utf-8'>"
// "<meta name='viewport' content='width=device-width, initial-scale=1'>"
// "<title>Инвертор 4000В</title>"
// "<style>"
// "body { font-family: Arial, sans-serif; background: #2c3e50; color: #ecf0f1; text-align: center; padding: 20px; }"
// ".card { background: #34495e; padding: 20px; border-radius: 12px; box-shadow: 0 4px 15px rgba(0,0,0,0.3); margin: 20px auto; max-width: 300px; border-left: 5px solid #3498db; }"
// ".val { font-size: 2.5em; font-weight: bold; color: #f1c40f; }"
// ".label { font-size: 1.2em; color: #bdc3c7; }"
// "</style></head><body>"
// "<h2>Мониторинг Системы</h2>"
// "<div class='card'><div class='label'>Напряжение сети</div><span class='val' id='u'>--</span> В</div>"
// "<div class='card'><div class='label'>Ток АКБ</div><span class='val' id='i'>--</span> А</div>"
// "<script>"
// "setInterval(() => {"
// "  fetch('/api/data').then(r => r.json()).then(d => {"
// "    document.getElementById('u').innerText = d.UsetiV.toFixed(2);"
// "    document.getElementById('i').innerText = d.IakbA.toFixed(2);"
// "  }).catch(e => console.log('Ошибка связи'));"
// "}, 500);" // Обновление каждые 500 мс
// "</script></body></html>";

// 2. Обработчик главной страницы (отдает HTML)
// esp_err_t index_get_handler(httpd_req_t *req) {
//     httpd_resp_set_type(req, "text/html");
//     return httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
// }

// Обработчик запроса главной страницы
esp_err_t index_get_handler(httpd_req_t *req)
{
    // Указываем браузеру тип контента
    httpd_resp_set_type(req, "text/html");
    
    // Вычисляем точную длину файла в байтах
    size_t index_html_len = index_html_end - index_html_start;
    
    // Отправляем данные напрямую из Flash-памяти (очень быстро, 0 потребления RAM heap)
    httpd_resp_send(req, (const char*)index_html_start, index_html_len);
    
    return ESP_OK;
}

// Ваша структура регистрации обработчика
httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_get_handler,
    .user_ctx  = NULL
};

// 3. Обработчик API данных (отдает JSON с цифрами от STM32)
esp_err_t api_data_handler(httpd_req_t *req) {
    char json_response[100];
    
    // Формируем JSON-строку из нашей структуры DBMain
    snprintf(json_response, sizeof(json_response), 
             "{\"UsetiV\": %.2f, \"IakbA\": %.2f}", 
             DBMain.f50.UsetiV, DBMain.f50.IakbA);
             
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json_response, HTTPD_RESP_USE_STRLEN);
}

esp_err_t api_data_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    char json_response[512];

    snprintf(json_response, sizeof(json_response),
        "{"
        "\"UsetiV\":%.2f,"
        "\"IakbA\":%.2f,"
        "\"GenFreqHz\":%.2f,"
        "\"phaseSum\":%ld,"
        "\"Run\":%d,"
        "\"Fault\":%d,"
        "\"Alarm\":%d,"
        "\"ABC380ok\":%d"
        "}",
        DBMain.f50.UsetiV,
        DBMain.f50.IakbA,
        DBMain.f50.FreqGenABHz,
        (long)DBMain.i50.phaseSum,
        DBMain.b64.Run,
        DBMain.b64.Fault,
        DBMain.b64.Alarm,
        DBMain.b64.ABC380ok
    );

    // ИСПРАВЛЕНО: используем правильный макрос ESP-IDF
    httpd_resp_send(req, json_response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// 1. Вспомогательная функция сопоставления имени группы и БД с номером блока (0..9)
uint16_t get_block_num(bool is_params, const char *group) {
    uint16_t base = is_params ? 5 : 0;

    if (strcmp(group, "f50") == 0)  return base + 0;
    if (strcmp(group, "f100") == 0) return base + 1;
    if (strcmp(group, "i50") == 0)  return base + 2;
    if (strcmp(group, "u50") == 0)  return base + 3;
    
    // Любой из битовых блоков вызывает считывание сразу всех трех флагов (b32, b64, b96)
    if (strcmp(group, "b32") == 0 || strcmp(group, "b64") == 0 || strcmp(group, "b96") == 0) {
        return base + 4;
    }

    return base + 0; // По умолчанию
}

// -------------------------------------------------------------------
// 2. ОБРАБОТЧИК ДЛЯ DBMain (/api/data_all)
// -------------------------------------------------------------------
esp_err_t api_data_all_handler(httpd_req_t *req) {
    char url_query[64] = {0};
    char group_param[16] = {0};

    // Читаем имя группы из URL: /api/data_all?group=f50
    if (httpd_req_get_url_query_str(req, url_query, sizeof(url_query)) == ESP_OK) {
        httpd_query_key_value(url_query, "group", group_param, sizeof(group_param));
    }

    // Если группа не передана — по умолчанию читаем f50
    if (strlen(group_param) == 0) {
        strcpy(group_param, "f50");
    }

    // Вычисляем номер блока (0..4) и читаем его по Modbus в DBMain
    uint16_t block_num = get_block_num(false, group_param);
    read_DB_Main_block_number(block_num); 

    // Формируем JSON ответ из свежих данных в DBMain
    cJSON *root = cJSON_CreateObject();

    if (strcmp(group_param, "f50") == 0) {
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < 50; i++) cJSON_AddItemToArray(arr, cJSON_CreateNumber(DBMain.f50.as_array[i]));
        cJSON_AddItemToObject(root, "f50", arr);
    } 
    else if (strcmp(group_param, "f100") == 0) {
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < 50; i++) cJSON_AddItemToArray(arr, cJSON_CreateNumber(DBMain.f100.as_array[i]));
        cJSON_AddItemToObject(root, "f100", arr);
    } 
    else if (strcmp(group_param, "i50") == 0) {
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < 50; i++) cJSON_AddItemToArray(arr, cJSON_CreateNumber(DBMain.i50.as_array[i]));
        cJSON_AddItemToObject(root, "i50", arr);
    } 
    else if (strcmp(group_param, "u50") == 0) {
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < 50; i++) cJSON_AddItemToArray(arr, cJSON_CreateNumber(DBMain.u50.as_array[i]));
        cJSON_AddItemToObject(root, "u50", arr);
    } 
    else if (strcmp(group_param, "b32") == 0) {
        cJSON_AddNumberToObject(root, "b32", DBMain.b32.all);
    } 
    else if (strcmp(group_param, "b64") == 0) {
        cJSON_AddNumberToObject(root, "b64", DBMain.b64.all);
    } 
    else if (strcmp(group_param, "b96") == 0) {
        cJSON_AddNumberToObject(root, "b96", DBMain.b96.all);
    }

    const char *json_str = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json_str);

    cJSON_free((void *)json_str);
    cJSON_Delete(root);

    return ESP_OK;
}

// -------------------------------------------------------------------
// 3. ОБРАБОТЧИК ДЛЯ DBParameters (/api/params_all)
// -------------------------------------------------------------------
esp_err_t api_params_all_handler(httpd_req_t *req) {
    char url_query[64] = {0};
    char group_param[16] = {0};

    if (httpd_req_get_url_query_str(req, url_query, sizeof(url_query)) == ESP_OK) {
        httpd_query_key_value(url_query, "group", group_param, sizeof(group_param));
    }

    if (strlen(group_param) == 0) {
        strcpy(group_param, "f50");
    }

    // Вычисляем номер блока (5..9) и читаем его по Modbus в DBParameters
    uint16_t block_num = get_block_num(true, group_param);
    read_DB_Main_block_number(block_num);

    // Формируем JSON ответ из свежих данных в DBParameters
    cJSON *root = cJSON_CreateObject();

    if (strcmp(group_param, "f50") == 0) {
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < 50; i++) cJSON_AddItemToArray(arr, cJSON_CreateNumber(DBParameters.f50.as_array[i]));
        cJSON_AddItemToObject(root, "f50", arr);
    } 
    else if (strcmp(group_param, "f100") == 0) {
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < 50; i++) cJSON_AddItemToArray(arr, cJSON_CreateNumber(DBParameters.f100.as_array[i]));
        cJSON_AddItemToObject(root, "f100", arr);
    } 
    else if (strcmp(group_param, "i50") == 0) {
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < 50; i++) cJSON_AddItemToArray(arr, cJSON_CreateNumber(DBParameters.i50.as_array[i]));
        cJSON_AddItemToObject(root, "i50", arr);
    } 
    else if (strcmp(group_param, "u50") == 0) {
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < 50; i++) cJSON_AddItemToArray(arr, cJSON_CreateNumber(DBParameters.u50.as_array[i]));
        cJSON_AddItemToObject(root, "u50", arr);
    } 
    else if (strcmp(group_param, "b32") == 0) {
        cJSON_AddNumberToObject(root, "b32", DBParameters.b32.all);
    } 
    else if (strcmp(group_param, "b64") == 0) {
        cJSON_AddNumberToObject(root, "b64", DBParameters.b64.all);
    } 
    else if (strcmp(group_param, "b96") == 0) {
        cJSON_AddNumberToObject(root, "b96", DBParameters.b96.all);
    }

    const char *json_str = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json_str);

    cJSON_free((void *)json_str);
    cJSON_Delete(root);

    return ESP_OK;
}

// esp_err_t api_data_all_handler(httpd_req_t *req)
// {
//     httpd_resp_set_type(req, "application/json");
//     httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

//     char chunk[128];

//     // Начало JSON
//     httpd_resp_send_chunk(req, "{\"f50\":[", HTTPD_RESP_USE_STRLEN);

//     // 1. Float 1-50
//     for (int i = 0; i < 50; i++) {
//         snprintf(chunk, sizeof(chunk), "%.2f%s", DBMain.f50.as_array[i], (i < 49) ? "," : "");
//         httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
//     }

//     // 2. Float 51-100
//     httpd_resp_send_chunk(req, "],\"f100\":[", HTTPD_RESP_USE_STRLEN);
//     for (int i = 0; i < 50; i++) {
//         snprintf(chunk, sizeof(chunk), "%.2f%s", DBMain.f100.as_array[i], (i < 49) ? "," : "");
//         httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
//     }

//     // 3. Int32 1-50
//     httpd_resp_send_chunk(req, "],\"i50\":[", HTTPD_RESP_USE_STRLEN);
//     for (int i = 0; i < 50; i++) {
//         snprintf(chunk, sizeof(chunk), "%ld%s", (long)DBMain.i50.as_array[i], (i < 49) ? "," : "");
//         httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
//     }

//     // 4. UInt32 1-50
//     httpd_resp_send_chunk(req, "],\"u50\":[", HTTPD_RESP_USE_STRLEN);
//     for (int i = 0; i < 50; i++) {
//         snprintf(chunk, sizeof(chunk), "%lu%s", (unsigned long)DBMain.u50.as_array[i], (i < 49) ? "," : "");
//         httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
//     }

//     // 5-7. Bools (передаем как 32-битные битовые маски)
//     snprintf(chunk, sizeof(chunk), "],\"b32\":%lu,\"b64\":%lu,\"b96\":%lu}",
//              (unsigned long)DBMain.b32.all,
//              (unsigned long)DBMain.b64.all,
//              (unsigned long)DBMain.b96.all);
//     httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);

//     // Завершение chunked-передачи
//     httpd_resp_send_chunk(req, NULL, 0);
//     return ESP_OK;
// }

// esp_err_t api_params_all_handler(httpd_req_t *req)
// {
//     httpd_resp_set_type(req, "application/json");
//     httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

//     char chunk[128];

//     // f50
//     httpd_resp_send_chunk(req, "{\"f50\":[", HTTPD_RESP_USE_STRLEN);
//     for (int i = 0; i < 50; i++) {
//         snprintf(chunk, sizeof(chunk), "%.2f%s", DBParameters.f50.as_array[i], (i < 49) ? "," : "");
//         httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
//     }

//     // f100
//     httpd_resp_send_chunk(req, "],\"f100\":[", HTTPD_RESP_USE_STRLEN);
//     for (int i = 0; i < 50; i++) {
//         snprintf(chunk, sizeof(chunk), "%.2f%s", DBParameters.f100.as_array[i], (i < 49) ? "," : "");
//         httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
//     }

//     // i50
//     httpd_resp_send_chunk(req, "],\"i50\":[", HTTPD_RESP_USE_STRLEN);
//     for (int i = 0; i < 50; i++) {
//         snprintf(chunk, sizeof(chunk), "%ld%s", (long)DBParameters.i50.as_array[i], (i < 49) ? "," : "");
//         httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
//     }

//     // u50
//     httpd_resp_send_chunk(req, "],\"u50\":[", HTTPD_RESP_USE_STRLEN);
//     for (int i = 0; i < 50; i++) {
//         snprintf(chunk, sizeof(chunk), "%lu%s", (unsigned long)DBParameters.u50.as_array[i], (i < 49) ? "," : "");
//         httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
//     }

//     // b32, b64, b96, crc32
//     snprintf(chunk, sizeof(chunk), "],\"b32\":%lu,\"b64\":%lu,\"b96\":%lu,\"crc32\":%lu}",
//              (unsigned long)DBParameters.b32.all,
//              (unsigned long)DBParameters.b64.all,
//              (unsigned long)DBParameters.b96.all,
//              (unsigned long)DBParameters.CRC32);
//     httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);

//     httpd_resp_send_chunk(req, NULL, 0);
//     return ESP_OK;
// }


esp_err_t api_param_write_handler(httpd_req_t *req)
{
    char buf[128];
    int ret = httpd_req_recv(req, buf, MIN(req->content_len, sizeof(buf) - 1));
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive payload");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    cJSON *json = cJSON_Parse(buf);
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    cJSON *item_group = cJSON_GetObjectItem(json, "group");
    cJSON *item_index = cJSON_GetObjectItem(json, "index");
    cJSON *item_val   = cJSON_GetObjectItem(json, "val");

    if (!item_group || !item_index || !item_val) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing fields");
        return ESP_FAIL;
    }

    const char *group = item_group->valuestring;
    int index         = item_index->valueint;
    double val        = item_val->valuedouble;

    uint16_t reg_start = 0;
    uint32_t raw32     = 0;

    // Вычисляем адрес Holding Register и формируем 32-битный паттерн
    if (strcmp(group, "f50") == 0) {
        reg_start = 0 + index * 2;
        float fval = (float)val;
        memcpy(&raw32, &fval, 4);
    } else if (strcmp(group, "f100") == 0) {
        reg_start = 100 + index * 2;
        float fval = (float)val;
        memcpy(&raw32, &fval, 4);
    } else if (strcmp(group, "i50") == 0) {
        reg_start = 200 + index * 2;
        int32_t ival = (int32_t)val;
        memcpy(&raw32, &ival, 4);
    } else if (strcmp(group, "u50") == 0) {
        reg_start = 300 + index * 2;
        uint32_t uval = (uint32_t)val;
        raw32 = uval;
    } else if (strcmp(group, "b32") == 0 || strcmp(group, "b64") == 0 || strcmp(group, "b96") == 0) {
        int bit_offset = (strcmp(group, "b32") == 0) ? 0 : (strcmp(group, "b64") == 0) ? 1 : 2;
        reg_start = 400 + bit_offset * 2;
        raw32 = (uint32_t)val; // Передается готовая 32-битная битовая маска
    }

    cJSON_Delete(json);

    // Выполняем запись по Modbus
    if (modbus_write_param_32(reg_start, raw32) == ESP_OK) {
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
        return ESP_OK;
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Modbus Write Timeout");
        return ESP_FAIL;
    }
}

esp_err_t api_logs_get_handler(httpd_req_t *req) {
    uint16_t offset = 0;
    uint8_t limit = 12; // По умолчанию отдаем по 12 записей

    // 1. Разбираем параметры query URL: /api/logs?offset=0&limit=12
    char query[64];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char val_str[10];
        if (httpd_query_key_value(query, "offset", val_str, sizeof(val_str)) == ESP_OK) {
            offset = atoi(val_str);
        }
        if (httpd_query_key_value(query, "limit", val_str, sizeof(val_str)) == ESP_OK) {
            limit = atoi(val_str);
            if (limit > 15) limit = 15; // Ограничение кадра Modbus
        }
    }

    // 2. Читаем статистику журнала (Адрес 2000, 3 регистра)
    uint16_t stats_regs[3];
    mb_param_request_t req_stats = {
        .slave_addr = SLAVE_ADDR, .command = 0x03, .reg_start = 2000, .reg_size = 3
    };
    
    uint32_t total_recorded = 0;
    uint16_t total_in_ring = 0;
    if (mbc_master_send_request(&req_stats, stats_regs) == ESP_OK) {
        total_recorded = ((uint32_t)stats_regs[0] << 16) | stats_regs[1];
        total_in_ring  = stats_regs[2];
    }

    // 3. Вычитываем 12 записей за ОДИН пакет Modbus
    LogEntry_t logs[15];
    uint8_t fetched_count = 0;
    modbus_read_log_page(offset, limit, logs, &fetched_count);

    // 4. Формируем JSON ответ для Web UI
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "total", total_in_ring);
    cJSON_AddNumberToObject(root, "total_all_time", total_recorded);
    cJSON_AddNumberToObject(root, "offset", offset);
    cJSON_AddNumberToObject(root, "limit", limit);

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < fetched_count; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "time",   logs[i].timestamp);
        cJSON_AddNumberToObject(item, "id",     logs[i].event_id);
        cJSON_AddNumberToObject(item, "sev",    logs[i].severity);
        cJSON_AddNumberToObject(item, "val",    logs[i].value);
        cJSON_AddNumberToObject(item, "param",  logs[i].param_id);
        cJSON_AddItemToArray(arr, item);
    }
    cJSON_AddItemToObject(root, "logs", arr);

    const char *json_string = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json_string);

    cJSON_free((void *)json_string);
    cJSON_Delete(root);

    return ESP_OK;
}

esp_err_t api_logs_export_handler(httpd_req_t *req) {
    // 1. Запрашиваем из STM32 общее количество записей в кольце (Адрес 2000)
    uint16_t stats_regs[3];
    mb_param_request_t req_stats = {
        .slave_addr = SLAVE_ADDR, .command = 0x03, .reg_start = 2000, .reg_size = 3
    };
    
    uint16_t total_in_ring = 0;
    if (mbc_master_send_request(&req_stats, stats_regs) == ESP_OK) {
        total_in_ring = stats_regs[2];
    }

    // 2. Устанавливаем HTTP-заголовки скачивания файла
    httpd_resp_set_type(req, "text/csv; charset=utf-8");
    httpd_resp_set_hdr(req, "Content-Disposition", "attachment; filename=\"event_log.csv\"");

    // 3. Отправляем заголовок таблицы CSV (разделитель точка с запятой ';' для Excel)
    const char *csv_header = "N;Timestamp_ms;Event_ID;Severity;Value;Param_ID\r\n";
    httpd_resp_send_chunk(req, csv_header, strlen(csv_header));

    // 4. Потоково вычитываем весь журнал из STM32 пачками по 12 записей и отправляем в сеть
    LogEntry_t logs[15];
    uint8_t fetched = 0;
    char row_buf[128];

    for (uint16_t offset = 0; offset < total_in_ring; offset += 12) {
        uint8_t count_to_read = (total_in_ring - offset > 12) ? 12 : (total_in_ring - offset);
        
        if (modbus_read_log_page(offset, count_to_read, logs, &fetched) == ESP_OK) {
            for (uint8_t i = 0; i < fetched; i++) {
                const char* sev_str = (logs[i].severity == 1) ? "WARN" : 
                                     (logs[i].severity == 2) ? "ALARM" : "INFO";
                
                int len = snprintf(row_buf, sizeof(row_buf), 
                    "%u;%lu;0x%04X;%s;%.2f;%lu\r\n",
                    offset + i + 1,
                    (unsigned long)logs[i].timestamp,
                    logs[i].event_id,
                    sev_str,
                    logs[i].value,
                    (unsigned long)logs[i].param_id
                );
                // Отправляем строку чанком
                httpd_resp_send_chunk(req, row_buf, len);
            }
        } else {
            break; // Обрыв связи по Modbus
        }
    }

    // 5. Завершаем поток передачи (пустой чанк)
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

// 4. Функция запуска самого веб-сервера
httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 8; // С запасом

    ESP_LOGI("WEB", "Запуск HTTP сервера на порту %d", config.server_port);

    if (httpd_start(&server, &config) == ESP_OK) {
        // Регистрируем путь "/"
        httpd_uri_t uri_index = {
            .uri      = "/",
            .method   = HTTP_GET,
            .handler  = index_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_index);

        // Регистрируем путь "/api/data_all"
        httpd_uri_t uri_api_all = {
        .uri      = "/api/data_all",
        .method   = HTTP_GET,
        .handler  = api_data_all_handler,
        .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_api_all);

        httpd_uri_t uri_params_all = {
        .uri      = "/api/params_all",
        .method   = HTTP_GET,
        .handler  = api_params_all_handler,
        .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_params_all);

        httpd_uri_t uri_param_write = {
        .uri      = "/api/param_write",
        .method   = HTTP_POST,
        .handler  = api_param_write_handler,
        .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_param_write);

        // 1. Определение структуры маршрута для журнала событий
        httpd_uri_t uri_logs_get = {
        .uri      = "/api/logs",
        .method   = HTTP_GET,
        .handler  = api_logs_get_handler,
        .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_logs_get);

        httpd_uri_t uri_logs_export = {
        .uri      = "/api/logs_export",
        .method   = HTTP_GET,
        .handler  = api_logs_export_handler,
        .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_logs_export);
        
        return server;
    }
    ESP_LOGE("WEB", "Ошибка запуска сервера!");
    return NULL;
}

