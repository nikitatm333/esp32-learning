/*
    1) FreeRTOS.h, task.h, queue.h
        — подключаем ядро RTOS, API для задач и API для очередей.
    2) driver/gpio.h
        — подключаем API для работы с GPIO-пинами ESP32.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define LED_GPIO     GPIO_NUM_2
#define BUTTON_GPIO  GPIO_NUM_0

static QueueHandle_t gpio_evt_queue = NULL; // Обьявляем глобально очередь 

// Прерывание 
/*
    IRAM_ATTR - атрибут, говорящий компилятору переместить эту функцию в IRAM;
    IRAM - быстрая внутренняя память.
    Это нужно для того чтобы ISR всегда был доступен без "подгрузок" из внешней FLASH.
*/
static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL); // Отправляем в очередь номер GPIO
}

// Задача - обработчик
static void gpio_task_example(void* arg) {
    uint32_t io_num;
    while (1) {
        // xQueueReceive - ждет (блокируется) до тех пор, пока в очередь не придет новое событие
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            // Читаем текущий уровень кнопки, после получения события
            int level = gpio_get_level(BUTTON_GPIO);
            // Инвертируем, чтобы нажатие (0) → LED=1, отпускание (1) → LED=0
            gpio_set_level(LED_GPIO, !level);
        }
    }
}

void app_main(void) {
    // LED как выход
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    // Кнопка как вход с pull-up
    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_en(BUTTON_GPIO);

    // Прерывание на любой фронт
    gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_ANYEDGE);

    // Создать очередь
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // Запустить GPIO-ISR сервис
    gpio_install_isr_service(0);
    // Повесить ISR-обработчик на кнопку
    gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, (void*) BUTTON_GPIO);
    // Запустить задачу - слушатель очереди
    xTaskCreate(gpio_task_example, "gpio_task", 2048, NULL, 10, NULL);
}
