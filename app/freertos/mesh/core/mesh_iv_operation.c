

#include "osapp_config.h"

#include "sdk_mesh_config.h"
#include "mesh_env.h"
#include "beacon.h"
#include "co_endian.h"

#include "mesh_iv_operation.h"
#include "mesh_iv_operation_ex.h"
#include "node_save.h"
#include "mesh_queued_msg.h"
#include "friend.h"
#ifdef MESH_UART_DEBUG_TEST_CMD
#include "uart_debug_test.h"
#endif /*MESH_UART_DEBUG_TEST_CMD*/

/*
 * DEFINES
 ****************************************************************************************
 */
/** Maximum difference in sequence numbers between two SAR segments of the same sessio */
#define TRANSPORT_SAR_SEQNUM_DIFF_MAX           (8191)
/** The minimum time between IV updates, in minutes. */
#define MESH_MIN_IV_UPDATE_INTERVAL_MINUTES     (96 * 60)
//#define MESH_IV_UPDATE_TIMER_INTERVAL_US        (60000000) /**< 60 seconds. */
#define MESH_IV_UPDATE_TICK                     (1000 * 60 * 60) //unit: ms
/** Longest time we're allowed to stay in an IV update state */
#define MESH_MAX_IV_UPDATE_INTERVAL_MINUTES     (144 * 60)
/** The minimum time between IV Recovery, in minutes. */
#define MESH_MIN_IV_RECOVERY_INTERVAL_MINUTES   (192 * 60)
/** Margin to ensure we're inside of the Mesh Profile Specification v1.0 time limits */
#define MESH_IV_UPDATE_TIME_MARGIN_MINUTES      (10)
#define MESH_IV_MS_TO_MINUTES                   (MESH_IV_UPDATE_TICK/1000/60)
/** ms to tick. */
#define IV_UPDATE_TIMEOUT                       (MESH_MIN_IV_UPDATE_INTERVAL_MINUTES/MESH_IV_MS_TO_MINUTES)
#define IV_RECOVERY_TIMEOUT                     (MESH_MIN_IV_RECOVERY_INTERVAL_MINUTES/MESH_IV_MS_TO_MINUTES)
#define IV_UPDATE_IN_PROGRESS_MAX_TIME          (MESH_MAX_IV_UPDATE_INTERVAL_MINUTES/MESH_IV_MS_TO_MINUTES - MESH_IV_UPDATE_TIME_MARGIN_MINUTES/MESH_IV_MS_TO_MINUTES)

#ifndef NETWORK_SEQNUM_IV_UPDATE_START_THRESHOLD
#define NETWORK_SEQNUM_IV_UPDATE_START_THRESHOLD (NETWORK_SEQNUM_MAX / 2)
#endif

#define NETWORK_IVI_MASK                        (0x00000001)
/** Number of bits in the sequence number. */
#define NETWORK_SEQNUM_BITS                     24
/** Maximum allowed sequence number. */
#define NETWORK_SEQNUM_MAX                      ((1 << NETWORK_SEQNUM_BITS) - 1)
#define MESH_NAX_SEQNUM                         ( NETWORK_SEQNUM_MAX + 1)

/**
 * The sequence number value that triggers the end of an IV update procedure.
 * This value should be set so that there are enough sequence numbers left for finishing any ongoing Transport SAR sessions.
 */
#ifndef NETWORK_SEQNUM_IV_UPDATE_END_THRESHOLD
#define NETWORK_SEQNUM_IV_UPDATE_END_THRESHOLD (NETWORK_SEQNUM_MAX - TRANSPORT_SAR_SEQNUM_DIFF_MAX)
#endif

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
typedef enum
{
    /** In normal operation. */
    MESH_IV_UPDATE_NORMAL,
    /** IV update procedure in progress. */
    MESH_IV_UPDATE_IN_PROGRESS,
} mesh_iv_update_t;

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

typedef struct
{
    uint32_t seqnum_max_available; /**< The highest sequence number available for use. */
    uint32_t iv_index; /**< IV index. */
    struct
    {
        mesh_iv_update_t state;   /**< IV update state. */
        bool pending; /**< An IV update state change has been triggered, awaiting timer. */
        bool locked; /**< Changes to IV index or update state has been locked by an external module. */
        uint16_t timeout_counter; /**< Counter for IV update procedure timeout, in minutes. */
        uint16_t ivr_timeout_counter; /**< Counter for IV recovery procedure timeout, in minutes. */
    } iv_update;
} network_state_t;

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static uint8_t m_test_mode = 0;
//static uint8_t mesh_iv_init_done = 0;
static network_state_t g_iv_state;
static mesh_timer_t meshIVTimer;
static uint8_t mesh_iv_debug_switch = 0;

static uint32_t sequence_number;
/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static void iv_update_timer_handler(void);

/*
 * FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static void iv_update_reset_seq_num(void)
{
   sequence_number = 0;
}

uint32_t iv_update_get_seq_num(void)
{
   return sequence_number ;
}

void iv_update_set_seq_num(uint32_t value)
{
   sequence_number = value;
}

void iv_update_log(const void *parma,uint16_t len)
{
    if(mesh_iv_debug_switch == 0)
    {
        return;
    }
#ifdef MESH_UART_DEBUG_TEST_CMD
    mesh_debug_uart_test_tx(parma,len);
#endif /* MESH_UART_DEBUG_TEST_CMD */
    //uart_log_send_cmd(UART_PKTCMD_DEBUG_TX, parma, len);
}


void iv_update_log_event(mesh_iv_update_log_cmd cmd)
{
    uint32_t data = cmd;
    if(mesh_iv_debug_switch == 0)
    {
        return;
    }
    iv_update_log(&data,sizeof(uint32_t));
    
}

void iv_update_log_iv_index()
{
    if(mesh_iv_debug_switch == 0)
    {
        return;
    }
    uint32_t iv_index = g_iv_state.iv_index;
    iv_update_log(&iv_index,sizeof(uint32_t));
}



uint32_t alloc_seq_num(uint8_t num)
{
    uint32_t current = sequence_number;
    sequence_number += num;
    return current;
}


static void meshTimeoutCallback(mesh_timer_t xTimer)
{
    mesh_run(iv_update_timer_handler,0,false);
}

void mesh_iv_timer_init(void)
{
     meshIVTimer = mesh_timer_create("meshIVTimer",pdMS_TO_TICKS(MESH_IV_UPDATE_TICK),pdTRUE,(void *)0,meshTimeoutCallback);
     if(meshIVTimer != NULL)
                mesh_timer_start(meshIVTimer);
}

void mesh_iv_init(void)
{
    memset(&g_iv_state,0,sizeof(network_state_t));
    iv_update_reset_seq_num();
    g_iv_state.seqnum_max_available =  MESH_NAX_SEQNUM;
    mesh_iv_timer_init();
    //mesh_iv_init_done = 1;
}


void iv_update_relay_sec_beacon(void)
{
    beacon_send_to_all();
}

static inline bool iv_timeout_limit_passed(uint32_t timeout)
{
    return (timeout >= IV_UPDATE_TIMEOUT || m_test_mode);
}

uint32_t mesh_iv_update_start(void)
{
    uint32_t status;
   
    if (g_iv_state.iv_update.state == MESH_IV_UPDATE_NORMAL &&
        !g_iv_state.iv_update.locked &&
        iv_timeout_limit_passed(g_iv_state.iv_update.timeout_counter))
    {
        g_iv_state.iv_update.state = MESH_IV_UPDATE_IN_PROGRESS;
        g_iv_state.iv_update.timeout_counter = 0;
        g_iv_state.iv_index = g_iv_state.iv_index + 1;
        iv_update_log_iv_index();
//        iv_update_beacon_send_immediately();
 //       iv_update_beacon_start();
        status = MESH_OK;
    }
    else
    {
        status = MESH_FAIL;
    }
    
    return status;
}


/** Trigger any pending IV update state transitions, if the state is unlocked. */
static bool iv_update_trigger_if_pending(void)
{
    bool pending = ( g_iv_state.iv_update.pending  && !g_iv_state.iv_update.locked &&
                    iv_timeout_limit_passed(g_iv_state.iv_update.timeout_counter));

    if (pending)
    {
        g_iv_state.iv_update.pending = false;
        switch (g_iv_state.iv_update.state)
        {
            case MESH_IV_UPDATE_NORMAL:
                iv_update_log_event(IV_UPDATE_START);
                BX_ASSERT(mesh_iv_update_start() == MESH_OK);
                iv_update_relay_sec_beacon();
                break;
            case MESH_IV_UPDATE_IN_PROGRESS:
                iv_update_log_event(IV_UPDATE_TO_NORMAL);
                iv_update_log_iv_index();
                iv_update_reset_seq_num();
                g_iv_state.iv_update.state = MESH_IV_UPDATE_NORMAL;
                g_iv_state.iv_update.timeout_counter = 0;
                iv_update_relay_sec_beacon();
               
                break;
            default:
                BX_ASSERT(0); /* Unimplemented state. */
        }
    }
    return pending;
}

static void iv_update_timer_handler(void)
{
    bool increment_counter = true;
    switch (g_iv_state.iv_update.state)
    {
        case MESH_IV_UPDATE_NORMAL:
            if (iv_timeout_limit_passed(g_iv_state.iv_update.timeout_counter))
            {
                /* After 96 hours of operating in Normal Operation, a node may initiate the IV Update procedure 
                   by lock
                */
                (void) iv_update_trigger_if_pending();
                increment_counter = false;
            }
            break;
        case MESH_IV_UPDATE_IN_PROGRESS:
             {
                /* max iv update time . */
                if (g_iv_state.iv_update.timeout_counter >= IV_UPDATE_IN_PROGRESS_MAX_TIME)
                {
                    g_iv_state.iv_update.pending = true;
                    increment_counter = false;
                }
                (void) iv_update_trigger_if_pending();
             }
             break;
        default:
            BX_ASSERT(0); /* Unimplemented state */
    }
    if (increment_counter)
    {
        g_iv_state.iv_update.timeout_counter++;
    }

    if (g_iv_state.iv_update.ivr_timeout_counter > 0)
    {
        g_iv_state.iv_update.ivr_timeout_counter--;
    }
}

static void beacon_received(const uint8_t * p_network_id, uint32_t iv_index, bool iv_update, bool key_refresh)
{
    if (g_iv_state.iv_update.state == MESH_IV_UPDATE_NORMAL)
    {
        if (g_iv_state.iv_update.ivr_timeout_counter == 0 &&
            ((!iv_update && iv_index > g_iv_state.iv_index) ||
             (iv_update && iv_index > g_iv_state.iv_index + 1)))
        {
            if (iv_index <= (g_iv_state.iv_index + MESH_IV_RECOVERY_LIMIT) &&
                !g_iv_state.iv_update.locked)
            {
                /*  silently IV index. */
                g_iv_state.iv_index = iv_index;
                iv_update_reset_seq_num();
                g_iv_state.iv_update.timeout_counter = 0;
                g_iv_state.iv_update.ivr_timeout_counter = IV_RECOVERY_TIMEOUT;
                g_iv_state.iv_update.pending = false;
                g_iv_state.iv_update.state = MESH_IV_UPDATE_NORMAL;
                iv_update_relay_sec_beacon();
                iv_update_log_iv_index();
                friend_update_add_to_q_for_all((uint8_t *)p_network_id);

            }
        }
        else if (iv_update && iv_index == g_iv_state.iv_index + 1)
        {
            g_iv_state.iv_update.pending = true;
            (void) iv_update_trigger_if_pending();
             friend_update_add_to_q_for_all((uint8_t *)p_network_id);

        }
    }
    else if (g_iv_state.iv_update.state == MESH_IV_UPDATE_IN_PROGRESS)
    {
        if (!iv_update && iv_index == g_iv_state.iv_index)
        {
            g_iv_state.iv_update.pending = true;
            (void) iv_update_trigger_if_pending();
            friend_update_add_to_q_for_all((uint8_t *)p_network_id);

        }
    }
}



void mesh_iv_sec_bean_rx(const uint8_t * p_network_id, uint32_t iv_index, bool iv_update, bool key_refresh)
{
    beacon_received(p_network_id, iv_index, iv_update, key_refresh);
}



void mesh_iv_update_test_mode_set(bool test_mode_on)
{
    m_test_mode = test_mode_on;
}

uint32_t mesh_test_mode_transition_run(mesh_iv_update_signals_t signal)
{
    if ((signal == MESH_TO_IV_UPDATE_IN_PROGRESS_SIGNAL &&
         g_iv_state.iv_update.state == MESH_IV_UPDATE_IN_PROGRESS) ||
        (signal == MESH_TO_NORMAL_SIGNAL &&
         g_iv_state.iv_update.state == MESH_IV_UPDATE_NORMAL))
    {
        return MESH_FAIL;
    }
    if(signal == MESH_TO_TESTMODE_ON)
    {
         mesh_iv_update_test_mode_set(1);
         return MESH_OK;
    }
    if(signal == MESH_TO_TESTMODE_OFF)
    {
         mesh_iv_update_test_mode_set(0);
         return MESH_OK;
    }
    g_iv_state.iv_update.pending = true;
    (void) iv_update_trigger_if_pending();

    return MESH_OK;
}

/* not use */
void mesh_iv_index_lock(bool lock)
{
    static uint32_t lock_count = 0;
    if (lock)
    {
        g_iv_state.iv_update.locked = true;
        lock_count++;
    }
    else
    {
        BX_ASSERT(lock_count > 0);
        lock_count--;
        if (lock_count == 0)
        {
            g_iv_state.iv_update.locked = false;
            (void) iv_update_trigger_if_pending();
        }
    }
}


uint32_t mesh_beacon_iv_index_get(void)
{
    return g_iv_state.iv_index;
}

void mesh_beacon_iv_index_set(uint32_t val)
{
     g_iv_state.iv_index = val;
}

void mesh_beacon_iv_kr_flag_set(bool flag)
{
     if(flag)
     {
         g_iv_state.iv_update.state = MESH_IV_UPDATE_IN_PROGRESS;
     }
     else
     {
         g_iv_state.iv_update.state = MESH_IV_UPDATE_NORMAL;
     }
}

uint32_t mesh_tx_iv_index_get(void)
{
    if (g_iv_state.iv_update.state == MESH_IV_UPDATE_IN_PROGRESS)
    {
        return g_iv_state.iv_index - 1;
    }
    else
    {
        return g_iv_state.iv_index;
    }
}

uint32_t mesh_rx_iv_index_get(uint8_t ivi)
{
    if ((g_iv_state.iv_index & NETWORK_IVI_MASK) != (ivi & 0x01))
    {
        if (g_iv_state.iv_update.state == MESH_IV_UPDATE_IN_PROGRESS)
        {
            return (g_iv_state.iv_index - 1);
        }
        return (g_iv_state.iv_index - 1);
    }
    else
    {
        return g_iv_state.iv_index;
    }
}

mesh_iv_update_t mesh_iv_update_get(void)
{
    return g_iv_state.iv_update.state;
}

bool mesh_iv_update_is_processing(void)
{
    return g_iv_state.iv_update.state == MESH_IV_UPDATE_IN_PROGRESS ? true:false;
}

uint32_t mesh_seqnum_alloc(uint32_t      num)
{
    uint32_t curSeq = 0;
    if (iv_update_get_seq_num() < g_iv_state.seqnum_max_available)
    {
        /* Check if we've reached the seqnum threshold for a state transition. */
        uint32_t threshold = NETWORK_SEQNUM_IV_UPDATE_START_THRESHOLD;
        if (g_iv_state.iv_update.state == MESH_IV_UPDATE_IN_PROGRESS)
        {
            threshold = NETWORK_SEQNUM_IV_UPDATE_END_THRESHOLD;
        }
        if (iv_update_get_seq_num() >= threshold)
        {
            g_iv_state.iv_update.pending = true;
            iv_update_trigger_if_pending();
        }
        curSeq  = iv_update_get_seq_num();
        node_save_misc_sequence_number();
        alloc_seq_num(num);

    }
    else
    {
        /* should not go to here*/
        g_iv_state.iv_update.pending = true;
        iv_update_trigger_if_pending();
    }
    return curSeq;
}



