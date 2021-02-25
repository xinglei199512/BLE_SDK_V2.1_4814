/**
 ****************************************************************************************
 *
 * @file   provisioner_intf.c
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-29 11:25
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) BlueX Microelectronics 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"
#include "provision_comm.h"
#include "provision_api.h"
#include "provisioner_intf.h"
#include "mesh_gatt.h"
#include "beacon.h"
#include "provision_api.h"
#include "osapp_utils.h" 
#include "mesh_core_api.h" 
#include "ecc_p256.h"
#include "network_keys_dm.h"
#include "device_keys_dm.h"
#include "mesh_queued_msg.h"
#include "adv_mesh_msg_cache.h"
#include "node_save_misc.h"
#include "mesh_env.h"
#include "provision_fsm_comm.h"
#include "provisioner_fsm.h"
#include "provision.h"
#include "mesh_iv_operation.h"
#include "beacon_mesh_msg_cache.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

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
 extern provisioner_t m_provisioner;

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static void provisioner_gen_public_keys(void);
static void user_unprovisioned_dev_beacon_rx_callback(mesh_beacon_t *p_beacon_para);
static void user_provisioner_config_add_devkey(uint8_t * devkey , uint16_t addr);
static void provisioner_net_key_all_keys_generate_done(void* dm_handle);

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

void set_is_provisioner(bool value)
{
    mesh_core_params_t param;
    param.is_provisioner = value;
    mesh_core_params_set(MESH_CORE_PARAM_IS_PROVISIONER , &param);
}

bool get_is_provisioner(void)
{
    mesh_core_params_t param;
    mesh_core_params_get(MESH_CORE_PARAM_IS_PROVISIONER , &param);
    return param.is_provisioner;
}

static void provisioner_init(void)
{
    regisiter_beacon_rx_callback(user_unprovisioned_dev_beacon_rx_callback);
}

/* function */
void provisioner_action_send (mesh_prov_action_type_t type , mesh_prov_evt_param_t param)
{
    switch(type)
    {
        /*******PROVISIONER*******/
        //PROV_EVT_AUTH_INPUT_NUMBER
        case  PROV_ACTION_AUTH_INPUT_NUMBER_DONE : //input random number done
        {
             uint8_t * dev_uuid = param.prov.dev_uuid;
              prov_fsm_tcb_t *  fsm_tcb = client_search_dev_tcb(dev_uuid);
              if(NULL == fsm_tcb)
              {
                  return;
              }
              provisioner_input_auth(fsm_tcb,param.prov.param.p_input_val);
              prov_fsm_evt_handle(fsm_tcb,User_Input_Complete,NULL);
        }
        break;
        //PROV_EVT_READ_PEER_PUBLIC_KEY_OOB
        case  PROV_ACTION_READ_PEER_PUBLIC_KEY_OOB_DONE :
        {
              uint8_t * dev_uuid = param.prov.dev_uuid;
              prov_fsm_tcb_t *  fsm_tcb = client_search_dev_tcb( dev_uuid);
              if(NULL == fsm_tcb)
              {
                  return;
              }
              provisioner_input_oob_key(fsm_tcb,(uint8_t *)param.prov.param.p_public_keys);
              prov_fsm_evt_handle(fsm_tcb,User_Input_Complete,NULL);
        }
        break;
        //PROV_EVT_BEACON
        case  PROV_ACTION_SET_LINK_OPEN :
        {
            //link open
            //TODO
        }
        break;
        //PROV_EVT_CAPABILITIES
        case  PROV_ACTION_SEND_START_PDU :
        {
            if(param.prov.param.p_start_pdu)
            {
              uint8_t * dev_uuid = param.prov.dev_uuid;
              prov_fsm_tcb_t *  fsm_tcb = client_search_dev_tcb( dev_uuid);
              if(NULL == fsm_tcb)
              {
                  return;
              }
              provisioner_input_start(fsm_tcb,(uint8_t *)param.prov.param.p_start_pdu);
              prov_fsm_evt_handle(fsm_tcb,User_Input_Complete,NULL);
            }
        }
        break;
        default:break;
    }
}
void provisioner_config (mesh_prov_config_type_t opcode , mesh_prov_evt_param_t param)
{
    switch(opcode)
    {
        /*******PROVISIONER*******/
        case  PROV_SET_PROVISION_METHOD :
        {
            m_provisioner.val.method = param.prov.param.method;
        }
        break;
        case  PROV_SET_PRIVATE_KEY :
        {
            if(param.prov.param.p_prov_private_key)
            {
                //1. set mesh system  private key
                memcpy(m_provisioner.provisioner_key.private,param.prov.param.p_prov_private_key,GAP_P256_KEY_LEN);
                //2. gen public key
                provisioner_gen_public_keys();
            }
        }
        break;
        case  PROV_SET_AUTH_STATIC :
        {
       /*   if(param.prov.p_static_val)
            {
                memcpy(provision_authentication.msg_for_cfm.detail.auth_value,param.prov.p_static_val,AUTHVALUE_LEN);
            }
            
        */
        }
        break;
        case  PROV_SET_DISTRIBUTION_DATA :
        {
            dm_netkey_handle_t netkey_handle;

            if(param.prov.param.p_distribution)// not null
            {
            /// <1> copy distribution data
                //1. network_key //2. key_index     lsb //3. flags //4. current_iv_index   lsb  //5. unicast_addr lsb
                memcpy(&m_provisioner.provisioner_resources,param.prov.param.p_distribution,sizeof(provision_data_t));
            /// <2> set init defalut data
                mesh_beacon_iv_index_set(m_provisioner.provisioner_resources.current_iv_index);
                node_save_misc_iv_index();
                if(MESH_CORE_SUCCESS != dm_netkey_add(m_provisioner.provisioner_resources.key_index, m_provisioner.provisioner_resources.network_key,&netkey_handle,provisioner_net_key_all_keys_generate_done))
                {
                     LOG(LOG_LVL_ERROR,"PROV_SET_DISTRIBUTION_DATA fail\n");
                }
            }
        }
        break;
        case  PROV_SET_INVITE_DURATION :
        {
            m_provisioner.val.attention_duration = param.prov.param.attention_duration;
        }
        break;
        case  PROV_RESET :
        {
            //app_provisioner_init();
        }
        break;
        case  PROV_CLEAR_CACHE :
        {
            //1.clean bear rx  msg cache
           // mesh_flush_msg_cache();
                adv_mesh_msg_cache_clear();
                beacon_mesh_msg_cache_clear();

        }
        break;
        default:break;
    }
}
void provisioner_role_init(mesh_prov_evt_cb_t cb)
{
    //1. callback
    m_provisioner.evt = cb;
    //2. init
    provisioner_init();
}

static void user_unprovisioned_dev_beacon_rx_callback(mesh_beacon_t *p_beacon_para)
{      
        LOG(LOG_LVL_INFO,"user_unprovisioned_dev_beacon_rx_callback");
        //1. send to pc  beacon data
        
        if(m_provisioner.evt && p_beacon_para)// not null
        {
            mesh_prov_evt_param_t param;
            //evt
            param.prov.param.p_beacon = p_beacon_para;
            m_provisioner.evt(PROV_EVT_BEACON,param);
        }
}


void user_provisioner_fail_notify(uint8_t * dev_uuid,uint8_t reason )
{
        if(m_provisioner.evt)// not null
        {
            mesh_prov_evt_param_t param;
            memcpy(param.prov.dev_uuid,dev_uuid,MESH_DEVICE_UUID_LENGTH);
            param.prov.param.done_state.success = 0;
            param.prov.param.done_state.reason = reason;
            m_provisioner.evt(PROV_EVT_PROVISION_DONE,param);
        }
}

void user_provisioner_dev_key_gen_done(uint8_t * dev_uuid, uint8_t * devkey, uint16_t addr)
{
    ///<1>. save devkey
    LOG(3, "user_provisioner_dev_key_gen_done addr:%x\n", addr);
    user_provisioner_config_add_devkey(devkey,addr);
    ///<2>. send pc devkey
    if(m_provisioner.evt)// not null
    {
        mesh_prov_evt_param_t param;
        memcpy(param.prov.dev_uuid,dev_uuid,MESH_DEVICE_UUID_LENGTH);
        param.prov.param.done_state.success = 1;;
        m_provisioner.evt(PROV_EVT_PROVISION_DONE,param);
    }
}

static void user_provisioner_config_add_devkey(uint8_t * devkey , uint16_t addr)
{
    dm_devkey_handle_t l_handle;
    dm_netkey_handle_t l_net;

    //1. add devkey
    if(MESH_CORE_SUCCESS !=dm_devkey_add(addr,devkey,&l_handle))
    {
        BX_ASSERT(0);
    }
    //2. find net key
    if(MESH_CORE_SUCCESS !=dm_netkey_index_to_netkey_handle(m_provisioner.provisioner_resources.key_index,&l_net))
    {
        BX_ASSERT(0);
    }
    //3. bind net key
    if(MESH_CORE_SUCCESS !=dm_devkey_bound_netkey_add(l_net,l_handle))
    {
        BX_ASSERT(0);
    }
}

//Generate provisioner's public key
static void provisioner_gen_public_keys_done_out_isr(void)
{
    //1. Adjust the order
    reverse_self(m_provisioner.provisioner_key.private,GAP_P256_KEY_LEN);
    reverse_self(m_provisioner.provisioner_key.local_public.x,GAP_P256_KEY_LEN);
    reverse_self(m_provisioner.provisioner_key.local_public.y,GAP_P256_KEY_LEN);
    //2. send system msg
    m_provisioner.inited = true;
    beacon_mesh_msg_cache_clear(); 

}

//Generate provisioner's public key
static void provisioner_gen_public_keys_done_in_isr(void* param)
{
    mesh_run(provisioner_gen_public_keys_done_out_isr,portMAX_DELAY,true);
}
static void provisioner_gen_public_keys(void)
{
    //1. Adjust the order
    reverse_self(m_provisioner.provisioner_key.private,GAP_P256_KEY_LEN);
    //2. ecc_gen_new_public_key
    ecc_gen_new_public_key_usr(m_provisioner.provisioner_key.private,m_provisioner.provisioner_key.local_public.x,m_provisioner.provisioner_key.local_public.y,provisioner_gen_public_keys_done_in_isr);
}

static void provisioner_net_key_all_keys_generate_done(void* dm_handle)
{
    LOG(LOG_LVL_INFO,"provisioner_net_key_all_keys_generate_done\n");
}

mesh_provsion_method_t provisioner_provision_method_get(void)
{
    return (m_provisioner.val.method);
}

void link_open_tx_cb(void *tcb, Tx_Reason reason)
{
     LOG(LOG_LVL_INFO,"link_open_tx_cb : %d\n",reason);
     if(reason != Tx_Success_Finish)
     {
        prov_fsm_close_protocol_instance(tcb);
     }
     else
     {
         prov_client_start_provision(tcb);
     }
}

int start_provision_dev(uint8_t *dev_uuid)
{
      extern void provisioning_transation_adv_tx(void *instance, uint8_t *data, uint8_t length, void (*cb)(void *tcb, Tx_Reason reason));
      extern int32_t  provision_adv_compare(void * cookie, void *p_item);
      extern void provisioning_pdu_adv_cancel_tx(void *cookie,void (*cb)(void *tcb, Tx_Reason reason));
      LOG(LOG_LVL_INFO,"entern start_provision_dev uuid = ");
      if(!m_provisioner.inited)
      {
          return -1; // can not do provision before init finish.(public key not ready)
      }
      if(NULL ==  prov_client_find_dev(dev_uuid))
      {
         log_hex_data(dev_uuid,MESH_DEVICE_UUID_LENGTH);
         provision_adv_env_t *env =  provisioning_tx_pdu_buf_alloc(false);
         BX_ASSERT(env);
         prov_client_fsm_tcb_t * tcb =(prov_client_fsm_tcb_t *) prov_client_create_adv_protocol_instance(env,provisioning_transation_adv_tx, provision_adv_compare, provisioning_pdu_adv_cancel_tx);
         memcpy(tcb->dev_uuid,dev_uuid,MESH_DEVICE_UUID_LENGTH);
         provisioning_link_open_tx(env,dev_uuid,link_open_tx_cb);
         LOG(LOG_LVL_INFO,"start_provision_dev\n");
         return 0;
      }
      else
      {
         return  -1;
      }
}

