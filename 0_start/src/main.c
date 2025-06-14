#include "freertos/FreeRTOS.h"

TaskHandle_t Task_1_Handle = NULL; // Дескриптор (хэндл) задачи 1
TaskHandle_t Task_2_Handle = NULL; // Дескриптор (хэндл) задачи 2

// Первая задача, которая будет прикреплена к ядру 0
void task_core0(void *arg) {
    while (1) {
        printf("Запущена первая задача на ядре %d\n", xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Вторая задача, которая будет прикреплена к ядру 1
void task_core1(void *arg) {
    while (1) {
        printf("Запущена вторая задача на ядре %d\n", xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main() {
    // Создаём первую задачу и прикрепляем её к ядру 0 (PRO_CPU)
    xTaskCreatePinnedToCore(
        task_core0,            // функция‑задача
        "TaskCore0",           // имя задачи
        2048,                  // размер стека
        NULL,                  // аргумент
        5,                     // приоритет
        &Task_1_Handle,        // хэндл
        0                      // ядро 0
    );

    // Создаём вторую задачу и пинним её к ядру 1 (APP_CPU)
    xTaskCreatePinnedToCore(
        task_core1,            // функция‑задача
        "TaskCore1",           // имя задачи
        2048,                  // размер стека
        NULL,                  // аргумент
        5,                     // приоритет
        &Task_1_Handle,        // хэндл
        1                      // ядро 1
    );
  }