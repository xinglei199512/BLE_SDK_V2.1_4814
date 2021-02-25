/**
 ****************************************************************************************
 *
 * @file   unprov_device_intf.c
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-22 09:43
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

#include "unprov_device_intf.h"
#include "mesh_gatt.h"
#include "provision_comm.h"
#include "beacon.h"
#include "provision_api.h"
#include "unprov_device_fsm.h"
#include "osapp_utils.h" 
#include "mesh_core_api.h" 
#include "ecc_p256.h"
#include "network_keys_dm.h"
#include "device_keys_dm.h"
#include "mesh_queued_msg.h"
#include "adv_mesh_msg_cache.h"
#include "mesh_gatt_proxy.h"
#include "mesh_iv_operation.h"
#include "mesh_kr_comm.h"
#include "mesh_kr_server.h"
#include "node_save_misc.h"
#include "mesh_env.h"
#include "provisioning_s.h"
#include "provision_fsm_comm.h"
#include "mesh_reset_database.h"
#include "node_save_common.h"
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

typedef struct
{
    uint8_t * pvalue;   //the location to save
    void (*cb)();       //input complete callback
}provision_input_auth_t; 

typedef struct
{
    mesh_prov_evt_cb_t event;
    mesh_provsion_method_t network_method;          //network access:GATT or ADV
    provision_input_auth_t input_value;           //input location/callback location save.
    mesh_beacon_t beacon_para;                      //beacon content
    prov_fsm_tcb_t *tcb;
}unprov_dev_env_t;



/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static unprov_dev_env_t unprov_dev_env;
provision_capabilities_t        provision_capabilities;
//set function pointer
void (*user_provision_done)(uint8_t ,uint8_t );

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static void user_unprov_dev_provision_done(uint8_t success , uint8_t reason);
static void app_unprov_device_start_work(void);

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


/*
 * Init function
 ****************************************************************************************
 */



uint8_t * get_unprov_dev_uuid(void)
{

    return unprov_dev_env.beacon_para.dev_uuid;
}

void unprovisioned_dev_reset(void)
{
    //clear unprov env but not event
    mesh_prov_evt_cb_t cb = unprov_dev_env.event;
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(unprov_dev_env.tcb);
    memset(&unprov_dev_env,0,sizeof(unprov_dev_env));
    unprov_dev_env.event = cb;
    unprov_dev_env.tcb = &tcb->fsm_tcb;
    //reset mesh stack
    tools_random_generate(tcb->random.field.random_device , RANDOM_PDU_PARAMS_LEN);
    //set role
    set_is_provisioned(false);
}

void unprovisioned_dev_reset_random(void)
{
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(unprov_dev_env.tcb);
    tools_random_generate(tcb->random.field.random_device , RANDOM_PDU_PARAMS_LEN);
}

void unprov_device_init(mesh_prov_evt_cb_t cb)
{
    //1. init callback
    unprov_dev_env.event = cb;
    //2. init parameters
    if(get_is_provisioned() == true)
    {
        return;
    }
    unprov_dev_env.tcb  = prov_fsm_create_instance(MESH_ROLE_UNPROV_DEVICE);
    unprovisioned_dev_reset();

    //3. init function
    user_provision_done         = user_unprov_dev_provision_done;
}

static void app_unprov_device_start_adv_beacon(void)
{
    //Enable GATT provision
    if(unprov_dev_env.network_method & PROVISION_BY_GATT)
    {
        if(get_is_provisioned() == true)// The device has be provisioned
        {
            ble_mesh_gatt_adv_start(PROXY_ADV_EVT_PROVISIONING_DONE,0);
        }
        else
        {
            ble_mesh_gatt_adv_start(PROXY_ADV_EVT_PROVISIONING_START,(const uint8_t *)&unprov_dev_env.beacon_para);
        }
    }
    //Enable ADV provision
    if(unprov_dev_env.network_method & PROVISION_BY_ADV)
    {
        if(get_is_provisioned() == false)// The device has not be provisioned
        {
            unprovisioned_dev_beacon_start(&unprov_dev_env.beacon_para);
        }
    }
    //log
    LOG(LOG_LVL_INFO,"app_unprov_device_start_adv_beacon\n");
}


static void app_unprov_device_start_work(void)
{
    //1.clean bear rx  msg cache
   // mesh_flush_msg_cache();
     adv_mesh_msg_cache_clear();

    //2. start adv beacon
     app_unprov_device_start_adv_beacon();

    LOG(LOG_LVL_INFO,"app_unprov_device_init\n");
}


/*
 * generate public keys
 ****************************************************************************************
 */

//Generate provisioner's public key
static void unprov_gen_public_keys_done_out_isr(void)
{
    //1. Adjust the order
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(unprov_dev_env.tcb);
    reverse_self(tcb->provision_key.private,GAP_P256_KEY_LEN);
    reverse_self(tcb->provision_key.local_public.x,GAP_P256_KEY_LEN);
    reverse_self(tcb->provision_key.local_public.y,GAP_P256_KEY_LEN);
    //2. start unprov device work.
    app_unprov_device_start_work();
}

//Generate provisioner's public key
static void unprov_gen_public_keys_done_in_isr(void* param)
{
    mesh_run(unprov_gen_public_keys_done_out_isr,portMAX_DELAY,true);
}
static void unprov_gen_public_keys(void)
{
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(unprov_dev_env.tcb);
    //1. Adjust the order
    reverse_self(tcb->provision_key.private,GAP_P256_KEY_LEN);
    //2. ecc_gen_new_public_key
    ecc_gen_new_public_key_usr(tcb->provision_key.private,tcb->provision_key.local_public.x,tcb->provision_key.local_public.y,unprov_gen_public_keys_done_in_isr);
}

void unprov_refresh_public_keys(void)
{
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(unprov_dev_env.tcb);
    tools_random_generate(tcb->provision_key.private , RANDOM_PDU_PARAMS_LEN);
    unprov_gen_public_keys();
}
/*
 * Provision complete generate device keys and netkey.
 ****************************************************************************************
 */
static void unprov_device_network_keys_generation_done_cb(void* handle)
{
    LOG(3,"config_server_network_keys_generation_done_cb \n");

 //   if(PROVISION_BY_GATT == provision_active_method_get())
    {
        ble_mesh_gatt_evt_update(BLE_MESH_EVT_PROVISIONING_DONE,0);
    }
}

static void unprov_device_dev_key_gen_done(void)
{
    //1. save devkey
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(unprov_dev_env.tcb);
    dm_devkey_handle_t l_handle1;
    if(MESH_CORE_SUCCESS !=dm_devkey_add(tcb->provision_data.unicast_addr,tcb->devkey,&l_handle1))
    {
        BX_ASSERT(0);
    }
    //2. save netkey
    dm_netkey_handle_t l_handle2;
    if(MESH_CORE_SUCCESS != dm_netkey_add(tcb->provision_data.key_index,tcb->provision_data.network_key,&l_handle2,unprov_device_network_keys_generation_done_cb))
    {
        BX_ASSERT(0);
    }
    //3. save iv index
    mesh_beacon_iv_index_set(tcb->provision_data.current_iv_index);
    node_save_misc_iv_index();
    net_beacon_sec_flags_t flags =*((net_beacon_sec_flags_t *)(&tcb->provision_data.flags));
    mesh_beacon_iv_kr_flag_set(flags.iv_update);
    mesh_provision_kr_handle(tcb->provision_data.key_index,tcb->provision_data.network_key,flags.key_refresh);
    //4. save unicast address
    init_elmt_addr(tcb->provision_data.unicast_addr);
    node_save_element_uni_adddr();
    //5. event
 //   if(PROVISION_BY_GATT == provision_active_method_get())
    {
        ble_mesh_gatt_evt_update(BLE_MESH_EVT_FLASHSAVE_DONE,0);
    }
    prov_server_update(BLE_MESH_CLOSE_EVT_KEY_DONE);
    node_save_write_through();
    unprov_dev_env.tcb = NULL;
    if(unprov_dev_env.event)
    {
         mesh_prov_evt_param_t param;
         param.unprov.done_state.success = 1;
         param.unprov.done_state.reason = 0;
         unprov_dev_env.event(UNPROV_EVT_PROVISION_DONE,param);
    }
}



/*
 * User config parameter to mesh stack
 ****************************************************************************************
 */

void unprov_device_action_send (mesh_prov_action_type_t type , mesh_prov_evt_param_t param)
{
    switch(type)
    {
        /*******UNPROV DEVICE*******/
        //UNPROV_EVT_AUTH_INPUT_NUMBER
        case  UNPROV_ACTION_AUTH_INPUT_NUMBER_DONE : //input random number done
        {
            //1. save to system
            if(unprov_dev_env.input_value.pvalue)//not null
            {
                memcpy(unprov_dev_env.input_value.pvalue,param.unprov.p_input_val,AUTHVALUE_LEN);
            }
            //2. input complete callback
            if(unprov_dev_env.input_value.cb)//not null
            {
                unprov_dev_env.input_value.cb();
            }
        }
        break;
        default:break;
    }
}
void unprov_device_config (mesh_prov_config_type_t opcode , mesh_prov_evt_param_t param)
{
    switch(opcode)
    {
        case  UNPROV_SET_PROVISION_METHOD :
        {
            unprov_dev_env.network_method = param.unprov.method;
            if(get_is_provisioned() == true)
            {
                app_unprov_device_start_work();
            }

        }
        break;
        case  UNPROV_SET_BEACON :
        {
            if(param.unprov.p_beacon)
            {
                //1. copy beacon
                memcpy(&unprov_dev_env.beacon_para,param.unprov.p_beacon,sizeof(mesh_beacon_t));
            }
        }
        break;
        case  UNPROV_SET_PRIVATE_KEY :
        {
            if(get_is_provisioned() == true )
            {
                 break;;
            }
            if(param.unprov.p_unprov_private_key)
            {                
                prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(unprov_dev_env.tcb);
                //1. set mesh system  private key
                memcpy(tcb->provision_key.private,param.unprov.p_unprov_private_key,GAP_P256_KEY_LEN);
                //2. gen public key
                unprov_gen_public_keys();
            }
        }
        break;
        case  UNPROV_SET_OOB_CAPS :
        {
            if(param.unprov.p_dev_capabilities)
            {
                memcpy(&provision_capabilities ,param.unprov.p_dev_capabilities,sizeof(provision_capabilities_t));
            }
        }
        break;
        case  UNPROV_SET_AUTH_STATIC :
        {
            if(get_is_provisioned() == true )
            {
                 break;;
            }
            if(param.unprov.p_static_val)
            {
                prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(unprov_dev_env.tcb);
                memcpy(tcb->static_auth_value,param.unprov.p_static_val,AUTHVALUE_LEN);
            }
        }
        break;
        case  UNPROV_RESET :
        {
            unprovisioned_dev_reset();
        }
        break;
        default:break;
    }
}


/*
 * Send action to user
 ****************************************************************************************
 */

void user_unprov_dev_make_attention(uint8_t duration)
{
    if(unprov_dev_env.event)
    {
        mesh_prov_evt_param_t param;
        param.unprov.attention_duration = duration;
        unprov_dev_env.event(UNPROV_EVT_INVITE_MAKE_ATTENTION,param);
    }
}

void user_unprov_dev_expose_public_key_oob(public_key_t * public_keys)
{
    if(unprov_dev_env.event)
    {
        mesh_prov_evt_param_t param;
        param.unprov.p_public_keys = public_keys;
        unprov_dev_env.event(UNPROV_EVT_EXPOSE_PUBLIC_KEY,param);
    }
}

void unprov_input_auth_value(uint8_t *buff,void (*cb)())
{
    if(buff&&cb)// not null
    {
        //save pvalue hanlde and callback
        unprov_dev_env.input_value.pvalue = buff;
        unprov_dev_env.input_value.cb = cb;

        if(unprov_dev_env.event)
        {
            mesh_prov_evt_param_t param;
            param.unprov.p_input_val = buff;
            unprov_dev_env.event(UNPROV_EVT_AUTH_INPUT_NUMBER,param);
        }
    }
}

void unprov_output_auth_value(uint8_t *buff)
{
    if(buff)// not null
    {
        if(unprov_dev_env.event)
        {
            mesh_prov_evt_param_t param;
            param.unprov.p_output_val = buff;
            unprov_dev_env.event(UNPROV_EVT_AUTH_DISPLAY_NUMBER,param);
        }
    }
}

static void user_unprov_dev_provision_done(uint8_t success , uint8_t reason)
{
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(unprov_dev_env.tcb);

    // <1> notify user

    // <2> generate devkey and netkey. 
    if(success == 1)
    {
        ecdh_prov_salt_to_devkey(tcb->provision_key.ecdh_secret.x , tcb->session_info.provisioning_salt ,tcb->devkey, unprov_device_dev_key_gen_done);
    }
    else//provision fail
    {
        //adv beacon restart
        if(provision_system_method_get()&PROVISION_BY_GATT)
        {
            provision_service_beacon_restart();
        }
        unprovisioned_dev_beacon_restart();
    }
}

mesh_provsion_method_t unprov_priovision_method_get(void)
{
    return (unprov_dev_env.network_method);
}

 

