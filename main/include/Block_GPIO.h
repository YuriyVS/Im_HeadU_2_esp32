#ifndef BLOCK_GPIO_H
#define BLOCK_GPIO_H

#include "driver/gpio.h"

#define GPIO_INT_TO_STM32 3
#define BLINK_GPIO 8

extern void init_int_to_stm32_signal();
extern void init_led();

#endif