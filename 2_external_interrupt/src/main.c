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
    IRAM_ATTR - атрибут (декоратор), говорящий компилятору переместить эту функцию в IRAM (в Instruction RAM, а не во флеш-память);
    IRAM - быстрая внутренняя память.
    Это нужно для того чтобы ISR всегда был доступен без "подгрузок" из внешней FLASH.
*/
// gpio_isr_handler вызывается аппаратно-контроллером прерываний, когда случаются события на BUTTON_GPIO
static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL); // Отправляем в очередь номер GPIO
}

// Задача - обработчик
static void gpio_task_example(void* arg) {
    uint32_t io_num;
    while (1) {
        // xQueueReceive - ждет (блокируется) до тех пор, пока в очередь не придет новое событие от ISR
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {  //portMAX_DELAY — ждать сколько угодно, т. е. блокировать задачу до события
            // Читаем текущий уровень кнопки, после получения события
            int level = gpio_get_level(BUTTON_GPIO);
            // Инвертируем, чтобы нажатие (0) → LED=1, отпускание (1) → LED=0
            gpio_set_level(LED_GPIO, !level);
        }
    }
}

void app_main(void) {
    // Приводим в дефолтное состояние
    gpio_reset_pin(LED_GPIO);
    // LED как выход
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

/*
    Как всё работает вместе:

    1) Инициализация: app_main() настраивает пины, очередь, ISR-сервис и создаёт задачу.
    2) Ожидание прерывания: ESP32 аппаратно следит за состоянием BUTTON_GPIO.
    3) ISR-обработчик: при изменении сигнала (любой край) срабатывает gpio_isr_handler в IRAM, который кладёт номер пина в очередь.
    4) Просыпается задача: gpio_task_example разблокируется в xQueueReceive, получает номер GPIO, читает текущее состояние пина и переключает светоди 
*/

/*
    Что такое IRAM и зачем нужен атрибут IRAM_ATTR:

    - IRAM (Instruction RAM) — это область внутренней оперативной памяти ESP32, в которую загружается код для очень быстрого и надёжного исполнения. В отличие от внешней Flash-памяти, доступ к IRAM не зависает во время операций записи/стирания флеша.
    - Когда происходит аппаратное прерывание, контроллер немедленно передаёт управление на адрес ISR (Interrupt Service Routine). Если ISR находится во Flash, а в этот момент идёт её стирание/перезапись, попытавшись выполнить код из Flash вы получите «cache error» и сбой.
    - Атрибут IRAM_ATTR говорит компилятору и линкеру поместить именно эту функцию в IRAM. Таким образом ISR всегда доступен и не зависает из-за состояния флеш-памяти 
*/