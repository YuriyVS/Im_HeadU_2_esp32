#ifndef WEB_SERVER_H
#define BLOCK_WEB_SERVER_HGPIO_H

#include <esp_http_server.h>
#include "esp_log.h"
#include "DB_Parameters.h"
#include "DB_Main.h"
#include "Block_FRAM.h"
// Убедитесь, что здесь подключен файл с вашей структурой DBMain, 
// чтобы сервер видел переменные UsetiV и IakbA!

extern esp_err_t index_get_handler(httpd_req_t *req);
extern esp_err_t api_data_handler(httpd_req_t *req);
extern httpd_handle_t start_webserver(void);
extern const char *index_html;


#endif