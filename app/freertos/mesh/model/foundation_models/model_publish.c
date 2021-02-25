/**
 ****************************************************************************************
 *
 * @file   model_publish.c
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2018-11-02 09:32
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"
#include "mesh_model.h"
#include "model_publish.h"
#include "mesh_queued_msg.h"
#include "timer_wrapper.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#define SEC_TO_100MS(s) ((s) * 10U)
//#define UINT32_MAX 0xffffffff
#define TIMER_OLDER_THAN(time, ref) (((uint32_t) (time)) - ((uint32_t) (ref)) > (UINT32_MAX/2))

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */




/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
mesh_timer_t modelPublishTimer;
publish_timestamp_t timeFromPowerOn = 0;   /*  unit:ms */
static model_publish_list_t * mp_publication_list;
static bool m_publish_timer_running = false;
publish_timestamp_t last_times = 0; /* unit:ms */
/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief   Func miaoshu
 *
 * @param[in] xxx1     Id of the message received.
 * @param[in] xxx2     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

publish_timestamp_t publish_timeNow(void)
{
    return timeFromPowerOn;
}


static inline uint32_t calculate_publish_period(const publish_period_t * p_period)
{
    switch (p_period->step_resolution)
    {
        case MODEL_PUBLISH_RESOLUTION_100MS:
            return p_period->num_steps;
        case MODEL_PUBLISH_RESOLUTION_1S:
            return p_period->num_steps * SEC_TO_100MS(1);   /* 1s = 10 * 100 ms */
        case MODEL_PUBLISH_RESOLUTION_10S:
            return p_period->num_steps * SEC_TO_100MS(10);  /* 10 s = 100 * 100 ms */
        case MODEL_PUBLISH_RESOLUTION_10MIN:
            return p_period->num_steps * SEC_TO_100MS(600); /* 10 m = 600 s = 6000 * 100 ms */
        default:
            return 0;
    }
}

static inline uint32_t calculate_publish_target(model_publish_state_t * p_state)
{
    return publish_timeNow() + calculate_publish_period(&p_state->period);
}

static model_publish_list_t * model_publish_alloc(void)
{
    model_publish_list_t  * buf = (model_publish_list_t  *)pvPortMalloc(sizeof(model_publish_list_t ));
    return buf;
}

static void model_publish_free(model_publish_list_t * p_list)
{
    vPortFree(p_list);
}

static void publication_list_insert(model_publish_list_t * p_pubstate_list)
{

    model_publish_list_t * p_current = mp_publication_list;
    model_publish_list_t * p_previous = NULL;
    p_pubstate_list->target = calculate_publish_target(p_pubstate_list->p_pubstate);

    while (p_current != NULL && !TIMER_OLDER_THAN(p_pubstate_list->target, p_current->target))
    {
        p_previous = p_current;
        p_current = p_current->p_next;
    }

    if (p_previous == NULL)
    {
        p_pubstate_list->p_next = p_current;
        mp_publication_list = p_pubstate_list;
    }
    else if (p_current != NULL)
    {
        p_previous->p_next = p_pubstate_list;
        p_pubstate_list->p_next = p_current;
    }
    else
    {
        p_previous->p_next = p_pubstate_list;
        p_pubstate_list->p_next = NULL;
    }
}

static void add_to_publication_list(model_publish_state_t * p_pubstate, model_publish_timeout_cb_t cb, void * cookie)
{
    model_publish_list_t * p_add = model_publish_alloc();
    if(NULL == p_add)
    {
        return;
    }
    p_add->p_next = NULL;
    p_add->p_pubstate = p_pubstate;
    p_add->cookie = cookie;
    p_add->cb = cb;
    publication_list_insert(p_add);
}

static publish_timestamp_t model_publish_get_new_timestamp(void)
{
    model_publish_list_t * p_current = mp_publication_list;

    return p_current->target;
}

void model_publish_period_set(model_publish_state_t * p_pubstate, model_publish_timeout_cb_t cb, void * cookie)
{
    if(NULL == p_pubstate || NULL == cb)
    {
        return;
    }
    model_publish_list_t * p_previous = NULL;
    model_publish_list_t * p_current = mp_publication_list;
    model_publish_list_t * old_head  = mp_publication_list;
    while (p_current != NULL && p_current->p_pubstate != p_pubstate)
    {
        p_previous = p_current;
        p_current = p_current->p_next;
    }

    if (p_current != NULL) 
    {
        if (p_previous == NULL) 
        {
            mp_publication_list = p_current->p_next;
        }
        else
        {
            p_previous->p_next = p_current->p_next;
        }
        model_publish_free(p_current);
    }
    if (p_pubstate->period.num_steps  != 0) /* Add publication event to the list; */
    {
        add_to_publication_list(p_pubstate, cb, cookie);
    }

    if(old_head == NULL && mp_publication_list != NULL)
    {
        if(modelPublishTimer != NULL)
            mesh_timer_start(modelPublishTimer);

        m_publish_timer_running = true;
    }
    else if(old_head != NULL && mp_publication_list == NULL)
    {
        if(modelPublishTimer != NULL)
            mesh_timer_stop(modelPublishTimer);

        m_publish_timer_running = false;
    }
    last_times = model_publish_get_new_timestamp();
}


static void publish_period_timer_handler(void)
{
    while (mp_publication_list != NULL && (mp_publication_list->target == publish_timeNow() || TIMER_OLDER_THAN(mp_publication_list->target,publish_timeNow())))
    {
        model_publish_list_t * p_pubstate_list = mp_publication_list;
        mp_publication_list = mp_publication_list->p_next;

        p_pubstate_list->cb(p_pubstate_list->cookie);
        publication_list_insert(p_pubstate_list);
    }
    last_times = model_publish_get_new_timestamp();
    if(mp_publication_list != NULL)
    {
        m_publish_timer_running = true;
    }
}

static void  publishTimeoutNotify(mesh_timer_t xTimer)
{  
   // static  uint32_t timeflag = 0;
    if(m_publish_timer_running)
    {
        if(timeFromPowerOn >= last_times)
        {
            mesh_run(publish_period_timer_handler,0,false);
            m_publish_timer_running = false;
        }
    }
}

static void publishIncTimer(mesh_timer_t xTimer)
{
   timeFromPowerOn = timeFromPowerOn + 1;
}
 
static void publishTimeoutCallback(mesh_timer_t xTimer)
{
    publishIncTimer(xTimer);
    publishTimeoutNotify(xTimer);
}
 


void mesh_model_publish_timer_init(void)
{
     modelPublishTimer = mesh_timer_create("modelPublishTimer",pdMS_TO_TICKS(MESH_PUBLISH_COUNTER_TICK),pdTRUE,(void *)0,publishTimeoutCallback);
#if 0
     if(modelPublishTimer != NULL)
            xTimerStart(modelPublishTimer,0);
#endif
}

void mesh_model_publish_init(void)
{
    mesh_model_publish_timer_init();
}

