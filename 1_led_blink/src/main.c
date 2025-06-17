#include "freertos/FreeRTOS.h"
#include "freertos/task.h"           
#include "driver/gpio.h"             

#define LED_GPIO GPIO_NUM_2 // Порт светодиода

TaskHandle_t Blink_Handle = NULL; // Дескриптор задачи Blink

void Blink_Task(void *arg){
    esp_rom_gpio_pad_select_gpio(LED_GPIO); // "переключение" выбранного физического контакта в режим GPIO
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT); // Устанавливаем направление как выход
    
    while(1){
        gpio_set_level(LED_GPIO, 1);        // Устанавливаем логический уровень 1
        vTaskDelay(pdMS_TO_TICKS(1000));    // Ждем
        gpio_set_level(LED_GPIO, 0);        // Устанавливаем логический уровень 0
        vTaskDelay(pdMS_TO_TICKS(1000));    // Ждем
    }
}


void app_main() {
    xTaskCreate(
        Blink_Task,     // указатель на функцию‑задачу
        "BLINK",        // имя задачи (для отладки)
        4096,           // размер стека
        NULL,           // аргумент, передаваемый в функцию (здесь не нужен)
        10,             // приоритет задачи
        &Blink_Handle   // указатель, в который запишут дескриптор задачи
    );
}