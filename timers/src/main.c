/*
    Аппаратные таймеры (hardware timers) – счётчики, встроенные в микроконтроллер.
    Снижают нагрузку на CPU, поскольку считаются «на железе», и могут генерировать
    прерывания при переполнении или достижении заданного значения.

    Программные таймеры
        - FreeRTOS timers (xTimerCreate) – реализованы в ОС на основе аппаратного системного тика. Удобны для планирования задач в контексте RTOS.
        - esp_timer (ESP-IDF) – высокоточный 64-битный таймер на базе аппаратного — для вызова колбэков в IRAM.

    ----------    

    ESP32 имеет два семейства аппаратных таймеров:
    1) General Purpose Timer Group (TIMG)
        - 2 группы (TIMERG0 и TIMERG1), в каждой по 2 таймера (TIMER0 и TIMER1).
        - 64-битный счётчик (в виде пары 32-бит), может работать в режиме авто-перезагрузки.
        - Прерывания на достижение compare-значения.

    2) RTC Timer
        - Работает в режиме низкого энергопотребления, работает в Deep-Sleep.
        - Каждый таймер имеет в TRM (Technical Reference Manual) набор регистров:
    
    ----------    

    FreeRTOS Software-таймеры
    - Используют системный tick (обычно 100 Hz или 1 kHz), который увеличивается аппаратным Systick’ом.
    - Внутри есть список активных таймеров, и каждый tick движется по нему, уменьшая их «оставшееся время».
    - Когда время истекает, в контексте системного тика планировщик создаёт задачу-сервис таймеров, и та вызывает колбэк пользователя.

    Основные API:

    TimerHandle_t xTimerCreate(
        const char * name,
        TickType_t period_ticks,
        UBaseType_t auto_reload,   // pdTRUE для периодического таймера
        void * id,                 // любой «контекст» для колбэка
        TimerCallbackFunction_t callback);

    xTimerStart(timer, portMAX_DELAY);
    xTimerStop(timer,  portMAX_DELAY);

    ----------    

    ESP-IDF esp_timer

    - Работает независимо от FreeRTOS-tick (более точен).
    - 64-битный счётчик, может задавать интервалы вплоть до микросекунд.
    - Колбэк вызывается в своей задаче-сервисе, не в ISR, поэтому можно использовать API.

    API:

    esp_timer_create_args_t args = {
        .callback = &my_callback,
        .arg      = some_pointer,
        .dispatch_method = ESP_TIMER_TASK,  // колбэк в задаче
        .name     = "my_timer"
    };

    esp_timer_handle_t timer;
    esp_timer_create(&args, &timer);
    esp_timer_start_periodic(timer, 1000000); // микросекунд: 1 000 000 = 1 с
    // или esp_timer_start_once(timer, 500000); // однократный вызов через 0.5 с

*/

#include "esp_timer.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_GPIO GPIO_NUM_4
static const char *TAG = "BLINK";

static bool led_on = false;

// callback-функция, которую таймер будет вызывать 
static void blink_cb(void* arg) {
    led_on = !led_on;
    ESP_LOGI(TAG, "Timer callback fired. LED = %d", led_on);
    gpio_set_level(LED_GPIO, led_on);
}

void app_main(void) {
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0); // стартуем с выкл

    // Настройка таймера
    /*
        Это структура esp_timer_create_args_t — аргументы для создания таймера:
        .callback - Указатель на функцию, которую таймер должен вызывать
        .arg - Аргумент, который передаётся в callback (в данной задаче не нужен)
        .name - Имя таймера (для отладки)
    */
    const esp_timer_create_args_t timer_args = {
        .callback = blink_cb,
        .arg = NULL,
        .name = "blink_timer"
    };

    // esp_timer_handle_t — это дескриптор таймера, через который мы управляем им
    esp_timer_handle_t blink_timer;
    // esp_timer_create(...) — создаёт таймер, используя timer_args.
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &blink_timer));
    // esp_timer_start_periodic(...) — запускает таймер, вызывая blink_cb каждые 500 000 микросекунд (то есть 500 мс).
    ESP_ERROR_CHECK(esp_timer_start_periodic(blink_timer, 500000));
    // Логирование
    ESP_LOGI(TAG, "Blink timer started");
}
