#ifndef WEB_SERVER_H
#define BLOCK_WEB_SERVER_HGPIO_H

#include <esp_http_server.h>
#include "esp_log.h"
#include "cJSON.h"
#include "DB_Parameters.h"
#include "DB_Main.h"
#include "Block_FRAM.h"
#include "Block_Modbus.h"
// Убедитесь, что здесь подключен файл с вашей структурой DBMain, 
// чтобы сервер видел переменные UsetiV и IakbA!

extern esp_err_t index_get_handler(httpd_req_t *req); // Обработчик главной страницы
extern esp_err_t api_data_handler(httpd_req_t *req);
extern esp_err_t api_data_get_handler(httpd_req_t *req);
extern uint16_t get_block_num(bool is_params, const char *group);
extern esp_err_t api_data_all_handler(httpd_req_t *req);
extern esp_err_t api_params_all_handler(httpd_req_t *req);
extern esp_err_t api_param_write_handler(httpd_req_t *req);
extern esp_err_t api_logs_get_handler(httpd_req_t *req);
extern esp_err_t api_logs_export_handler(httpd_req_t *req);
extern httpd_handle_t start_webserver(void);
//extern const char *index_html;

// Регистрируем внешние символы, которые CMake автоматически создаст для index.html
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");




#endif