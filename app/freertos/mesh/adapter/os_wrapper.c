#include "os_wrapper.h"
#include "FreeRTOS.h"
#include "task.h"

void os_enter_critical()
{
    taskENTER_CRITICAL();
}

void os_exit_critical()
{
    taskEXIT_CRITICAL();
}
