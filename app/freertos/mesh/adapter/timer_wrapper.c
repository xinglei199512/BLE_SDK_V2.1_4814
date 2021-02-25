#include "timer_wrapper.h"
#include "static_buffer.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "stack_mem_cfg.h"
#include "os_wrapper.h"


#define TIMER_POOL_SIZE (NETWORK_ADV_TX_BUF_SIZE+PB_ADV_TX_BUF_SIZE+BEACON_ADV_TX_BUF_SIZE+UPPER_TX_MNGT_BUF_SIZE+REASSEMBLY_ENV_BUF_SIZE*2)

DEF_ARRAY_BUF(timer_pool,StaticTimer_t,TIMER_POOL_SIZE);

mesh_timer_t mesh_timer_create(const char * const name,const uint32_t ticks,const bool auto_reload,void *const associated_data,void (*callback)(mesh_timer_t))
{
    StaticTimer_t *env = array_buf_alloc(&timer_pool);
    if(env)
    {
        return xTimerCreateStatic(name,ticks,auto_reload?pdTRUE:pdFALSE,associated_data,callback,env);
    }else
    {
        return NULL;
    }
}

bool mesh_timer_cancel(mesh_timer_t timer)
{
    bool cancelled;
    os_enter_critical();
    cancelled = mesh_timer_active(timer);
    if(cancelled)
    {
        mesh_timer_stop(timer);
    }
    os_exit_critical();
    return cancelled;

}

bool mesh_timer_active_then_reset(mesh_timer_t timer)
{
    bool active;
    os_enter_critical();
    active = mesh_timer_active(timer);
    if(active)
    {
        mesh_timer_reset(timer);
    }
    os_exit_critical();
    return active;
}

bool mesh_timer_active(mesh_timer_t timer)
{
    return xTimerIsTimerActive(timer);
}

void mesh_timer_start(mesh_timer_t timer)
{
    xTimerStart(timer,portMAX_DELAY);

}

void mesh_timer_stop(mesh_timer_t timer)
{
    BaseType_t stop =  xTimerStop(timer,0);
    BX_ASSERT(stop == pdPASS);
}

void mesh_timer_change_period(mesh_timer_t timer,uint32_t new_period)
{
    BX_ASSERT(new_period);
    xTimerChangePeriod(timer, new_period, portMAX_DELAY);
}

void mesh_timer_delete(mesh_timer_t timer)
{
    xTimerDelete(timer,portMAX_DELAY);
    array_buf_release(&timer_pool,timer);
}

void mesh_timer_reset(mesh_timer_t timer)
{
    BaseType_t reset = xTimerReset(timer, 0);
    BX_ASSERT(reset == pdPASS);
}

void *mesh_timer_get_associated_data(const mesh_timer_t timer)
{
    return pvTimerGetTimerID(timer);
}

void mesh_timer_set_associated_data(mesh_timer_t timer,void *associated_data)
{
    vTimerSetTimerID(timer,associated_data);
}

const char *mesh_timer_get_name(mesh_timer_t timer)
{
    return pcTimerGetName(timer);
}

uint32_t mesh_timer_get_period(mesh_timer_t timer)
{
    return xTimerGetPeriod(timer);
}

uint32_t mesh_timer_get_expiry_time(mesh_timer_t timer)
{
    return xTimerGetExpiryTime(timer);
}

uint32_t mesh_timer_get_remain_time(mesh_timer_t timer)
{
    int32_t expiry_time = mesh_timer_get_expiry_time(timer);
    int32_t tick_count = xTaskGetTickCount();
    uint32_t remain_time = 0;

    remain_time = (uint32_t)(expiry_time - tick_count);

    return remain_time;
}
