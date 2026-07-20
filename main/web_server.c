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

esp_err_t api_data_all_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    char chunk[128];

    // Начало JSON
    httpd_resp_send_chunk(req, "{\"f50\":[", HTTPD_RESP_USE_STRLEN);

    // 1. Float 1-50
    for (int i = 0; i < 50; i++) {
        snprintf(chunk, sizeof(chunk), "%.2f%s", DBMain.f50.as_array[i], (i < 49) ? "," : "");
        httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
    }

    // 2. Float 51-100
    httpd_resp_send_chunk(req, "],\"f100\":[", HTTPD_RESP_USE_STRLEN);
    for (int i = 0; i < 50; i++) {
        snprintf(chunk, sizeof(chunk), "%.2f%s", DBMain.f100.as_array[i], (i < 49) ? "," : "");
        httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
    }

    // 3. Int32 1-50
    httpd_resp_send_chunk(req, "],\"i50\":[", HTTPD_RESP_USE_STRLEN);
    for (int i = 0; i < 50; i++) {
        snprintf(chunk, sizeof(chunk), "%ld%s", (long)DBMain.i50.as_array[i], (i < 49) ? "," : "");
        httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
    }

    // 4. UInt32 1-50
    httpd_resp_send_chunk(req, "],\"u50\":[", HTTPD_RESP_USE_STRLEN);
    for (int i = 0; i < 50; i++) {
        snprintf(chunk, sizeof(chunk), "%lu%s", (unsigned long)DBMain.u50.as_array[i], (i < 49) ? "," : "");
        httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
    }

    // 5-7. Bools (передаем как 32-битные битовые маски)
    snprintf(chunk, sizeof(chunk), "],\"b32\":%lu,\"b64\":%lu,\"b96\":%lu}",
             (unsigned long)DBMain.b32.all,
             (unsigned long)DBMain.b64.all,
             (unsigned long)DBMain.b96.all);
    httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);

    // Завершение chunked-передачи
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t api_params_all_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    char chunk[128];

    // f50
    httpd_resp_send_chunk(req, "{\"f50\":[", HTTPD_RESP_USE_STRLEN);
    for (int i = 0; i < 50; i++) {
        snprintf(chunk, sizeof(chunk), "%.2f%s", DBParameters.f50.as_array[i], (i < 49) ? "," : "");
        httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
    }

    // f100
    httpd_resp_send_chunk(req, "],\"f100\":[", HTTPD_RESP_USE_STRLEN);
    for (int i = 0; i < 50; i++) {
        snprintf(chunk, sizeof(chunk), "%.2f%s", DBParameters.f100.as_array[i], (i < 49) ? "," : "");
        httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
    }

    // i50
    httpd_resp_send_chunk(req, "],\"i50\":[", HTTPD_RESP_USE_STRLEN);
    for (int i = 0; i < 50; i++) {
        snprintf(chunk, sizeof(chunk), "%ld%s", (long)DBParameters.i50.as_array[i], (i < 49) ? "," : "");
        httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
    }

    // u50
    httpd_resp_send_chunk(req, "],\"u50\":[", HTTPD_RESP_USE_STRLEN);
    for (int i = 0; i < 50; i++) {
        snprintf(chunk, sizeof(chunk), "%lu%s", (unsigned long)DBParameters.u50.as_array[i], (i < 49) ? "," : "");
        httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);
    }

    // b32, b64, b96, crc32
    snprintf(chunk, sizeof(chunk), "],\"b32\":%lu,\"b64\":%lu,\"b96\":%lu,\"crc32\":%lu}",
             (unsigned long)DBParameters.b32.all,
             (unsigned long)DBParameters.b64.all,
             (unsigned long)DBParameters.b96.all,
             (unsigned long)DBParameters.CRC32);
    httpd_resp_send_chunk(req, chunk, HTTPD_RESP_USE_STRLEN);

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
        
        return server;
    }
    ESP_LOGE("WEB", "Ошибка запуска сервера!");
    return NULL;
}

