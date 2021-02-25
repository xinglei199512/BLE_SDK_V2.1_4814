#include "osapp_config.h"
#include <stdint.h>
#include "sdk_mesh_definitions.h"
#include "co_endian.h"
#include "beacon.h"
#include "bx_ring_queue.h"
#include "security.h"
#include "mesh_model.h"
#include "mesh_env.h"
#include "config_server.h"
#include "provision_api.h"
#include "mesh_iv_operation_ex.h"
#include "mesh_kr_comm.h"
#include "mesh_kr_server.h"
#include "mesh_kr_client.h"
#include "beacon_config.h"
//#include "provision_base.h"
#include "aes_ccm_cmac.h"
#include "adv_bearer_tx.h"
#include "mesh_queued_msg.h"
#include "timer_wrapper.h"
#include "beacon_mesh_msg_cache.h"
#include "mesh_reset_database.h"
#include "mesh_gatt_proxy.h"
#include "unprov_device_intf.h"
#include "mesh_core_api.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"
#include "foundation_msg_handler.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#define BEACON_DATA_OFFSET                           BEACON_TYPE_LENGTH
#define DEVICE_UUID_OFFSET                           0
#define OOB_INFO_OFFSET                              (DEVICE_UUID_OFFSET + DEVICE_UUID_LENGTH)
#define URI_HASH_OFFSET                              (OOB_INFO_OFFSET + OOB_INFO_LENGTH)
#define MESH_SEC_BEACON_FLAG_TICK                    (10*60)
#define SEC_BEACON_RX_AUTHENTICATION_QUEUE_LENGTH    3

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


typedef struct
{
    mesh_beacon_t *beacon_para;
    uint8_t beaconing;
} beacon_mng_t;


typedef struct {
    net_key_t * ptr;
    dm_netkey_pos_t cur_net_handle;
    uint8_t aes_cmc[MESH_KEY_LENGTH];
}adv_sec_beacon_env_t;

typedef struct
{
    uint8_t aes_cmc[MESH_KEY_LENGTH];
    net_beacon_t * pdu;
}sec_beacon_rx_env_t;

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

void unprovisioned_dev_beacon_send(void);

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static adv_sec_beacon_env_t adv_sec_beacon_env;
static beacon_rx_callback_t beacon_rx_callback;
static beacon_mng_t beacon_mng;
static net_beacon_payload_t payload;
static uint8_t adv_sec_beacon_queue_busy = 0;
static uint32_t flag_timecounter;
static mesh_timer_t meshSecBeaconTimer;
static sec_beacon_rx_env_t  sec_beacon_rx_env;
static uint8_t sec_beacon_counter = 0;
static uint8_t upprov_beacon_counter = 0;
uint8_t send_secoure_beacon = 1;
static uint8_t send_unprov_beacon = 1;


DEF_RING_QUEUE(sec_beacon_rx_authentication_q, SEC_BEACON_RX_AUTHENTICATION_QUEUE_LENGTH, net_beacon_t);


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
static void iv_update_calc_auth(void);
static void mesh_sec_beacon_auth(void);



static void mesh_dec_beacon_send_pkt(void)
{
    net_key_t* ptr = NULL;
    dm_netkey_pos_t index = adv_sec_beacon_env.cur_net_handle + 1;
    if(adv_sec_beacon_queue_busy || !get_is_node_can_tx_pkt())
    {
        return;
    }
    if(index >= sec_beacon_get_search_size())
    {
        index = 0;
    }
    for (; ; )
    {
        if(beacon_get_send_flag(index))
        {
            ptr = mesh_sec_beacon_get_netlist(index);
            if(NULL != ptr)
            {
            
                adv_sec_beacon_queue_busy = 1;
                adv_sec_beacon_env.cur_net_handle = index;
                adv_sec_beacon_env.ptr = ptr;
                mesh_run(iv_update_calc_auth, portMAX_DELAY, false);
                beacon_clear_send_flag(index);
                return;
            }
            beacon_clear_send_flag(index);
        }
        if(index == adv_sec_beacon_env.cur_net_handle)
        {
            break;
        }
        index ++;
        if(index >= sec_beacon_get_search_size())
        {
            index = 0;
        }
    }
}

static void mesh_unprov_tx_timer_expire_handler(beacon_adv_tx_t *ptr)
{
    beacon_pdu_adv_tx_start(ptr);
}

void mesh_unprov_adv_tx_timer_callback(mesh_timer_t xTimer)
{
    mesh_adv_tx_t *adv = pvTimerGetTimerID(xTimer);
    beacon_adv_tx_t *ptr = CONTAINER_OF(adv, beacon_adv_tx_t, adv);
    //LOG(3,"mesh_unprov_adv_tx_timer_callback\n");
    mesh_queued_msg_send((void (*)(void *))mesh_unprov_tx_timer_expire_handler, ptr);
}


void mesh_unprov_beacon_send_pkt(void)
{
    if(beacon_mng.beaconing != 1)
    {
         return;
    }
    if(provision_is_database_pending())
    {
        return;
    }
    beacon_adv_tx_t *tx_buf = beacon_adv_tx_buf_alloc(UNPROV_DEV_BEACON_ADV_PKT);
    if(NULL == tx_buf)
    {
        return;
    }
    bearer_adv_tx_timer_create("unprov_adv", &tx_buf->adv, false, mesh_unprov_adv_tx_timer_callback);
    mesh_core_params_t unprov_beacon_transmit;
    mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_UNPROV_BEACON_TRANSMIT   , &unprov_beacon_transmit);
    tx_buf->param.count = 0;
    tx_buf->param.repeats = unprov_beacon_transmit.unprov_beacon_transmit.count;
    tx_buf->param.interval_steps = (unprov_beacon_transmit.unprov_beacon_transmit.interval_steps + 1) * 10;
    uint8_t *buf = (uint8_t *)tx_buf->adv.adv_data;
    buf[1] = MESH_BEACON_AD_TYPE;
    buf[2] = Unprovisioned_Device_Beacon;
    uint8_t *beacon_data_ptr = buf + BEACON_TYPE_LENGTH + DATA_OFFSET_IN_BEARER;
    uint8_t uri_hash_len = 0;
    uint16_t OOB_info_swap = co_bswap16(beacon_mng.beacon_para->oob_info);
    uint32_t URI_hash_swap = co_bswap32(beacon_mng.beacon_para->uri_hash);
    memcpy(beacon_data_ptr + DEVICE_UUID_OFFSET,beacon_mng.beacon_para->dev_uuid,DEVICE_UUID_LENGTH);
    memcpy(beacon_data_ptr + OOB_INFO_OFFSET,&OOB_info_swap,OOB_INFO_LENGTH);
    if(beacon_mng.beacon_para->uri_hash)
    {
        memcpy(beacon_data_ptr + URI_HASH_OFFSET,&URI_hash_swap,URI_HASH_LENGTH);
        uri_hash_len = URI_HASH_LENGTH;
    }
    buf[0] = BEACON_TYPE_LENGTH + DEVICE_UUID_LENGTH + OOB_INFO_LENGTH + uri_hash_len + 1;
    tx_buf->adv.data_length = BEACON_TYPE_LENGTH + DEVICE_UUID_LENGTH + OOB_INFO_LENGTH + uri_hash_len + DATA_OFFSET_IN_BEARER ;
    beacon_pdu_adv_tx_start(tx_buf);

}

void stop_current_beacon(void) 
{
    beacon_mng.beaconing = 0;
}

void unprovisioned_dev_beacon_restart(void)
{
    LOG(3,"unprovisioned_dev_beacon_restart\n");
    //adv beacon
    if(provision_system_method_get()&PROVISION_BY_ADV)
    {
        if(!get_is_provisioned())// is not provisioned
        {
            beacon_mng.beaconing = 1;
            mesh_unprov_beacon_send_pkt();
        }
    }
}


void unprovisioned_dev_beacon_start(mesh_beacon_t *beacon_para)
{
    LOG(3,"==Beacon:tx==\n");
    beacon_mng.beacon_para = beacon_para;
    beacon_mng.beaconing = 1;
    mesh_unprov_beacon_send_pkt();
}

#if 0
uint8_t set_iv_update_flag(uint8_t data, uint32_t value)
{
    mesh_test_mode_transition_run((mesh_iv_update_signals_t)data);
    //mesh_beacon_iv_index_set(value);
}
#endif

static void iv_update_fill_beacon_payload(net_beacon_payload_t * pPayload, uint8_t *pNetwork_id)
{
    dm_netkey_handle_t p_netkey = mesh_sec_beacon_get_netlist_by_netid(pNetwork_id);
    if(NULL == p_netkey)
    {
        return;
    }
    memset(pPayload, 0, sizeof(net_beacon_payload_t));
    pPayload->flags.key_refresh = mesh_get_key_refresh(p_netkey);

    pPayload->iv_index = co_htonl(mesh_beacon_iv_index_get());
    pPayload->flags.iv_update = mesh_iv_update_is_processing()?1:0;

    memcpy(pPayload->network_id, pNetwork_id, NETWORK_ID_LENGTH);
}

static void iv_update_fill_beacon_aes_cmac(uint8_t * cmac, uint8_t *pCmac)
{
    memset(cmac, 0, NET_BEACON_CMAC_SIZE);
    memcpy(cmac, pCmac, NET_BEACON_CMAC_SIZE);
}

void iv_update_beacon_pkt_done()
{
     
     adv_sec_beacon_queue_busy = 0;
     mesh_dec_beacon_send_pkt();
     iv_update_log_event(IV_UPDATE_SEND_PKT);
     //LOG(3,"beacon_pkt_done!\n");
}

void sec_beacon_send_notify(dm_netkey_pos_t index)
{
     beacon_inc_tx_info(index);
}

void iv_update_send_pkt(adv_sec_beacon_env_t *pEnv)
{
    beacon_adv_tx_t *tx_buf = beacon_adv_tx_buf_alloc(SECURE_NETWORK_BEACON_ADV_PKT);
    BX_ASSERT(tx_buf);
    uint8_t *buf = (uint8_t *)tx_buf->adv.adv_data;
    net_key_t* ptr = NULL;
    uint8_t keyindex;
    dm_netkey_pos_t index = pEnv->cur_net_handle ;
    tx_buf->adv.data_length = BEACON_SEC_PKT_LENGTH + DATA_OFFSET_IN_BEARER;
    buf[0] = BEACON_SEC_PKT_LENGTH + 1;
    buf[1] = MESH_BEACON_AD_TYPE;
    buf[0+DATA_OFFSET_IN_BEARER] = Secure_Network_Beacon;
    net_beacon_t *beacon_data_ptr =(net_beacon_t *) (buf + BEACON_TYPE_LENGTH + DATA_OFFSET_IN_BEARER);
    ptr = mesh_sec_beacon_get_netlist(index);
    if(NULL == ptr)
    {
        return;
    }
    keyindex = mesh_kr_Get_tx_key_index(ptr);
    iv_update_fill_beacon_payload(&beacon_data_ptr->payload,ptr->key[keyindex].network_id);
    iv_update_fill_beacon_aes_cmac(beacon_data_ptr->cmac,pEnv->aes_cmc);
    beacon_pdu_adv_tx_start(tx_buf);
    //LOG(3,"==sec beacon:tx==\n");
//    friend_update_add_to_q_for_all(ptr->key[keyindex].network_id);
    iv_update_beacon_pkt_done();
    sec_beacon_send_notify(index);
}




static void iv_update_aes_cmac_done(ccm_cmac_buf_t * ccm, void * ptr,uint8_t status)
{

    //iv_update_beacon_stub();
    iv_update_send_pkt(&adv_sec_beacon_env);
    //iv_update_beacon_stub_iv();

}

static void beacon_aes_cmac_calc(uint8_t const* k,uint8_t const*m,uint8_t length,uint8_t *result,void (*cb)(ccm_cmac_buf_t *,void *,uint8_t))
{
    ccm_cmac_buf_t ccm_encrypt=
    {
        .param.cmac = {
            .k = k,
            .m = m,
            .length = length,
            .rslt = result,
        },
        .op_type = CMAC_CALC,
    };
    ccm_cmac_start(&ccm_encrypt, cb);
}


static void iv_update_calc_auth()
{
    net_key_t *p_netkey = adv_sec_beacon_env.ptr;
    if(NULL == p_netkey)
    {
        adv_sec_beacon_queue_busy = 0;
        return;
    }
    uint8_t keyindex = mesh_kr_Get_tx_key_index(p_netkey);

    iv_update_fill_beacon_payload(&payload,p_netkey->key[keyindex].network_id);
    memset(adv_sec_beacon_env.aes_cmc, 0, MESH_KEY_LENGTH);
    beacon_aes_cmac_calc(p_netkey->key[keyindex].beacon_key,(const uint8_t *)&payload,sizeof(net_beacon_payload_t), adv_sec_beacon_env.aes_cmc, iv_update_aes_cmac_done);
}


void unprovisioned_dev_beacon_rx(uint8_t *data,uint8_t length)
{
    uint8_t *dev_uuid = data + DEVICE_UUID_OFFSET;
    uint16_t oob_info;
    uint32_t uri_hash;
    mesh_beacon_t beacon_para;

    if(beacon_rx_callback)
    {
        //1. copy uuid
        memcpy(beacon_para.dev_uuid , dev_uuid , DEVICE_UUID_LENGTH);
        //2. copy oob info
        memcpy(&oob_info,data + OOB_INFO_OFFSET,OOB_INFO_LENGTH);
        oob_info = co_bswap16( oob_info);
        beacon_para.oob_info = oob_info;
        //3. copy uri hash and callback
        if(length==DEVICE_UUID_LENGTH + OOB_INFO_LENGTH + URI_HASH_LENGTH)
        {
            memcpy(&uri_hash,data + URI_HASH_OFFSET,URI_HASH_LENGTH);
            uri_hash = co_bswap32(uri_hash);
            beacon_para.uri_hash = uri_hash;
        }else if(length == DEVICE_UUID_LENGTH + OOB_INFO_LENGTH)
        {
            beacon_para.uri_hash = 0;
        }else
        {
            LOG(LOG_LVL_WARN,"Invalid unprovisioned_dev_beacon packet length\n");
            return;
        }
        if(!beacon_mesh_msg_cache_search(&beacon_para))
        {
            if(beacon_mesh_msg_cache_full())
            {
                beacon_mesh_msg_cache_remove_oldest();
                LOG(LOG_LVL_WARN,"beacon cache full\n");
            }
            beacon_mesh_msg_cache_add(&beacon_para);
            beacon_rx_callback(&beacon_para);
        }
        else
        {
            LOG(LOG_LVL_WARN,"beacon cache found\n");
        }
    }
}


static void mesh_sec_beacon_tx_counter_inc(dm_netkey_pos_t net_index)
{
     if(net_index < sec_beacon_get_search_size())
     {
//          beacon_clear_send_flag(net_index);
          beacon_rx_pkt_rec(net_index);
     }
}


static void mesh_sec_beacon_rx(const uint8_t * p_beacon_data, uint8_t data_length)
{
   net_beacon_t* p_beacon = (net_beacon_t*) p_beacon_data;
   uint32_t iv_index = co_ntohl(p_beacon->payload.iv_index);
   bool key_refresh =  p_beacon->payload.flags.key_refresh;
   net_beacon_kr_handle(p_beacon->payload.network_id,key_refresh);
   mesh_iv_sec_bean_rx(p_beacon->payload.network_id, iv_index, p_beacon->payload.flags.iv_update,p_beacon->payload.flags.key_refresh);
}

void mesh_sec_beacon_opr_queue(void)
{
    bx_dequeue(&sec_beacon_rx_authentication_q);
    sec_beacon_rx_env.pdu = bx_ring_queue_glance(&sec_beacon_rx_authentication_q, 0);
    if(NULL != sec_beacon_rx_env.pdu)
    {
        mesh_run(mesh_sec_beacon_auth,portMAX_DELAY,false);
    }
}


static void mesh_sec_beacon_aes_cmac_done_new(ccm_cmac_buf_t * ccm ,void * ptr,uint8_t status)
{
    net_beacon_t *p_beacon = sec_beacon_rx_env.pdu; 
    if(NULL == sec_beacon_rx_env.pdu)
    {
        mesh_sec_beacon_opr_queue();
        return;
    }
    if(!memcmp(p_beacon->cmac, sec_beacon_rx_env.aes_cmc, NET_BEACON_CMAC_SIZE ))
    {
         mesh_sec_beacon_rx((uint8_t*)p_beacon,sizeof(net_beacon_t));
    }
    mesh_sec_beacon_opr_queue();
}

static void mesh_sec_beacon_aes_cmac_done_cur(ccm_cmac_buf_t * ccm ,void * ptr,uint8_t status)
{
    net_beacon_t *p_beacon = sec_beacon_rx_env.pdu; 
    if(NULL == sec_beacon_rx_env.pdu)
    {
        mesh_sec_beacon_opr_queue();
        return;
    }
    if(!memcmp(p_beacon->cmac, sec_beacon_rx_env.aes_cmc, NET_BEACON_CMAC_SIZE ))
    {
         mesh_sec_beacon_rx((uint8_t*)p_beacon,sizeof(net_beacon_t));
         mesh_sec_beacon_opr_queue();
    }
    else
    {
        net_beacon_t *p_beacon = sec_beacon_rx_env.pdu;
        if(NULL == p_beacon)
        {
            mesh_sec_beacon_opr_queue();
            return;
        }
        dm_netkey_handle_t p_netkey = mesh_sec_beacon_get_netlist_by_netid(p_beacon->payload.network_id);
        uint8_t keyindex = mesh_kr_Get_beacon_key_newindex(p_netkey);
        beacon_aes_cmac_calc(p_netkey->key[keyindex].beacon_key,(const uint8_t *) &p_beacon->payload, sizeof(net_beacon_payload_t), sec_beacon_rx_env.aes_cmc,  mesh_sec_beacon_aes_cmac_done_new);
    }
}




static void mesh_sec_beacon_auth(void)
{
    net_beacon_t *p_beacon = sec_beacon_rx_env.pdu;
    if(NULL == p_beacon)
    {
        mesh_sec_beacon_opr_queue();
        return;
    }
    net_key_t *p_netkey = mesh_sec_beacon_get_netlist_by_netid(p_beacon->payload.network_id);
    if(NULL == p_netkey)
    {
        mesh_sec_beacon_opr_queue();
        return;
    }
    uint8_t keyindex = mesh_kr_Get_beacon_key_index(p_netkey);
    beacon_aes_cmac_calc(p_netkey->key[keyindex].beacon_key, (const uint8_t *)&p_beacon->payload,sizeof(net_beacon_payload_t), sec_beacon_rx_env.aes_cmc,  mesh_sec_beacon_aes_cmac_done_cur);
}


static void mesh_sec_beacon_queue(uint8_t * p_beacon_data, uint8_t data_length)
{
    bool bool_queue_full = bx_ring_queue_full(&sec_beacon_rx_authentication_q);
    net_beacon_t *p_beacon = (net_beacon_t *)p_beacon_data;
    net_key_t *p_netkey = mesh_sec_beacon_get_netlist_by_netid(p_beacon->payload.network_id);
    if(NULL == p_netkey)
    {
        return;
    }
    mesh_sec_beacon_rx_pkt_inc(p_beacon->payload.network_id);
    if(mesh_sec_beacon_is_exsit_cache(p_beacon_data, data_length))
    {
        return;
    }
    mesh_sec_beacon_cache_cpy(p_beacon_data, data_length);
    if(bool_queue_full)
    {
        LOG(3,"sec_beacon_rx_q full!\n");
        BX_ASSERT(0);
    }
    bx_enqueue(&sec_beacon_rx_authentication_q,p_beacon_data);
    if(sec_beacon_rx_env.pdu == NULL)
    {
       sec_beacon_rx_env.pdu = bx_ring_queue_glance(&sec_beacon_rx_authentication_q, 0);//make sure change state quickly.
       mesh_run(mesh_sec_beacon_auth,portMAX_DELAY,false);
    }

}

void mesh_sec_beacon_rx_pkt_inc(uint8_t *network_id)
{
    dm_netkey_pos_t index = 0;
    err_t err = mesh_sec_beacon_get_net_index(network_id,&index);
    if(err != MESH_CORE_SUCCESS)
    {
        return;
    }
    mesh_sec_beacon_tx_counter_inc(index);
}

bool is_sec_beacon(uint8_t *data,uint8_t length)
{
    uint8_t beacon_type = data[0];
    if(length == MESH_SEC_BEACON_LENGTH && beacon_type == Secure_Network_Beacon)
    {
        return true;
    }
    return false;
}

void iv_update_beacon_send_immediately(void)
{
    uint32_t index = 0;
    for (; index < sec_beacon_get_search_size(); index++)
    {
        beacon_clear_tx_info(index);
        beacon_set_send_flag(index);
    }
}

void beacon_send_by_net(uint8_t *network_id)
{
    dm_netkey_pos_t index = 0;
    uint32_t handle = 0;
    err_t err = mesh_sec_beacon_get_net_index(network_id,&index);
    if(err != MESH_CORE_SUCCESS)
    {
        return;
    }
    handle = index;
    beacon_set_immediately_flag(handle);
    beacon_clear_tx_info(index);
}

void beacon_send_to_all(void)
{
    uint32_t index = 0;
    for (; index < sec_beacon_get_search_size(); index++)
    {
        dm_netkey_pos_t pos = index;
        beacon_clear_tx_info(pos);
        beacon_set_immediately_flag(index);
    }
}

void beacon_rx(uint8_t *data,uint8_t length)
{
    uint8_t beacon_type = data[0];
    //LOG(3,"beacon_rx!\n");
    mesh_core_params_t low_power;
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_LOW_POWER   , &low_power);
    if(MESH_FEATURE_ENABLED ==low_power.low_power)
    {
        return;
    }
    if(beacon_type == Unprovisioned_Device_Beacon)
    {
        //LOG(3,"Unprovisioned_Device_Beacon_rx!\n");
        unprovisioned_dev_beacon_rx(data + BEACON_DATA_OFFSET, length -BEACON_TYPE_LENGTH);
    }else if(beacon_type == Secure_Network_Beacon)
    {
        if(!get_is_node_can_rx_pkt())
        {
           return;
        }
        mesh_sec_beacon_queue(data + BEACON_DATA_OFFSET, length -BEACON_TYPE_LENGTH);
    }
}


static void mesh_sec_beacon_flag_counter_upt()
{
    extern bool provision_is_database_pending(void);
    if(provision_is_database_pending())
    {
        return;
    }

    if(adv_sec_beacon_queue_busy == 1)
    {
         flag_timecounter ++;
    }
    else
    {
        flag_timecounter = 0;
    }
}

static bool mesh_sec_beacon_flag_timeout(void)
{
   if(flag_timecounter > MESH_SEC_BEACON_FLAG_TICK)
   {
       return true;
   }
   return false;
}

static void mesh_sec_beacon_flag_time_handle(void)
{
     mesh_sec_beacon_flag_counter_upt();
     if(mesh_sec_beacon_flag_timeout())
     {
          adv_sec_beacon_queue_busy = 0;
     }
}

void unprovsion_beacon_stop(void)
{
    send_unprov_beacon = 0;
}

void unprovsion_beacon_start(void)
{
    send_unprov_beacon = 1;
}

void beacon_stop(void)
{
    if(mesh_timer_active(meshSecBeaconTimer)) {
        mesh_timer_stop(meshSecBeaconTimer);
    }
}

void beacon_start(void)
{
    if(!mesh_timer_active(meshSecBeaconTimer)) {
        mesh_timer_start(meshSecBeaconTimer);
    }
}

static void meshSecBeaconTimeoutCallback(mesh_timer_t xTimer)
{
     //LOG(3, "meshSecBeaconTimeoutCallback\n");
     mesh_core_params_t unprov_beacon_transmit;
     mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_UNPROV_BEACON_TRANSMIT   , &unprov_beacon_transmit);
     mesh_core_params_t low_power;
     mesh_core_params_get(MESH_CORE_PARAM_FEATURE_LOW_POWER   , &low_power);

     sec_beacon_counter++;
     upprov_beacon_counter++;
     if(send_unprov_beacon && upprov_beacon_counter >= unprov_beacon_transmit.unprov_beacon_transmit.period/MESH_BEACON_TICK)
     {
         mesh_run(mesh_unprov_beacon_send_pkt, portMAX_DELAY, false);
         upprov_beacon_counter = 0;
     }
     if(MESH_FEATURE_ENABLED !=low_power.low_power && send_secoure_beacon && sec_beacon_counter >= MESH_SEC_BEACON_TICK/MESH_BEACON_TICK)
     {
         beacon_calc_interval();
         mesh_sec_beacon_flag_time_handle();
         mesh_run(mesh_dec_beacon_send_pkt, portMAX_DELAY, false);
         sec_beacon_counter = 0;
     }
}

void mesh_beacon_timer_init(void)
{
     flag_timecounter = 0;
     uint8_t period = rand() % ( 100);
     meshSecBeaconTimer = mesh_timer_create("meshSecBeaconTimer",pdMS_TO_TICKS(MESH_BEACON_TICK+ period),pdTRUE,(void *)0,meshSecBeaconTimeoutCallback);
#if 0
     if(meshSecBeaconTimer != NULL)
        xTimerStart(meshSecBeaconTimer,0);
#endif
}

void mesh_beacon_init(void)
{
    mesh_beacon_timer_init();
    beacon_config_init();
    memset(&sec_beacon_rx_env, 0, sizeof(sec_beacon_rx_env_t));
}

void regisiter_beacon_rx_callback(beacon_rx_callback_t cb)
{
    beacon_rx_callback = cb;
}

void beacon_pdu_adv_tx_start(beacon_adv_tx_t *ptr)
{
    bearer_adv_tx_msg_send(&ptr->adv , false);
}

void config_beacon_status_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    LOG(3, "config_beacon_stauts_rx\n");
}

