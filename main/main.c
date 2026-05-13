#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "DB_Parameters.h"
#include "DB_Main.h"
#include "Block_FRAM.h"

void app_main(void)
{
    DBMain = DBMainInit;
    DBParameters = DBParametersFactory;
    while (true) {
        printf("Hello from app_main!\n");
        sleep(1);
    }
}
