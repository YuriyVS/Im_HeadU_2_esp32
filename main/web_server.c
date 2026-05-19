#include "web_server.h"

// 1. Сама веб-страница (HTML + CSS + JS)
const char *index_html = 
"<!DOCTYPE html><html><head><meta charset='utf-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<title>Инвертор 4000В</title>"
"<style>"
"body { font-family: Arial, sans-serif; background: #2c3e50; color: #ecf0f1; text-align: center; padding: 20px; }"
".card { background: #34495e; padding: 20px; border-radius: 12px; box-shadow: 0 4px 15px rgba(0,0,0,0.3); margin: 20px auto; max-width: 300px; border-left: 5px solid #3498db; }"
".val { font-size: 2.5em; font-weight: bold; color: #f1c40f; }"
".label { font-size: 1.2em; color: #bdc3c7; }"
"</style></head><body>"
"<h2>Мониторинг Системы</h2>"
"<div class='card'><div class='label'>Напряжение сети</div><span class='val' id='u'>--</span> В</div>"
"<div class='card'><div class='label'>Ток АКБ</div><span class='val' id='i'>--</span> А</div>"
"<script>"
"setInterval(() => {"
"  fetch('/api/data').then(r => r.json()).then(d => {"
"    document.getElementById('u').innerText = d.UsetiV.toFixed(2);"
"    document.getElementById('i').innerText = d.IakbA.toFixed(2);"
"  }).catch(e => console.log('Ошибка связи'));"
"}, 500);" // Обновление каждые 500 мс
"</script></body></html>";

// 2. Обработчик главной страницы (отдает HTML)
esp_err_t index_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
}

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

        // Регистрируем путь "/api/data"
        httpd_uri_t uri_api = {
            .uri      = "/api/data",
            .method   = HTTP_GET,
            .handler  = api_data_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_api);
        
        return server;
    }
    ESP_LOGE("WEB", "Ошибка запуска сервера!");
    return NULL;
}