/**
 ****************************************************************************************
 *
 * @file   beacon_config.c
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2018-10-08 14:18
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
#include "sdk_mesh_config.h"
//#include "bearer.h"
#include "sdk_mesh_definitions.h"
#include "co_endian.h"
#include "mesh_model.h"
#include "mesh_env.h"
#include "mesh_kr_comm.h"
#include "beacon_config.h"
#include "beacon.h"

/*
 * MACROS
 ****************************************************************************************
 */
#define MAX_BEACON_NETKEY_SEARCH_SIZE   (DM_CFG_NETKEY_MAX_NUM)
/** Number of periods to keep beacon observations of. */
#define MESH_BEACON_OBSERVATION_PERIODS             2
#define MESH_SEC_OBE_TICK                          (10000) //unit: ms/ 10s
#define MAX_BEACON_RX_COUNTER                        (MESH_SEC_OBE_TICK/MESH_SEC_BEACON_TICK)
#define MAX_BEACON_TX_COUNTER                      1
#define SEC_BEACON_CACHE_BUF_LENGTH                  (sizeof(net_beacon_payload_t) + 1)

/*
 * DEFINES
 ****************************************************************************************
 */
typedef struct
{
    /** received number of beacons in each preceding period. */
    uint16_t rx_count[MESH_BEACON_OBSERVATION_PERIODS];
    uint32_t tx_timecounter;
    uint16_t tx_count;
} mesh_beacon_tx_info_t;
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
static uint32_t g_beacon_search_size = MAX_BEACON_NETKEY_SEARCH_SIZE;
static uint8_t g_beacon_send_bit[BIT_BLOCK_COUNT(MAX_BEACON_NETKEY_SEARCH_SIZE)];
static uint8_t g_beacon_immediately_bit[BIT_BLOCK_COUNT(MAX_BEACON_NETKEY_SEARCH_SIZE)];
static mesh_beacon_tx_info_t beacon_tx_info[MAX_BEACON_NETKEY_SEARCH_SIZE];
static uint8_t mesh_sec_beacon_msg_cache[MAX_BEACON_NETKEY_SEARCH_SIZE][SEC_BEACON_CACHE_BUF_LENGTH];

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

uint32_t sec_beacon_get_search_size(void){

    return g_beacon_search_size;
}


void beacon_set_send_flag(uint32_t net_handle)
{
    bit_set(g_beacon_send_bit, net_handle);
}


bool beacon_get_send_flag(uint32_t net_handle)
{
    return bit_get(g_beacon_send_bit, net_handle);
}


void beacon_clear_send_flag(uint32_t net_handle)
{
    bit_clear(g_beacon_send_bit, net_handle);
}


void beacon_set_immediately_flag(uint32_t net_handle)
{
    bit_set(g_beacon_immediately_bit, net_handle);
}
bool beacon_get_immediately_flag(uint32_t net_handle)
{
    return bit_get(g_beacon_immediately_bit, net_handle);
}
void beacon_clear_immediately_flag(uint32_t net_handle)
{
    bit_clear(g_beacon_immediately_bit, net_handle);
}


void beacon_config_init(void)
{
    memset(beacon_tx_info,0,sizeof(mesh_beacon_tx_info_t)*MAX_BEACON_NETKEY_SEARCH_SIZE);
    memset(g_beacon_send_bit,0,sizeof(uint8_t)*BIT_BLOCK_COUNT(MAX_BEACON_NETKEY_SEARCH_SIZE));
    memset(g_beacon_immediately_bit,0,sizeof(uint8_t)*BIT_BLOCK_COUNT(MAX_BEACON_NETKEY_SEARCH_SIZE));
}

void beacon_clear_tx_info(dm_netkey_pos_t index)
{
    beacon_tx_info[index].tx_count = 0;
}

void beacon_inc_tx_info(dm_netkey_pos_t index)
{
    beacon_tx_info[index].tx_count ++;
    if(beacon_tx_info[index].tx_count >= MAX_BEACON_TX_COUNTER)
    {
        uint32_t nethandle = index;
        beacon_clear_immediately_flag(nethandle);
    }
}

void beacon_rx_pkt_rec(dm_netkey_pos_t index)
{
    beacon_tx_info[index].rx_count[0]++;
}

static inline uint32_t mesh_sec_beacon_interval_in_seconds(const mesh_beacon_tx_info_t * p_tx_info)
{

    uint32_t observed_beacons = 0;
    for (uint32_t period = 0; period < MESH_BEACON_OBSERVATION_PERIODS; ++period)
    {
        observed_beacons += p_tx_info->rx_count[period];
    }
    return MESH_SEC_OBE_TICK * (observed_beacons + 1)/MESH_SEC_BEACON_TICK;
}

static void beacon_send(uint32_t index)
{
    beacon_tx_info[index].tx_timecounter = 0;
    beacon_set_send_flag(index);
    beacon_clear_tx_info(index);
    beacon_clear_immediately_flag(index);
}
void beacon_calc_interval(void)
{
    uint32_t index = 0;
    for (; index < sec_beacon_get_search_size(); index++)
    {
        beacon_tx_info[index].tx_timecounter ++;
        uint32_t interval = mesh_sec_beacon_interval_in_seconds(&beacon_tx_info[index]);
        uint32_t period  = beacon_tx_info[index].tx_timecounter % MAX_BEACON_RX_COUNTER;
        if(0 == period)
        {
            for (uint32_t i = 1; i < MESH_BEACON_OBSERVATION_PERIODS; ++i)
            {
                beacon_tx_info[index].rx_count[i] = beacon_tx_info[index].rx_count[i - 1];
            }
            beacon_tx_info[index].rx_count[0] = 0;
        /* Send beacon if the time is right */
            if (beacon_tx_info[index].tx_timecounter >= interval)
            {
                 beacon_send(index);
            }
        }
        else if(beacon_get_immediately_flag(index))
        {
            beacon_send(index);
        }
     }
}


void mesh_sec_beacon_cache_cpy(uint8_t *data,uint8_t length)
{
    static uint8_t mesh_sec_beacon_msg_cache_wr_index = 0;
    mesh_sec_beacon_msg_cache_wr_index ++;
    
    if(mesh_sec_beacon_msg_cache_wr_index >= MAX_BEACON_NETKEY_SEARCH_SIZE) mesh_sec_beacon_msg_cache_wr_index = 0;
    memcpy(&mesh_sec_beacon_msg_cache[mesh_sec_beacon_msg_cache_wr_index][0] , data + 1 , SEC_BEACON_CACHE_BUF_LENGTH);
}


bool mesh_sec_beacon_is_exsit_cache(uint8_t *data,uint8_t length)
{
    uint8_t i = 0;
    for(i=0;i<MAX_BEACON_NETKEY_SEARCH_SIZE;i++)
    {
        if(memcmp(data + 1 , mesh_sec_beacon_msg_cache[i] , SEC_BEACON_CACHE_BUF_LENGTH ) == 0)
        {
            return  true;
        }
    }

    return false;
}


