


#include "osapp_config.h"


#include "mesh_env.h"
#include "beacon.h"
#include "co_endian.h"
#include "mesh_iv_operation.h"
#include "mesh_iv_operation_ex.h"
#include "mesh_model.h"
#include "mesh_kr_comm.h"
#include "mesh_kr_client.h"
#include "device_keys_dm.h"
#include "network_keys_dm.h"
#include "app_keys_dm.h"
#include "mesh_node_base.h"
#include "mesh_core_api.h"
#include "access_tx_process.h"
#include "foundation_msg_handler.h"
#include "foundation_common.h"

#ifdef MESH_UART_DEBUG_TEST_CMD
#include "uart_debug_test.h"
#endif /*MESH_UART_DEBUG_TEST_CMD*/

typedef struct
{
    uint32_t netkey_update_timer; 
    mesh_kr_client_update_t  status; 
    dm_devkey_handle_t dev_handler; 
    dm_netkey_handle_t net_handler; 
} netkey_update_state_t;

netkey_update_state_t netkey_update_state[DM_CFG_NETKEY_MAX_NUM]; 
mesh_timer_t meshKrClientTimer;
static void meshKrClientTimeoutCallback(mesh_timer_t xTimer);
static void mesh_kr_client_distribute_netkey_callback(void * access_pdu, uint8_t param);
void mesh_kr_client_key_refresh_phase_set_callback(void * access_pdu, uint8_t param);
static void mesh_kr_client_set_kr_trans_state(dm_netkey_handle_t netkey_handle,uint8_t flag);
uint16_t blacklist_addr[MESH_KR_LIST_MAX_SIZE];
static uint8_t kr_start_flag[BIT_BLOCK_COUNT(DM_CFG_NETKEY_MAX_NUM)];


static void kr_flag_init(void)
{
    memset(kr_start_flag,0,sizeof(kr_start_flag));
}

static void addr_blacklist_init(void)
{
    memset(blacklist_addr,0,sizeof(blacklist_addr));
}

static void set_kr_flag(uint32_t net_handle)
{
    bit_set(kr_start_flag, net_handle);
}


static bool get_kr_flag(uint32_t net_handle)
{
    return bit_get(kr_start_flag, net_handle);
}


static void clear_kr_flag(uint32_t net_handle)
{
    bit_clear(kr_start_flag, net_handle);
}


static model_base_t* mesh_kr_get_client_model_base(void)
{
    mesh_core_params_t param;
    mesh_core_params_get(MESH_CORE_PARAM_MESH_ROLES , &param);
    if(param.role == MESH_ROLE_CONFIG_CLIENT)//client
    {
        return &(get_config_client()->model.base);
    }
    return NULL;
}


static void mesh_kr_client_timer_inc(dm_netkey_handle_t netkey_handle)
{
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return;
    }
    netkey_update_state[pos].netkey_update_timer ++;
}

static void mesh_kr_client_timer_clear(dm_netkey_handle_t netkey_handle)
{
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return;
    }
    netkey_update_state[pos].netkey_update_timer = 0;
}

static uint32_t mesh_kr_client_get_timercout(dm_netkey_handle_t netkey_handle)
{
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return 0;
    }
    return netkey_update_state[pos].netkey_update_timer;
}


static void mesh_kr_set_client_update_status(dm_netkey_handle_t netkey_handle, mesh_kr_client_update_t status)
{
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return;
    }
    netkey_update_state[pos].status = status;
    dm_devkey_ack_reset(netkey_handle);
    LOG(LOG_LVL_INFO,"mesh_kc_status %d\n",status);
}


static void mesh_kr_set_client_key_refresh_phase(dm_netkey_handle_t netkey_handle, mesh_key_refresh_transition_t tranaction)
{
   
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return;
    }
    if(tranaction == MESH_KEY_TRANSITION_TO_PHASE_2)
    {
        netkey_update_state[pos].status = MESH_KR_UPDATE_WAIT_PHASE2_ACK;
    }
    else
    {
        netkey_update_state[pos].status = MESH_KR_UPDATE_WAIT_PHASE3_ACK;
    }
    LOG(LOG_LVL_INFO,"mesh_kr_set_client_key_refresh_phase %d\n",tranaction);
}


static mesh_kr_client_update_t mesh_kr_get_client_update_status(dm_netkey_handle_t netkey_handle)
{
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return MESH_KR_UPDATE_INVALID_STATUS;
    }
    return netkey_update_state[pos].status;
}



void mesh_kr_client_devkey_status_reset(dm_netkey_handle_t netkey_handle)
{
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return;
    }
    netkey_update_state[pos].netkey_update_timer = 0;
    netkey_update_state[pos].status = MESH_KR_UPDATE_NORMAL;
    netkey_update_state[pos].dev_handler = MESH_INVALID_DEVKEY_HANDLE;
    netkey_update_state[pos].net_handler = MESH_INVALID_NETKEY_HANDLE;
}

static void mesh_kr_client_devkey_next_init(dm_netkey_handle_t netkey_handle)
{
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return;
    }
    netkey_update_state[pos].dev_handler = INVALID_DEV_HANDLER;
}


bool  mesh_kr_client_is_in_blackList(dev_key_t * dev_key)
{
    uint8_t index =0;
    if(NULL == dev_key)
    {
        return false;
    }
    for(; index < MESH_KR_LIST_MAX_SIZE; index++)
    {
        if(blacklist_addr[index] == dev_key->addr)
        {
            return true;
        }
    }
    return false;
}

mesh_global_idx_t mesh_kr_client_get_netkey_index(access_pdu_tx_t *pdu)
{

    config_msg_netkey_add_update_t *kr_netkey_set =(config_msg_netkey_add_update_t *) (access_model_tx_payload_ptr(pdu,TWO_OCTETS_OPCODE_GEN(FOUNDATION_MODELS_TWO_OCTETS_OPCODE_OFFSET, Config_Key_Refresh_Phase_Set)));
    return kr_netkey_set->netkey_index;
}


dm_devkey_handle_t mesh_kr_client_getnext_devkey(dm_netkey_handle_t netkey_handle)
{
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return MESH_INVALID_DEVKEY_HANDLE;
    }
    if(MESH_CORE_SUCCESS != dm_devkey_getnext_dev(netkey_handle, &(netkey_update_state[pos].dev_handler)))
    {
        return MESH_INVALID_DEVKEY_HANDLE;
    }
    return netkey_update_state[pos].dev_handler;
    
}


static void mesh_kr_client_distribute_netkey(dm_netkey_handle_t netkey_handle, dm_devkey_handle_t  devkey_handle, model_base_t* base)
{
    config_msg_netkey_add_update_t netkey_update;
    uint16_t addr = 0;
    net_key_box_t *keyinfo =  NULL;
    mesh_global_idx_t index = 0;
    if(NULL == netkey_handle || MESH_INVALID_DEVKEY_HANDLE == devkey_handle)
    {
        return ;
    }
    LOG(LOG_LVL_INFO,"mesh_kr_client_distribute_netkey %d\n",netkey_handle);
    if(MESH_CORE_SUCCESS != dm_netkey_handle_to_netkey_index(netkey_handle,&index))
    {
        return;
    }
    if(MESH_CORE_SUCCESS != dm_devkey_addr_get(devkey_handle,&addr))
    {
        return;
    }
    if(MESH_CORE_SUCCESS != dm_netkey_get_update_info(netkey_handle, &keyinfo))
    {
        return;
    }
    netkey_update.netkey_index = index;
    memcpy(netkey_update.netkey,keyinfo->netkey,MESH_KEY_LENGTH);
    fd_model_two_octets_status_tx(netkey_handle->global_idx, base->elmt->uni_addr, NULL, mesh_kr_client_distribute_netkey_callback,
        (uint8_t *)&netkey_update, sizeof(config_msg_netkey_add_update_t), Config_NetKey_Update);

}


static void mesh_kr_client_distribute_netkey_callback(void * access_pdu, uint8_t param)
{
    model_base_t *base = mesh_kr_get_client_model_base();
    dm_devkey_handle_t dev_handle = NULL;
    mesh_global_idx_t netkey_index = 0;
    dm_netkey_handle_t netkey_handle = 0;
    if(NULL == base)
    {
        return ;
    }
    netkey_index = mesh_kr_client_get_netkey_index(access_pdu);
    if(MESH_CORE_SUCCESS != dm_netkey_index_to_netkey_handle(netkey_index,&netkey_handle))
    {
        return ;
    }
    dev_handle =mesh_kr_client_getnext_devkey(netkey_handle);
    if(MESH_INVALID_DEVKEY_HANDLE == dev_handle)
    {
        mesh_kr_set_client_update_status(netkey_handle, MESH_KR_UPDATE_WAIT_ACK);
        return ;
    }
    mesh_kr_client_distribute_netkey(netkey_handle,dev_handle,base);
    return ;
}



static void mesh_kr_client_key_refresh_phase_set(dm_netkey_handle_t netkey_handle, dm_devkey_handle_t  dev_handle,model_base_t* base, mesh_key_refresh_transition_t transaction)
{
    config_msg_key_refresh_phase_set_t refresh_phase;
    uint16_t addr = 0;
    mesh_global_idx_t index;
    if(NULL == netkey_handle)
    {
        return ;
    }
    
    if(MESH_CORE_SUCCESS != dm_netkey_handle_to_netkey_index(netkey_handle,&index))
    {
        return;
    }
    if(MESH_CORE_SUCCESS != dm_devkey_addr_get(dev_handle,&addr))
    {
        return;
    }
    LOG(LOG_LVL_INFO,"mesh_kr_client_key_refresh_phase_set %d:%d\n",index,transaction);
    refresh_phase.netkey_index = index;
    refresh_phase.transition = transaction;

    fd_model_two_octets_status_tx(netkey_handle->global_idx,base->elmt->uni_addr,NULL,mesh_kr_client_key_refresh_phase_set_callback, 
        (uint8_t *)&refresh_phase,sizeof(config_msg_key_refresh_phase_set_t), Config_Key_Refresh_Phase_Set);


}


mesh_key_refresh_transition_t mesh_kr_client_key_refresh_phase_get_phase(access_pdu_tx_t *pdu)
{
    config_msg_key_refresh_phase_set_t *kr_netkey_set =(config_msg_key_refresh_phase_set_t *) (access_model_tx_payload_ptr(pdu,TWO_OCTETS_OPCODE_GEN(FOUNDATION_MODELS_TWO_OCTETS_OPCODE_OFFSET, Config_Key_Refresh_Phase_Set)));
    return kr_netkey_set->transition;
}

mesh_global_idx_t mesh_kr_client_key_refresh_phase_get_netkey_index(access_pdu_tx_t *pdu)
{

    config_msg_key_refresh_phase_set_t *kr_netkey_set =(config_msg_key_refresh_phase_set_t *) (access_model_tx_payload_ptr(pdu,TWO_OCTETS_OPCODE_GEN(FOUNDATION_MODELS_TWO_OCTETS_OPCODE_OFFSET, Config_Key_Refresh_Phase_Set)));
    return kr_netkey_set->netkey_index;
}

void mesh_kr_client_key_refresh_phase_set_callback(void * access_pdu, uint8_t param)
{
    access_pdu_tx_t * pdu = (access_pdu_tx_t *)access_pdu;
    model_base_t *base = mesh_kr_get_client_model_base();
    mesh_global_idx_t netkey_index = 0;
    mesh_key_refresh_transition_t transaction;
    dm_devkey_handle_t dev_handle = NULL;
    dm_netkey_handle_t netkey_handle = 0;
    if(NULL == base)
    {
        return ;
    }
    netkey_index = mesh_kr_client_key_refresh_phase_get_netkey_index(pdu);
    transaction = mesh_kr_client_key_refresh_phase_get_phase(pdu);
    if(MESH_CORE_SUCCESS != dm_netkey_index_to_netkey_handle(netkey_index,&netkey_handle))
    {
        return ;
    }
    dev_handle =mesh_kr_client_getnext_devkey(netkey_handle);
    if(MESH_INVALID_DEVKEY_HANDLE == dev_handle)
    {
        mesh_kr_client_devkey_next_init(netkey_handle);
        mesh_kr_set_client_key_refresh_phase(netkey_handle, transaction);
        return ;
    }
    mesh_kr_client_key_refresh_phase_set(netkey_handle,dev_handle,base,transaction);
    return ;
}


uint32_t mesh_kr_client_start_distribute(dm_netkey_handle_t netkey_handle)
{
     model_base_t *base = mesh_kr_get_client_model_base();
     dm_devkey_handle_t dev_handle = NULL;
     if(NULL == base)
     {
        return MESH_FAIL;
     }
     mesh_kr_set_client_update_status(netkey_handle, MESH_KR_UPDATE_DISTRIBUTE_KEY);
     dev_handle =mesh_kr_client_getnext_devkey(netkey_handle);
     if(MESH_INVALID_DEVKEY_HANDLE == dev_handle)
     {
         mesh_kr_set_client_update_status(netkey_handle, MESH_KR_UPDATE_NORMAL);
         return MESH_FAIL;
     }
      mesh_kr_client_distribute_netkey(netkey_handle,dev_handle,base);
      return MESH_OK;
}

void mesh_kr_client_rx_phase_set(dm_netkey_handle_t netkey_handle, mesh_key_refresh_transition_t transaction)
{

    if(transaction == MESH_KEY_TRANSITION_TO_PHASE_2)
    {
        dm_netkey_kr_phase_set(netkey_handle, MESH_KEY_REFRESH_PHASE_2);
    }
    else
    {
        dm_netkey_kr_phase_set(netkey_handle, MESH_KEY_REFRESH_PHASE_3);
    }
}


uint32_t mesh_kr_client_start_phase_set(dm_netkey_handle_t netkey_handle, mesh_key_refresh_transition_t transaction)
{
     model_base_t *base = mesh_kr_get_client_model_base();
     dm_devkey_handle_t dev_handle = NULL;
     if(NULL == base)
     {
        return MESH_FAIL;
     }
     mesh_kr_client_devkey_next_init(netkey_handle);
     dev_handle =mesh_kr_client_getnext_devkey(netkey_handle);
     if(MESH_INVALID_DEVKEY_HANDLE == dev_handle)
     {
         return MESH_FAIL;
     }
     mesh_kr_client_rx_phase_set(netkey_handle,transaction);
     mesh_kr_client_key_refresh_phase_set(netkey_handle,dev_handle,base,transaction);
     return MESH_OK;
}

uint32_t mesh_kr_client_update_done(dm_netkey_handle_t netkey_handle)
{
    return dm_netkey_update_commit(netkey_handle);    
}

mesh_kr_client_update_t mesh_kr_client_get_next_status(dm_netkey_handle_t netkey_handle)
{
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return MESH_KR_UPDATE_INVALID_STATUS;
    }
    mesh_kr_client_update_t status = netkey_update_state[pos].status;

    if(status == MESH_KR_UPDATE_WAIT_PHASE3_ACK)
    {
        return MESH_KR_UPDATE_NORMAL;
    }
    else 
    {
        return ((mesh_kr_client_update_t)(status+1));
    }
}


bool mesh_kr_client_check_all_ack(dm_netkey_handle_t netkey_handle)
{
    return dm_devkey_check_ack(netkey_handle);
}


void mesh_kr_client_distribute_key_timeout(dm_netkey_handle_t netkey_handle)
{
    
    LOG(LOG_LVL_INFO,"mesh_kr_client_distribute_key_timeout\n");
    mesh_kr_set_client_update_status(netkey_handle,MESH_KR_UPDATE_DISTRIBUTE_KEYFRESH_FLAG);  /* not good opr, some node maybe loss*/
    mesh_kr_client_start_phase_set(netkey_handle, MESH_KEY_TRANSITION_TO_PHASE_2);
}

void mesh_kr_client_distribute_flag_timeout(dm_netkey_handle_t netkey_handle)
{   
     LOG(LOG_LVL_INFO,"mesh_kr_client_distribute_flag_timeout\n");
     mesh_kr_set_client_update_status(netkey_handle,MESH_KR_UPDATE_INVOKE_KEY);
     mesh_kr_client_start_phase_set(netkey_handle, MESH_KEY_TRANSITION_TO_PHASE_3);
}

void mesh_kr_client_invoke_key_timeout(dm_netkey_handle_t netkey_handle)
{
    LOG(LOG_LVL_INFO,"mesh_kr_client_invoke_key_timeout\n");
    mesh_kr_client_update_done(netkey_handle);
    mesh_kr_set_client_update_status(netkey_handle,MESH_KR_UPDATE_NORMAL);
    mesh_kr_client_set_kr_trans_state(netkey_handle,0);
}


void mesh_kr_client_timer_init(void)
{
     
     meshKrClientTimer = mesh_timer_create("meshKrClientTimer",pdMS_TO_TICKS(MESH_KR_UPDATE_TICK),pdTRUE,(void *)0,meshKrClientTimeoutCallback);
#if 0
     if(meshKrClientTimer != NULL)
            xTimerStart(meshKrClientTimer,0);
#endif
}


void mesh_kr_client_init(void)
{
    memset(netkey_update_state,0,sizeof(netkey_update_state_t)*DM_CFG_NETKEY_MAX_NUM);
    mesh_kr_client_timer_init();
    addr_blacklist_init();
    kr_flag_init();
}


void mesh_kr_client_kr_reset(dm_netkey_handle_t netkey_handle)
{
     mesh_kr_client_devkey_status_reset(netkey_handle);
     dm_devkey_ack_reset(netkey_handle);
}


void mesh_kr_client_kr_start(dm_netkey_handle_t netkey_handle)
{
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return ;
    }
    mesh_kr_client_kr_reset(netkey_handle);
    mesh_kr_client_start_distribute(netkey_handle);
    netkey_update_state[pos].net_handler = netkey_handle;
    
    
}

void mesh_kr_client_normal_transaction(dm_netkey_handle_t netkey_handle)
{
   LOG(LOG_LVL_INFO,"mesh_kr_client_normal_transaction\n");
   mesh_kr_set_client_update_status(netkey_handle,mesh_kr_client_get_next_status(netkey_handle)); /* change securit beacon key refresh flag to 1 */
}

void mesh_kr_client_ack_timeout_transaction(dm_netkey_handle_t netkey_handle)
{
   
   LOG(LOG_LVL_INFO,"mesh_kr_client_ack_timeout_transaction\n");
   mesh_kr_set_client_update_status(netkey_handle,mesh_kr_client_get_next_status(netkey_handle)); /* change securit beacon key refresh flag to 1 */
   dm_devkey_ack_reset(netkey_handle);
   mesh_kr_client_devkey_next_init(netkey_handle);
   mesh_kr_client_set_kr_trans_state(netkey_handle,0);
   mesh_kr_client_update_done(netkey_handle);
}

void mesh_kr_client_trans_waitack_to_distribute_keyrefresh(dm_netkey_handle_t netkey_handle)
{
     LOG(LOG_LVL_INFO,"mesh_kr_client_trans_waitack_to_distribute_keyrefresh\n");
     mesh_kr_set_client_update_status(netkey_handle, MESH_KR_UPDATE_DISTRIBUTE_KEYFRESH_FLAG);
     mesh_kr_client_start_phase_set(netkey_handle, MESH_KEY_TRANSITION_TO_PHASE_2);
}


void mesh_kr_client_trans_phase2ack_to_phase3(dm_netkey_handle_t netkey_handle)
{
    
    LOG(LOG_LVL_INFO,"mesh_kr_client_trans_phase2ack_to_phase3\n");
    mesh_kr_set_client_update_status(netkey_handle, MESH_KR_UPDATE_INVOKE_KEY);
    mesh_kr_client_start_phase_set(netkey_handle, MESH_KEY_TRANSITION_TO_PHASE_3);
}


uint32_t mesh_kr_timer[MESH_KR_UPDATE_INVALID_STATUS] =
{
    0,
    0,
    DEFAULT_NETKEY_UPDATE_TIMER,
    DEFAULT_NETKEY_DISTRIBUTE_FLAG_TIMER,
    DEFAULT_NETKEY_DISTRIBUTE_FLAG_TIMER,
    DEFAULT_NETKEY_INVOKE_KEY_TIMER,
    DEFAULT_NETKEY_INVOKE_KEY_TIMER,    
};


static mesh_kr_client_timer_handler_table_t mesh_kr_client_timer_handler_table[] = {

    [0] =   {MESH_KR_UPDATE_NORMAL, NULL, NULL,NULL},
            {MESH_KR_UPDATE_DISTRIBUTE_KEY, NULL, mesh_kr_client_check_all_ack, mesh_kr_client_trans_waitack_to_distribute_keyrefresh},
            {MESH_KR_UPDATE_WAIT_ACK,mesh_kr_client_distribute_key_timeout, mesh_kr_client_check_all_ack, mesh_kr_client_trans_waitack_to_distribute_keyrefresh},
            {MESH_KR_UPDATE_DISTRIBUTE_KEYFRESH_FLAG, NULL, mesh_kr_client_check_all_ack, mesh_kr_client_trans_phase2ack_to_phase3},
            {MESH_KR_UPDATE_WAIT_PHASE2_ACK, mesh_kr_client_distribute_flag_timeout, mesh_kr_client_check_all_ack,mesh_kr_client_trans_phase2ack_to_phase3},
            {MESH_KR_UPDATE_INVOKE_KEY, NULL, mesh_kr_client_check_all_ack,mesh_kr_client_normal_transaction},
            {MESH_KR_UPDATE_WAIT_PHASE3_ACK, mesh_kr_client_invoke_key_timeout, mesh_kr_client_check_all_ack,mesh_kr_client_ack_timeout_transaction},
            
};

bool mesh_kr_client_is_timeout(uint32_t timecout,  mesh_kr_client_update_t update_status )
{
     if(timecout >= mesh_kr_timer[update_status])
     {
         return true;
     }
     return false;
}




static void meshKrClientTimeoutCallback(mesh_timer_t xTimer)
{
    dm_netkey_handle_t netkey_handle;
    dm_netkey_pos_t pos = 0;
    mesh_kr_client_update_t update_status = MESH_KR_UPDATE_INVALID_STATUS;

   
    mesh_core_params_t param;
    mesh_core_params_get(MESH_CORE_PARAM_MESH_ROLES , &param);
    if(MESH_ROLE_CONFIG_SERVER == param.role)
    {
        return ;
    }
    for(pos=0; pos < DM_CFG_NETKEY_MAX_NUM; pos++)
    {
        netkey_handle = netkey_update_state[pos].net_handler;
        if(MESH_INVALID_DEVKEY_HANDLE == netkey_handle)
        {
            continue;
        }
        update_status = mesh_kr_get_client_update_status(netkey_handle);
        if(update_status >= MESH_KR_UPDATE_INVALID_STATUS)
        {
            continue;
        }
        mesh_kr_client_timer_inc(netkey_handle);
        if(mesh_kr_client_is_timeout(mesh_kr_client_get_timercout(netkey_handle),update_status))
        {
            if(mesh_kr_client_timer_handler_table[update_status].timeout_handler != NULL)
            {
                mesh_kr_client_timer_handler_table[update_status].timeout_handler(netkey_handle);
            }
            mesh_kr_client_timer_clear(netkey_handle);
        }
        else if(mesh_kr_client_timer_handler_table[update_status].check_handler != NULL)
        {
            if(mesh_kr_client_timer_handler_table[update_status].check_handler(netkey_handle))
            {
                if(mesh_kr_client_timer_handler_table[update_status].trans_handler != NULL)
                {
                    mesh_kr_client_timer_handler_table[update_status].trans_handler(netkey_handle);
                    mesh_kr_client_timer_clear(netkey_handle);
                }
            }
        }
        
    }
}



void mesh_kr_client_devkey_ack_set(dm_netkey_handle_t netkey_handle, uint16_t dev_addr)
{
    dm_devkey_ack_set(netkey_handle,dev_addr);
}


void config_client_kr_netkey_status_ack(mesh_global_idx_t netkey_index, uint16_t dev_addr)
{
    dm_netkey_handle_t netkey_handle;
    if(MESH_CORE_SUCCESS != dm_netkey_index_to_netkey_handle(netkey_index,&netkey_handle))
    {
        return ;
    }
    mesh_kr_client_devkey_ack_set(netkey_handle, dev_addr);
}

void config_client_kr_phase_status_ack(mesh_global_idx_t netkey_index, uint16_t dev_addr)
{
    dm_netkey_handle_t netkey_handle;
    if(MESH_CORE_SUCCESS != dm_netkey_index_to_netkey_handle(netkey_index,&netkey_handle))
    {
        return ;
    }
    mesh_kr_client_devkey_ack_set(netkey_handle, dev_addr);
}

uint32_t config_client_kr_update_netkey(uint16_t global_idx, uint8_t *netkey)
{
    dm_netkey_handle_t netkey_handle;
    if(MESH_CORE_SUCCESS != dm_netkey_index_to_netkey_handle(global_idx,&netkey_handle))
    {
        return MESH_CORE_ERROR_NOT_FOUND;
    }
    return dm_netkey_update(netkey_handle,netkey,NULL);
}

uint32_t config_client_kr_add_netkey(uint16_t global_idx, uint8_t *netkey)
{
    dm_netkey_handle_t netkey_handle;
    return dm_netkey_add(global_idx,netkey,&netkey_handle,NULL);
}

void mesh_kr_client_output_env(void)
{
    dm_netkey_handle_t netkeylist[DM_CFG_NETKEY_MAX_NUM];
    uint32_t counter = 0;
    
    dm_get_all_netkey_handles((dm_netkey_handle_t *)netkeylist,&counter);
    if(MESH_INVALID_NETKEY_HANDLE == netkeylist[0])
    {
        return;
    }

#ifdef MESH_UART_DEBUG_TEST_CMD
     mesh_debug_uart_test_tx(netkeylist[0]->key[0].network_id,NETWORK_ID_LENGTH);
     mesh_debug_uart_test_tx(netkeylist[0]->key[0].netkey,MESH_KEY_LENGTH);
     
     mesh_debug_uart_test_tx(netkeylist[0]->key[1].network_id,NETWORK_ID_LENGTH);
     mesh_debug_uart_test_tx(netkeylist[0]->key[1].netkey,MESH_KEY_LENGTH);
#endif /* MESH_UART_DEBUG_TEST_CMD */
}


static void mesh_kr_client_set_kr_trans_state(dm_netkey_handle_t netkey_handle,uint8_t flag)
{
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return;
    }

    if(flag)
    {
        set_kr_flag(pos);
    }
    else
    {
        clear_kr_flag(pos);
    }
}

static bool mesh_kr_client_is_in_kr_transaction(dm_netkey_handle_t netkey_handle)
{
    dm_netkey_pos_t pos = 0;
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&pos))
    {
        return true;
    }
    return (get_kr_flag(pos) ? true:false);

}

static err_t  mesh_kr_client_search_dev(uint8_t *dev_uuid, uint8_t length, dm_netkey_handle_t *p_netkey_index, uint16_t* p_dev_addr)
{
    *p_netkey_index = 0;
    *p_dev_addr = 1;
    return MESH_CORE_ERROR_NOT_FOUND;
}

err_t mesh_client_blacklist_add(uint16_t *addr_list, uint8_t size)
{
    uint8_t index =0;
    if(NULL == addr_list)
    {
        return MESH_CORE_ERROR_INVALID_PARAM;
    }
    for(; index < size && index < MESH_KR_LIST_MAX_SIZE; index++)
    {
        blacklist_addr[index] = *(addr_list+index);
    }
    return MESH_CORE_SUCCESS;
}

static void  mesh_client_batch_kr_start(dm_netkey_handle_t * flag_list)
{
    dm_netkey_pos_t pos = 0;
    for(; pos < DEVKEY_BOUND_NETKEY_MAX_NUM; pos++)
    {
        if(flag_list[pos] == MESH_INVALID_NETKEY_HANDLE)
        {
             mesh_kr_client_kr_start(flag_list[pos]);
        }
    }
}

void mesh_client_add_kr_pool(dm_netkey_handle_t * pp_handle_pool,dm_netkey_handle_t p_net_handle)
{
    dm_netkey_pos_t pos;
    for(pos=0; pos < DEVKEY_BOUND_NETKEY_MAX_NUM; pos++)
    {
        if(pp_handle_pool[pos] == p_net_handle)
        {
            return;
        }
    }
    for(pos=0; pos < DEVKEY_BOUND_NETKEY_MAX_NUM; pos++)
    {
        if(pp_handle_pool[pos] == MESH_INVALID_NETKEY_HANDLE)
        {
            pp_handle_pool[pos]  = p_net_handle;
            return;
        }
    }
}

err_t mesh_client_batch_remove_node(uint16_t* p_dev_addr, uint8_t size)
{
      dm_netkey_handle_t netindex_search[DM_CFG_NETKEY_MAX_NUM];
      dm_netkey_handle_t nethandle_sum[DM_CFG_NETKEY_MAX_NUM];
      dm_netkey_pos_t pos;
      uint8_t index = 0;
      if(NULL == p_dev_addr)
      {
          return MESH_CORE_ERROR_INVALID_PARAM;
      }
      LOG(LOG_LVL_WARN,"mesh_client_batch_remove_node\n");
      memset(nethandle_sum,0,sizeof(nethandle_sum));
      for(; index < size; index++)
      {
           
           memset(netindex_search,0,sizeof(netindex_search));
           if(MESH_CORE_SUCCESS !=dm_devkey_uniaddr_belong_to_subnet_get(*(p_dev_addr+index),netindex_search))
           {
               continue; 
           }
           for(pos=0; pos < DEVKEY_BOUND_NETKEY_MAX_NUM; pos++)
           {
               if(netindex_search[pos] == MESH_INVALID_NETKEY_HANDLE)
               {
                    continue;
               }
               if(mesh_kr_client_is_in_kr_transaction(netindex_search[pos] ))
               {
                   
                   LOG(LOG_LVL_WARN,"mesh_client_remove_node IN_PROCESSING\n");
                   continue; 
               }
               mesh_client_add_kr_pool(nethandle_sum,netindex_search[pos] );
           }
      }
      addr_blacklist_init();
      mesh_client_blacklist_add(p_dev_addr,size);
      mesh_client_batch_kr_start(nethandle_sum);
      return MESH_CORE_SUCCESS;
}


err_t mesh_client_remove_node(uint8_t *dev_uuid, uint8_t length)
{
    err_t return_code = MESH_CORE_SUCCESS;
    dm_netkey_handle_t netkey_handle = 0;
    uint16_t dev_addr = 0;
    return_code = mesh_kr_client_search_dev(dev_uuid,length,&netkey_handle,&dev_addr);
    if(return_code != MESH_CORE_SUCCESS)
    {
        return return_code;
    }
    
    if(mesh_kr_client_is_in_kr_transaction(netkey_handle))
    {
        LOG(LOG_LVL_WARN,"mesh_client_remove_node IN_PROCESSING\n");
        return MESH_CORE_ERROR_KEYREFRESH_IN_PROCESSING;
    }
    mesh_kr_client_set_kr_trans_state(netkey_handle,1);
    addr_blacklist_init();
    return_code = mesh_client_blacklist_add(&dev_addr,1);
    if(return_code != MESH_CORE_SUCCESS)
    {
        return return_code;
    }
    mesh_kr_client_kr_start(netkey_handle);
    return MESH_CORE_SUCCESS;
}



