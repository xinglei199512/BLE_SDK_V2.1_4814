#ifndef GENERIC_COMMON__H
#define    GENERIC_COMMON__H
#include <stdint.h>
#include "timer_wrapper.h"

#define GENERIC_MSG_MAX   12  //max number of messages in 6 sec to this model
#define GENERIC_TIMER_MAX 4   //handle onoff delay value,maybe a lot of message with delay field.

#define TRANSITION_TIME_STEP_MASK               (0xC0)
#define TRANSITION_TIME_STEP_100MS_FACTOR       (100)
#define TRANSITION_TIME_STEP_1S_FACTOR          (1000)
#define TRANSITION_TIME_STEP_10S_FACTOR         (10*1000)
#define TRANSITION_TIME_STEP_10M_FACTOR         (10*60*1000)


#define TRANSITION_TIME_STEP_RESOLUTION_100MS   (0x00)
#define TRANSITION_TIME_STEP_RESOLUTION_1S      (0x40)
#define TRANSITION_TIME_STEP_RESOLUTION_10S     (0x80)
#define TRANSITION_TIME_STEP_RESOLUTION_10M     (0xC0)


#define TRANSITION_TIME_STEP_100MS_MAX          (6200ul)
#define TRANSITION_TIME_STEP_1S_MAX             (62000ul)
#define TRANSITION_TIME_STEP_10S_MAX            (620000ul)
#define TRANSITION_TIME_STEP_10M_MAX            (37201275ul) // notify delay timer
#define TRANSITION_TIME_MAX                     (0x3E)
#define TRANSITION_TIME_UNKNOWN                 (0x3F)

#define pdS_TO_TICKS( xTimeIns ) ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeIns ) * ( TickType_t ) configTICK_RATE_HZ )) )
#define TICKS_TO_pdS( xTimeTICKS ) ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeTICKS) / ( TickType_t ) configTICK_RATE_HZ )))
#define TICKS_TO_pdMS( xTimeTICKS ) ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeTICKS) * 1000 / ( TickType_t ) configTICK_RATE_HZ )))

#define BIT(a) (1 << (a))

typedef void (*user_set_current_value_cb)(uint8_t, uint16_t);
typedef uint8_t (*user_get_current_state_cb)(void);

enum GENERIC_TRANS_STATE_T
{
    GENERIC_TRANS_IDALE = 0x00,
    GENERIC_TRANS_PROCESS
    
};

enum
{
    GENERIC_LEVEL_UNRESOLVE = 0x00 ,
    GENERIC_LEVEL_DELTA_TRANSACTING ,
    GENERIC_LEVEL_DELTA_TRANSACTION_START,
    GENERIC_LEVEL_DELTA_TRANSACTION_ABORD
};

enum
{
    DELAY_TRANS_TIMER_STEPS = 0,
    DELAY_TIMER_STEPS       = 1,
    TRANS_TIMER_100MS_STEPS = 20,
    TRANS_TIMER_1S_STEPS    = 20,
    TRANS_TIMER_10S_STEPS   = 1000,
    TRANS_TIMER_10M_STEPS   = 60000
};

enum
{
    GENERIC_DELAY_TRANS_TIMER_ONOFF = 1,
    GENERIC_DELAY_TRANS_TIMER_LEVEL = 2,
    GENERIC_DELAY_TRANS_TIMER_DELTA = 3,
    GENERIC_DELAY_TRANS_TIMER_MOVE = 4,
    GENERIC_DELAY_TRANS_TIMER_POWER_ONOFF,
    GENERIC_DELAY_TRANS_TIMER_LIGHT_LIGHTNESS_ACTUAL,
    GENERIC_DELAY_TRANS_TIMER_LIGHT_LIGHTNESS_LINEAR,
    GENERIC_DELAY_TRANS_TIMER_LIGHT_CTL,
    GENERIC_DELAY_TRANS_TIMER_LIGHT_CTL_TEMPERATURE,
    GENERIC_DELAY_TRANS_TIMER_LIGHT_CTL_DELTA_UV,
    GENERIC_DELAY_TRANS_TIMER_LIGHT_HSL,
    GENERIC_DELAY_TRANS_TIMER_LIGHT_HSL_HUE,
    GENERIC_DELAY_TRANS_TIMER_LIGHT_HSL_SATURATION,
    GENERIC_DELAY_TRANS_TIMER_SCENE_RECALL,
};

enum
{
    BUTTON_SEND_PKT_STATUS = 0,
    WAIT_FOR_GENERIC_ONOFF_STATUS = 1,
    WAIT_FOR_GENERIC_LEVEL_STATUS = 2,
    WAIT_FOR_LIGHT_LIGHTNESS_STATUS = 3,
    WAIT_FOR_LIGHT_CTL_STATUS = 4,
    WAIT_FOR_LIGHT_HSL_STATUS = 5,
};

enum
{
    USE_FOR_DELAY_TIME = 0,
    USE_FOR_TRANS_TIME = 1,
};

typedef struct
{
    uint8_t num_steps: 6,   //low bit
        step_resolution: 2; //high bit
}generic_transition_time_t;


typedef struct
{
    void *inst;
    uint8_t mask;
}inst_param_t;


typedef struct
{
    uint16_t src;
    uint16_t dst;
    uint8_t tid;
    uint16_t interval_ms;
}generic_valid_field_check_t;

typedef struct
{
    generic_valid_field_check_t data[GENERIC_MSG_MAX];
    inst_param_t inst_param;
    mesh_timer_t Timer;
    uint8_t count;
    uint8_t front;
    uint8_t rear;
}generic_valid_field_queue_t;


typedef void(*user_generic_server_action_cb)(void *timer_param);
typedef struct
{
    void *inst;
    mesh_timer_t Timer;
    user_generic_server_action_cb action_cb;
    uint32_t remain_tick_count;
    uint16_t target_value;
    uint16_t trans_timer_step;
    uint8_t trans_time;
    uint8_t dt_timer_flag;
    uint16_t type;
}generic_delay_trans_param_t;


generic_transition_time_t generic_time_transition_cal(uint32_t tick);

int generic_msg_queue_push(generic_valid_field_queue_t *queue,generic_valid_field_check_t value);

generic_valid_field_check_t generic_msg_queue_pop(generic_valid_field_queue_t *queue);

uint8_t  generic_msg_queue_search(generic_valid_field_queue_t *queue,generic_valid_field_check_t value);
uint8_t  generic_msg_queue_search_last(generic_valid_field_queue_t *queue,generic_valid_field_check_t value);


uint8_t generic_timer_free_get(uint16_t mask);
uint32_t generic_transition_time_cal(uint8_t *trans_time);



uint8_t model_transition_time_encode(uint32_t transition_time);

uint32_t model_transition_time_decode(uint8_t enc_transition_time, uint16_t *trans_timer_step);


uint8_t generic_msg_queue_init(generic_valid_field_queue_t *queue);

generic_valid_field_check_t generic_get_front_msg_queue(generic_valid_field_queue_t *queue);

generic_valid_field_check_t generic_get_rear_msg_queue(generic_valid_field_queue_t *queue);

uint16_t generic_calculate_6s_tid_time(generic_valid_field_queue_t *queue);
int generic_deal_recv_6s_tid_pkt(generic_valid_field_queue_t *queue, generic_valid_field_check_t *p_msg_field);

uint32_t generic_get_delay_trans_timer_ticks(uint8_t delay, uint8_t trans_time, uint16_t *trans_timer_step);

uint32_t generic_get_delay_trans_expiry(generic_delay_trans_param_t *timer_param, uint8_t trans_time, uint16_t trans_timer_step);

void generic_deal_delay_trans_func(generic_delay_trans_param_t *timer_param, uint8_t delay, uint8_t trans, user_generic_server_action_cb cb);
#endif





