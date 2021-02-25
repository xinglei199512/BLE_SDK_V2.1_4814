#include "osapp_config.h"
#include "generic_common.h"
#include "mesh_model.h"
#include "mesh_queued_msg.h"

int generic_msg_queue_push(generic_valid_field_queue_t *queue, generic_valid_field_check_t value)
{
    uint8_t posi;
    if(queue->count < GENERIC_MSG_MAX)
    {
        queue->data[queue->rear] = value;
        posi = queue->rear;
        queue->rear = (queue->rear+1)%GENERIC_MSG_MAX;
        queue->count++;
        return posi;
    }
    else
    {
        return -1;//keep the compiler from issuing warnings
    }
     
}

generic_valid_field_check_t generic_msg_queue_pop(generic_valid_field_queue_t *queue)
{
    generic_valid_field_check_t value;
    //LOG(3, "generic_msg_queue_pop count:%d rear:%x front:%x\n", queue->count, queue->rear, queue->front);
    if(queue->count > 0)
    {
        value = queue->data[queue->front];
        queue->front = (queue->front+1)%GENERIC_MSG_MAX;
        queue->count--;
        return value;
    }
    else
    {
        BX_ASSERT(0);
        value.src = 0;
        return value;//keep the compiler from issuing warnings
    }
} 

generic_valid_field_check_t generic_get_front_msg_queue(generic_valid_field_queue_t *queue)
{
    generic_valid_field_check_t value;
    if(queue->count > 0)
    {
        value = queue->data[queue->front];
        return value;
    }
    else
    {
       value.src = 0;
       return value;//keep the compiler from issuing warnings
    }
}

uint8_t generic_get_msg_queue_count(generic_valid_field_queue_t *queue)
{
    uint8_t count = 0xff;
    if(queue) {
        count = queue->count;
    }

    return count;
}

generic_valid_field_check_t generic_get_rear_msg_queue(generic_valid_field_queue_t *queue)
{
    generic_valid_field_check_t value;
    if(queue->count > 0)
    {
        value = queue->data[queue->rear - 1];
        return value;
    }
    else
    {
        value.src = 0;//keep the compiler from issuing warnings
        return value;
    }
}

uint16_t  generic_search_all_msg_queue_interval_ms(generic_valid_field_queue_t *queue)
{

    uint8_t i;
    uint16_t interval_ms = 0;
    i = queue->front;
    i = (i + 1)%GENERIC_MSG_MAX;
    while(i != queue->rear)
    {
        interval_ms += queue->data[i].interval_ms;
        i = (i + 1)%GENERIC_MSG_MAX;
    }

    return interval_ms;
}

// search dst src tid have no change within 6 second
uint8_t  generic_msg_queue_search(generic_valid_field_queue_t *queue,generic_valid_field_check_t value)
{

    uint8_t i;
    i = queue->front;
    while(i != queue->rear)
    {
        if(queue->data[i].dst == value.dst && queue->data[i].src == value.src && queue->data[i].tid == value.tid)
        {
            return 1;
        }
        i = (i + 1)%GENERIC_MSG_MAX;
    }

    return 0;
}
// search dst src tid have no change within 6 second
uint8_t  generic_msg_queue_search_last(generic_valid_field_queue_t *queue,generic_valid_field_check_t value)
{

    uint8_t i;
    i = queue->front;
    if(queue->front == queue->rear)
    {
        return GENERIC_LEVEL_DELTA_TRANSACTION_START;
    }
    else if(queue->data[i].dst != value.dst || queue->data[i].src != value.src )
    {
        return GENERIC_LEVEL_DELTA_TRANSACTION_ABORD;
    }
    else if(queue->data[i].dst == value.dst && queue->data[i].src == value.src && queue->data[i].tid == value.tid)
    {
        return GENERIC_LEVEL_DELTA_TRANSACTING;
    }    
    return GENERIC_LEVEL_UNRESOLVE;
}

//return unit is ms
uint32_t generic_transition_time_cal(uint8_t *trans_time)
{
    generic_transition_time_t *time_tran = (generic_transition_time_t *)trans_time;
    LOG(3,"time_tran=0x%x,num_steps=0x%x,step_resolution=0x%x\n",*time_tran,time_tran->num_steps,time_tran->step_resolution);
    uint32_t ms100_value = time_tran->num_steps;
    switch(time_tran->step_resolution)
    {
        case Milliseconds_100:
            ms100_value = ms100_value *  1;break;
        case Second_1:
            ms100_value = ms100_value *  10;break;
        case Seconds_10:
            ms100_value = ms100_value *  100;break;
        case Minutes_10:
            ms100_value = ms100_value *  6000;break;
    }
    return ms100_value * 100;
}

uint8_t model_transition_time_encode(uint32_t transition_time)
{
    uint8_t enc_time = TRANSITION_TIME_UNKNOWN;

    if(transition_time <= TRANSITION_TIME_STEP_100MS_MAX)
    {
        enc_time = (transition_time / TRANSITION_TIME_STEP_100MS_FACTOR) | TRANSITION_TIME_STEP_RESOLUTION_100MS;
    }
    else if (transition_time <= TRANSITION_TIME_STEP_1S_MAX)
    {
        enc_time = (transition_time / TRANSITION_TIME_STEP_1S_FACTOR) | TRANSITION_TIME_STEP_RESOLUTION_1S;
    }
    else if (transition_time <= TRANSITION_TIME_STEP_10S_MAX)
    {
        enc_time = (transition_time / TRANSITION_TIME_STEP_10S_FACTOR) | TRANSITION_TIME_STEP_RESOLUTION_10S;
    }
    else if (transition_time <= TRANSITION_TIME_STEP_10M_MAX)
    {
        enc_time = (transition_time / TRANSITION_TIME_STEP_10M_FACTOR) | TRANSITION_TIME_STEP_RESOLUTION_10M;
    }

    return enc_time;
}

uint32_t model_transition_time_decode(uint8_t enc_transition_time, uint16_t *trans_timer_step)
{
    uint32_t time = 0;

    if ((enc_transition_time & ~TRANSITION_TIME_STEP_MASK) == TRANSITION_TIME_UNKNOWN)
    {
        return 0;
    }

    switch(enc_transition_time & TRANSITION_TIME_STEP_MASK)
    {
        case TRANSITION_TIME_STEP_RESOLUTION_100MS:
            time = (enc_transition_time & ~TRANSITION_TIME_STEP_MASK) * TRANSITION_TIME_STEP_100MS_FACTOR;
            *trans_timer_step = TRANS_TIMER_100MS_STEPS;
        break;

        case TRANSITION_TIME_STEP_RESOLUTION_1S:
            time = (enc_transition_time & ~TRANSITION_TIME_STEP_MASK) * TRANSITION_TIME_STEP_1S_FACTOR;
            *trans_timer_step = TRANS_TIMER_1S_STEPS;
        break;

        case TRANSITION_TIME_STEP_RESOLUTION_10S:
            time = (enc_transition_time & ~TRANSITION_TIME_STEP_MASK) * TRANSITION_TIME_STEP_10S_FACTOR;
            *trans_timer_step = TRANS_TIMER_10S_STEPS;
        break;

        case TRANSITION_TIME_STEP_RESOLUTION_10M:
            time = (enc_transition_time & ~TRANSITION_TIME_STEP_MASK) * TRANSITION_TIME_STEP_10M_FACTOR;
            *trans_timer_step = TRANS_TIMER_10M_STEPS;
        break;

        default:
            break;
    }

    return time;
}

//convert remaining time to trans_time
generic_transition_time_t generic_time_transition_cal(uint32_t tick)
{
    uint8_t tick_ms = 20;
    generic_transition_time_t time_tran; 
    if(tick <= (6200/tick_ms)) //the range is 0 through 6.2 seconds
    {
        time_tran.step_resolution = Milliseconds_100;
        time_tran.num_steps = (tick * tick_ms)/100;
    }
    else if(tick <= (62000/tick_ms))
    {
        time_tran.step_resolution = Second_1;
        time_tran.num_steps = (tick * tick_ms)/1000;
    }
    else if(tick <= (620000/tick_ms))
    {
        time_tran.step_resolution = Seconds_10;
        time_tran.num_steps = (tick * tick_ms)/10000;
    }
    else
    {
        time_tran.step_resolution = Minutes_10;
        time_tran.num_steps = (tick * tick_ms)/600000;
    }
    return time_tran;
}

uint8_t generic_timer_free_get(uint16_t mask)
{
    uint8_t i;
    for(i = 0;i < GENERIC_TIMER_MAX;i++)
    {
        if(((1<<i)&(mask)) == 0)
            return i;
    }
    BX_ASSERT(0);
    return 0;//keep the compiler from issuing warnings
}

uint16_t generic_calculate_6s_tid_time(generic_valid_field_queue_t *queue)
{
    uint16_t interval_ms;
    uint16_t timer_expiry;
    uint16_t queue_interval_ms;
    if(queue->Timer) {
        uint8_t msg_count = generic_get_msg_queue_count(queue);
        timer_expiry = (uint16_t)(mesh_timer_get_remain_time(queue->Timer));
        queue_interval_ms = generic_search_all_msg_queue_interval_ms(queue);
        if(msg_count == 1) {
            interval_ms = (uint16_t)(pdMS_TO_TICKS(6000) - timer_expiry);
        }else {
            interval_ms = (uint16_t)(pdMS_TO_TICKS(6000) - queue_interval_ms - timer_expiry);

            //interval_ms = interval_ms > 0x12c ? (uint16_t)(0x12c - queue_interval_ms) : interval_ms;
        }

        LOG(3,"generric_calculate_6s_tid_time msg_count:%x msg:%x queue%x expiry:%x\n",
                msg_count, interval_ms, queue_interval_ms, timer_expiry);
        if(interval_ms == 0)
            interval_ms = 1;
    }else {
        interval_ms = (uint16_t)pdMS_TO_TICKS(6000);
    }
    return interval_ms;
}

uint32_t generic_get_delay_trans_timer_ticks(uint8_t delay, uint8_t trans_time, uint16_t *trans_timer_step)
{
    uint32_t normal_msg_ticks;
    if(delay) {
        normal_msg_ticks = pdMS_TO_TICKS(delay * 5);
        *trans_timer_step = DELAY_TIMER_STEPS;
    }
    else {
        normal_msg_ticks = pdMS_TO_TICKS(model_transition_time_decode(trans_time, trans_timer_step));
       // *trans_timer_step = TRANS_TIMER_100MS_STEPS;
    }

    return normal_msg_ticks;
}

uint32_t generic_get_delay_trans_expiry(generic_delay_trans_param_t *timer_param, uint8_t trans_time, uint16_t trans_timer_step)
{
    uint32_t delay_trans_expiry =  0;
    uint16_t trans_timer_tmp;
    if(timer_param->Timer == NULL)
        return 0;

    if(trans_timer_step == DELAY_TRANS_TIMER_STEPS) {
        delay_trans_expiry = 0;
    }
    else if(trans_timer_step == DELAY_TIMER_STEPS) {
        delay_trans_expiry = TICKS_TO_pdMS(mesh_timer_get_remain_time(timer_param->Timer)) + model_transition_time_decode(trans_time, &trans_timer_tmp);
    }
    else {
        delay_trans_expiry = TICKS_TO_pdMS(mesh_timer_get_remain_time(timer_param->Timer) + timer_param->remain_tick_count);
    }

    return delay_trans_expiry;
}

static void handle_generic_six_sec_timer(mesh_timer_t thandle)
{
    generic_valid_field_check_t value;
    //LOG(3,"handle_generic_six_sec_timer\n");
    generic_valid_field_queue_t *queue = (generic_valid_field_queue_t *)mesh_timer_get_associated_data(thandle);
    generic_msg_queue_pop(queue);
    value = generic_get_front_msg_queue(queue);

    if(value.src != 0) {
        mesh_timer_change_period(queue->Timer, value.interval_ms);
    }else {
        mesh_timer_delete(queue->Timer);
        queue->Timer = NULL;
    }
    //LOG(3,"mesh_timer_t=0x%x,value.Timer=0x%x\n",thandle, queue->Timer);
}

int generic_deal_recv_6s_tid_pkt(generic_valid_field_queue_t *queue, generic_valid_field_check_t *p_msg_field)
{
    int ret = 0;
    int8_t pos = 0;

    ret = generic_msg_queue_search(queue, *p_msg_field);
    if(ret) {
         LOG(3,"TID SAME , DISCARD THIS MESSAGE\n");
         return ret;
    }

    p_msg_field->interval_ms = generic_calculate_6s_tid_time(queue);

    pos = generic_msg_queue_push(queue, *p_msg_field);
    if(pos == -1) {
        uint16_t timer_expiry = (uint16_t)(mesh_timer_get_remain_time(queue->Timer));
        generic_msg_queue_pop(queue);
        generic_valid_field_check_t value = generic_get_front_msg_queue(queue);
        mesh_timer_change_period(queue->Timer, value.interval_ms + timer_expiry);
        pos = generic_msg_queue_push(queue, *p_msg_field);
        LOG(3,"pos:%d timer_expiry:%x value.interval_ms:%x\n", pos, timer_expiry, value.interval_ms);
    }
    if(queue->Timer == NULL) {
        queue->Timer = mesh_timer_create(
                "tid_Timer", pdMS_TO_TICKS(6000), pdFALSE, (void *)queue, handle_generic_six_sec_timer); //6s

        if(queue->Timer != NULL)
            mesh_timer_start(queue->Timer);
    }

    return ret;
}

static void handler_generic_transition_timer(mesh_timer_t thandle)
{
    generic_delay_trans_param_t  *timer_param = (generic_delay_trans_param_t *)mesh_timer_get_associated_data(thandle);

    LOG(3,"handler_generic_transition_timer\n");

    if(timer_param->Timer)
    {
        mesh_timer_delete(timer_param->Timer);
        timer_param->Timer = NULL;
    }

    timer_param->trans_timer_step = DELAY_TRANS_TIMER_STEPS;
    timer_param->dt_timer_flag = USE_FOR_TRANS_TIME;
    timer_param->action_cb(timer_param);
}

static void generic_transition_timer_deal(mesh_timer_t thandle)
{
    mesh_queued_msg_send(handler_generic_transition_timer, thandle);
}

static void generic_delay_trans_timer_deal(mesh_timer_t thandle);
typedef void(*generic_timer_cb)(mesh_timer_t thandle);
static void generic_delay_trans_timer(generic_delay_trans_param_t *timer_param)
{
    if(timer_param->trans_time == 0) {
        timer_param->dt_timer_flag = USE_FOR_DELAY_TIME;
        timer_param->action_cb(timer_param);
    }else {
        uint32_t ticks;
        generic_timer_cb cb;
        if(timer_param->remain_tick_count == 0 && timer_param->type == GENERIC_DELAY_TRANS_TIMER_ONOFF && timer_param->target_value == 1) /* turn on */
        {
            timer_param->dt_timer_flag = USE_FOR_DELAY_TIME;
            timer_param->action_cb(timer_param);
        }

        //LOG(3,"generic_delay_trans_timer remain_tick_count:%d type:%x\n", timer_param->remain_tick_count, timer_param->type);
        if(timer_param->remain_tick_count == 0)
            timer_param->remain_tick_count = generic_get_delay_trans_timer_ticks(0, timer_param->trans_time, &timer_param->trans_timer_step);
        else {
            timer_param->dt_timer_flag = USE_FOR_DELAY_TIME;
            timer_param->action_cb(timer_param);
        }

        //LOG(3,"generic_delay_trans_timer remain_tick_count:%x timer_step:%x\n", timer_param->remain_tick_count, pdMS_TO_TICKS(timer_param->trans_timer_step));
        if(timer_param->remain_tick_count > pdMS_TO_TICKS(timer_param->trans_timer_step)) {
            ticks = pdMS_TO_TICKS(timer_param->trans_timer_step);
            timer_param->remain_tick_count -= ticks;
            cb = generic_delay_trans_timer_deal;
        }else {
            ticks = timer_param->remain_tick_count;
            timer_param->remain_tick_count = 0;
            cb = generic_transition_timer_deal;
        }
        //LOG(3,"generic_delay_trans_timer ticks=0x%x remain_tick_count:%x\n", ticks, timer_param->remain_tick_count);
        if(timer_param->Timer == NULL)
        {    
            timer_param->Timer = mesh_timer_create(
                "Timer", ticks, pdFALSE, (void *)timer_param, cb);//transtion timer
        }

        if(timer_param->Timer != NULL)
            mesh_timer_change_period(timer_param->Timer,ticks);
    }
}

static void handler_generic_delay_trans_timer(mesh_timer_t thandle)
{
    //LOG(3, "handler_generic_delay_trans_timer\n");
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)mesh_timer_get_associated_data(thandle);

    if(timer_param->Timer) {
        mesh_timer_delete(timer_param->Timer);
        timer_param->Timer = NULL;
    }
    generic_delay_trans_timer(timer_param);

}

static void generic_delay_trans_timer_deal(mesh_timer_t thandle)
{
    mesh_queued_msg_send(handler_generic_delay_trans_timer, thandle);
}

void generic_deal_delay_trans_func(generic_delay_trans_param_t *timer_param, uint8_t delay, uint8_t trans, user_generic_server_action_cb cb)
{
    timer_param->action_cb = cb;
    timer_param->trans_timer_step = DELAY_TRANS_TIMER_STEPS;
    timer_param->remain_tick_count = 0;

    LOG(3, "generic_deal_delay_trans_func delay:%x trans:%x\n", delay, trans);
    if(delay == 0 && trans == 0) {
        timer_param->dt_timer_flag = USE_FOR_TRANS_TIME;
        timer_param->action_cb(timer_param);
    }else if(delay == 0) {
        generic_delay_trans_timer(timer_param);
    }else {
        uint32_t normal_msg_ticks = generic_get_delay_trans_timer_ticks(delay, trans, &timer_param->trans_timer_step);
        if(timer_param->Timer == NULL)
        {
            timer_param->Timer= mesh_timer_create(
                    "Timer", normal_msg_ticks, pdFALSE, (void *)timer_param, handler_generic_delay_trans_timer);
        }
        if(timer_param->Timer != NULL)
            mesh_timer_change_period(timer_param->Timer,normal_msg_ticks);

    }
}

