#include "freertos/FreeRTOS.h"
#include "freertos/task.h"           // vTaskDelay
#include "driver/gpio.h"             // GPIO API



#define LED_GPIO GPIO_NUM_2

void app_main() {
    // 1. Cбрасываем пин к дефолту
    gpio_reset_pin(LED_GPIO);
    
    // 2. Конфигурируем направление — выход
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    
    // 3. Бесконечный цикл мигания
    while (1) {
        // Включаем светодиод (уровень HIGH)
        gpio_set_level(LED_GPIO, 1);
        // Задержка 500 мс
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // Выключаем светодиод (уровень LOW)
        gpio_set_level(LED_GPIO, 0);
        // Задержка 500 мс
        vTaskDelay(pdMS_TO_TICKS(500));

        /*
            Мы конвертируем миллисекунды в тики при помощи макроса pdMS_TO_TICKS(500),
            чтобы задержка была именно 500 мс.
        */
    }
}