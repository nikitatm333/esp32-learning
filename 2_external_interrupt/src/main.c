#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_GPIO    GPIO_NUM_2     // Пин, к которому подключён светодиод
#define BUTTON_GPIO GPIO_NUM_23    // Пин, к которому подключена кнопка

uint8_t state = 0;                 // Переменная, хранящая текущее состояние LED (0 – выключен, 1 – включён)

TaskHandle_t Blink_Handle = NULL;  // Дескриптор задачи Blink

// Задача, которая постоянно устанавливает уровень на LED_GPIO согласно state
void Blink_Task(void *arg) {
    while (1) {
        gpio_set_level(LED_GPIO, state);
    }
}

/*
    Обработчик прерывания от кнопки.
    Просто инвертирует переменную state.
    IRAM_ATTR — атрибут, гарантирующий, что этот код попадёт в быстрый IRAM,
    а не во флеш (важно для надёжности ISR).
*/
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    state = !state;
}

void app_main() {
    esp_rom_gpio_pad_select_gpio(LED_GPIO);          // "Переключение" выбранного физического контакта в режим GPIO
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);  // Настройка LED_GPIO как цифровой вывод

    gpio_set_level(LED_GPIO, 0);   // сразу гасим

    esp_rom_gpio_pad_select_gpio(BUTTON_GPIO);          // "Переключение" выбранного физического контакта в режим GPIO
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);   // Настройка LED_GPIO как цифровой вывод

    gpio_pullup_en(BUTTON_GPIO);    // Подтяжка к VCC

    gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_POSEDGE); // Прерывание при нажатии (положительный фронт)

    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);       // Устанавливаем сервис ISR 
 
    gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, NULL);  // Регистрируем функцию-обработчик прерывания

    gpio_intr_enable(BUTTON_GPIO);  // Включаем прерывания для кнопки

    // Создаём задачу, которая будет “мигать” LED в соответствии с state
    xTaskCreate(
        Blink_Task,       // функция‑задача
        "BLINK",          // имя (для отладки)
        2048,             // размер стека (в байтах)
        NULL,             // параметр задачи
        5,                // приоритет
        &Blink_Handle     // сюда запишется хэндл задачи
    );
}
