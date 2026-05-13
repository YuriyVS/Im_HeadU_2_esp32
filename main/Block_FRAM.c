#include "Block_FRAM.h"

uint32_t current_point = 0;
bool archive_requested = false;
LogFrame_t data_to_log; // 10 сигналов (float/uint)
DBSled_t DBSled= {0};

