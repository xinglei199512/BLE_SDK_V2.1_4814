 
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "bx_config.h"
#include "osapp_config.h"
#include "rwip_config.h"
#include <stdbool.h>
#include <stdint.h>
#include "task_init.h"
#include "rwip.h"
#include "bx_dbg.h"
#include "log.h"
#include "arch.h"
#include "ble_task.h"
#include "osapp_task.h"
#include "gap.h"
#include "gapm_task.h"
#include "apollo_00.h"
#include "awo.h"
#include "rtc.h"
#include "log.h"
#include "rst_gen.h"
#include "plf.h"
#include "reg_sysc_awo.h"
#include "sys_sleep.h"
#include "rc_calib.h"
#include "compiler_flag.h"
#if defined( INCLUDE_uxTaskGetStackHighWaterMark )
extern void stk_chk_dbg( void );
#endif

/*
 * DEFINES
 ****************************************************************************************
 */


static uint32_t ulTimerCountsForOneTick;
static uint32_t ulTimerCountsRemainder;
static uint32_t rtcMatchCount;
static uint32_t compensate_cnt;
#define RTC_INITIAL_VAL 0xfff00000


/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
/*
#if defined( INCLUDE_uxTaskGetStackHighWaterMark )
void stk_chk_dbg( void )
{
	unsigned portBASE_TYPE uxHighWaterMark;
	uint8_t task_num,i;

	task_num = core_task_table_info.table_size;
    const task_table_t *task_table = core_task_table_info.task_table;
	for(i=0; i < task_num; i++)
	{
		if(task_table[i].is_used == true)
		{
			BX_ASSERT(task_table[i].task_handle != NULL);									// task should already been initialized

			uxHighWaterMark = uxTaskGetStackHighWaterMark(*task_table[i].task_handle);		// null means check current task stack
			LOG(LOG_LVL_INFO,"%s stk remain 0x%x*4Byte\n",task_table[i].task_name,uxHighWaterMark);
		}
	}

    
    task_num = extra_task_table_info.table_size;
    task_table = extra_task_table_info.task_table;
    for(i=0; i < task_num; i++)
    {
        if(task_table[i].is_used == true)
        {
            BX_ASSERT(task_table[i].task_handle != NULL);                                   // task should already been initialized

            uxHighWaterMark = uxTaskGetStackHighWaterMark(*task_table[i].task_handle);      // null means check current task stack
            LOG(LOG_LVL_INFO,"%s stk remain 0x%x*4Byte\n",task_table[i].task_name,uxHighWaterMark);
        }
    }

}
#endif
*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if malloc failed */
    LOG(LOG_LVL_ERROR,"Malloc Failed\n");
    __BKPT(2);
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* vApplicationStackOverflowHook will only be called if stackoverflow */
    LOG(LOG_LVL_ERROR,"StackOverFlow\n");
    __BKPT(2);
}

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

bool msg_send(QueueHandle_t q,void *data,uint32_t xTicksToWait)
{
    return xQueueSend(q,data,xTicksToWait);
}

N_XIP_SECTION bool msg_send_isr(QueueHandle_t q,void *data)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool retval = xQueueSendFromISR(q,data,&xHigherPriorityTaskWoken);
    if(retval)
    {
       portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    return retval;
}


void rtos_task_init()
{
    ble_queue_create();
    app_queue_create();
    ble_stack_task_create(NULL);
    osapp_task_create(NULL);
    vTaskStartScheduler();

}

N_XIP_SECTION static bool rtc_match_cnt_increase(uint32_t step)
{
    uint32_t old_rtc_match_cnt = rtcMatchCount;
    rtcMatchCount += ulTimerCountsForOneTick * step;
    #if (RC32K_USED == 0)
    compensate_cnt += step;
    rtcMatchCount += ulTimerCountsRemainder * (compensate_cnt / configTICK_RATE_HZ );
    compensate_cnt %= configTICK_RATE_HZ;
    #endif
    if(rtcMatchCount< old_rtc_match_cnt)
    {
        return true;
    }else
    {
        return false;
    }
}

N_XIP_SECTION int32_t time_diff(uint32_t time0,uint32_t time1)
{
    int32_t diff = time0 - time1;
    return diff;
}

void xPortSysTickHandler( void );
N_XIP_SECTION void vPortOSTick_IRQ()
{
    RTC_INT_CLR();
    rtc_match_cnt_increase(1);
    /* --start-- for breakpoint debug use */
    uint32_t current_rtc_val = RTC_CURRENTCNT_GET();
    int32_t rtc_diff = time_diff(current_rtc_val,rtcMatchCount);
    if(rtc_diff >= 0)
    {
        LOG(LOG_LVL_WARN,"missing ticks\n");
        uint32_t step_ticks = rtc_diff / ulTimerCountsForOneTick + 1;
        rtc_match_cnt_increase(step_ticks);
    }
    /* --end-- */
    RTC_MATCHCNT_SET(rtcMatchCount+2);
    xPortSysTickHandler();
}

N_XIP_SECTION void RTC_IRQHandler(void)
{
    vPortOSTick_IRQ();
}

N_XIP_SECTION void SysTick_Handler(void)
{
    xPortSysTickHandler();
}

static void wakeup_delay_and_timer_counts_config()
{
    ulTimerCountsForOneTick = portRTC_FREQUENCY / configTICK_RATE_HZ;
    ulTimerCountsRemainder = portRTC_FREQUENCY % configTICK_RATE_HZ;
    #if(RC32K_USED == 1)
    if(ulTimerCountsRemainder * 2 >= configTICK_RATE_HZ)
    {
        ulTimerCountsForOneTick += 1;
    }
    #endif
}  

#if(SYSTICK_USED == 1)
void system_tick_init()
{
    SysTick_Config(MAIN_CLOCK/configTICK_RATE_HZ);
}
#else 
void system_tick_init()
{
    wakeup_delay_and_timer_counts_config();
    rtcMatchCount = RTC_INITIAL_VAL;
    rtc_match_cnt_increase(1);
    RTC_MATCHCNT_SET(rtcMatchCount);
    RTC_LOADCNT_SET(RTC_INITIAL_VAL);
    RTC_EN_WITH_INT();
}
#endif

static void clr_rtc_intr()
{   
    RTC_INT_CLR();
    NVIC_ClearPendingIRQ(RTC_IRQn);
}

N_XIP_SECTION void bxSuppressTicksAndSleep(uint32_t xExpectedIdleTime )
{
    taskENTER_CRITICAL();
    uint8_t sleep_type = sleep_prepare_and_check();
    #if(RC32K_USED == 1)
    wakeup_delay_and_timer_counts_config();
    #endif
    eSleepModeStatus eSleepStatus = eTaskConfirmSleepModeStatus();
    if(eSleepStatus != eAbortSleep && sleep_type == (PROCESSOR_SLEEP|BLE_DEEP_SLEEP|SYS_DEEP_SLEEP))
    {
        uint32_t wakeupMatchValue; 
        uint32_t prevRTCMatchCount = rtcMatchCount - ulTimerCountsForOneTick;
        if(eSleepStatus == eStandardSleep)
        {
            uint32_t delta_compensate = ulTimerCountsRemainder * (compensate_cnt+xExpectedIdleTime)/configTICK_RATE_HZ;
            wakeupMatchValue = prevRTCMatchCount + ulTimerCountsForOneTick*xExpectedIdleTime + delta_compensate;
        }else
        {
            wakeupMatchValue = prevRTCMatchCount - 1; // long enough before RTC IRQ raise
        }
        RTC_MATCHCNT_SET(wakeupMatchValue);
        clr_rtc_intr();
        pre_deepsleep_processing_mp();
        __WFI();
        post_deepsleep_processing_mp();
        /* static */ uint32_t RTCValueAfterSleep;
        RTCValueAfterSleep = RTC_CURRENTCNT_GET();
        int32_t RTCIncrement = time_diff(RTCValueAfterSleep,prevRTCMatchCount);
        if(RTCIncrement > 0)
        {
            if(RTCIncrement*configTICK_RATE_HZ > ulTimerCountsRemainder*compensate_cnt)
            {
                uint32_t OSTickIncrement = (RTCIncrement*configTICK_RATE_HZ - ulTimerCountsRemainder*compensate_cnt) / portRTC_FREQUENCY;
                vTaskStepTick(OSTickIncrement);
                rtc_match_cnt_increase(OSTickIncrement);
            }
            uint32_t current_rtc_val = RTC_CURRENTCNT_GET();
            if(time_diff(rtcMatchCount,current_rtc_val)>2)
            {
                RTC_MATCHCNT_SET(rtcMatchCount);
            }else
            {
                RTC_MATCHCNT_SET(current_rtc_val + 2);
            }
            clr_rtc_intr();
        }
    }else if(sleep_type & PROCESSOR_SLEEP)
    {
        __WFI();
    }
    taskEXIT_CRITICAL();
}





