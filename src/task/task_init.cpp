#include "task_init.h"

void task_intit()
{
    xTaskCreate(TaskTime, "TaskTime", 4096, NULL, 1, NULL);
}