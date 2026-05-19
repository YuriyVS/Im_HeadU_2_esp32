#include "Block_GPIO.h"

//Настройка прерывания для STM32 (GPIO 3)
void init_int_to_stm32_signal() {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,    // Это выход
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << GPIO_INT_TO_STM32),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&io_conf);
    
    // По умолчанию держим в логическом нуле
    gpio_set_level(GPIO_INT_TO_STM32, 0);
}

void init_led() {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pin_bit_mask = (1ULL << BLINK_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&io_conf);

    // Зажечь светодиод
    gpio_set_level(BLINK_GPIO, 1);
}

