/**
 ****************************************************************************************
 *
 * @file   unprov_device_fsm.c
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-16 18:37
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

#include <string.h>
#include <co_math.h>
#include <co_endian.h>
#include "unprov_device_fsm.h"
#include "aes_ccm_cmac.h"
#include "provision_security.h"
#include "stack_mem_cfg.h"
#include "log.h"
#include "timer_wrapper.h"
#include "unprov_device_intf.h"
#include "mesh_env.h"
#include "mesh_queued_msg.h"
#include "mesh_gatt.h"
#include "mesh_reset_database.h"
#include "mesh_gatt_proxy.h"
#include "provisioning_s.h"
#include "osapp_utils.h"
#include "provision_fsm_comm.h"
#include "provision.h"
#include "osapp_utils.h"
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
typedef enum
{
    MEM_FREE_MASK_INVALID=0,
    MEM_FREE_MASK_PROVISION = 0x01,
    MEM_FREE_MASK_KEY = 0x02,
    MEM_FREE_MASK = 0x03,
}mem_free_mask_t;

typedef enum
{
    PROV_SERV_ADV_TYPE = 0,
    PROV_SERV_GATT_TYPE = 0x01,
}prov_serv_type_t;
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

static void confirmation_check(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param);



/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static prov_server_fsm_tcb_t* g_tcb;
static uint8_t confirmation_input[CONFIRMATION_INPUTS_BUF_SIZE];
extern  provision_capabilities_t        provision_capabilities;
extern void (*user_provision_done)(uint8_t ,uint8_t );


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void instance_to_save_data(prov_fsm_tcb_t *fsm_tcb)
{
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    uint8_t *buf = tcb->provision_encrypted_data.packed_data; 
    memcpy(tcb->provision_data.network_key       ,buf           ,MESH_NETWORK_KEY_LENGTH);
    memcpy(&tcb->provision_data.key_index         ,buf+16        ,2);
    memcpy(&tcb->provision_data.flags             ,buf+18        ,1);
    memcpy(&tcb->provision_data.current_iv_index  ,buf+19        ,4);
    memcpy(&tcb->provision_data.unicast_addr      ,buf+23        ,2);
    //reverse
    tcb->provision_data.key_index = co_bswap16(tcb->provision_data.key_index);
    tcb->provision_data.unicast_addr = co_bswap16(tcb->provision_data.unicast_addr);
    tcb->provision_data.current_iv_index = co_bswap32(tcb->provision_data.current_iv_index);
    set_is_provisioned(true);
    if((provision_system_method_get() & PROVISION_BY_GATT) == PROVISION_BY_GATT)
    {
         provision_set_database_reset();
         provision_set_database_pending();
    }
    user_provision_done(1,0);
}

prov_fsm_tcb_t *prov_serv_get_tcb(void)
{
    return (prov_fsm_tcb_t *)g_tcb;
}

void prov_server_changeto_idle(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    LOG(LOG_LVL_INFO,"changeto_idle\n");
//    prov_fsm_prepare_close_link(fsm_tcb,NULL);
//    if(provision_system_method_get()&PROVISION_BY_GATT)
//    {
//        provision_service_beacon_restart();
//    }
    unprovisioned_dev_beacon_restart();
    fsm_tcb->used = 0;
    fsm_tcb->state = IDLE_STATUS;
    fsm_tcb->timeout_flag = 0;
    mesh_timer_stop(fsm_tcb->state_timeout_timer);
    unprovisioned_dev_reset_random();
    unprov_refresh_public_keys();
    prov_fsm_close_protocol_instance(fsm_tcb);

}

static void prov_fsm_failed_pdu_tx(prov_fsm_tcb_t *fsm_tcb,uint8_t fail_reason)
{
    uint8_t length;
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    (void)tcb;
    uint8_t *buf = provisioning_pdu_build(Provisioning_Failed,&length);
    buf[PARAMS_OFFSET_IN_PROVISIONING_PDU] = fail_reason;
    LOG(LOG_LVL_INFO,"fail tx = ");
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
    mesh_free(buf);
}

static void prov_fsm_timeout_handler(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *pParam)
{
    prov_fsm_params_t param;
    param.close_param.error = PROVISIOON_TIMEOUT;
    prov_fsm_prepare_close_link(fsm_tcb,&param);
}
/*
static void prov_fsm_fail_handler(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *pParam)
{
    prov_fsm_params_t param;
    param.close_param.error = PROVISIOON_FAIL;
    prov_fsm_prepare_close_link(fsm_tcb,&param);
}
*/
static void capablities_pdu_fill(uint8_t *buf)
{
    buf[0 + PARAMS_OFFSET_IN_PROVISIONING_PDU] = provision_capabilities.elements_num;
    buf[1 + PARAMS_OFFSET_IN_PROVISIONING_PDU] = provision_capabilities.algorithms >> 8;
    buf[2 + PARAMS_OFFSET_IN_PROVISIONING_PDU] = provision_capabilities.algorithms && 0xff;
    buf[3 + PARAMS_OFFSET_IN_PROVISIONING_PDU] = provision_capabilities.public_key_type;
    buf[4 + PARAMS_OFFSET_IN_PROVISIONING_PDU] = provision_capabilities.static_oob_type;
    buf[5 + PARAMS_OFFSET_IN_PROVISIONING_PDU] = provision_capabilities.output_oob_size;
    buf[6 + PARAMS_OFFSET_IN_PROVISIONING_PDU] = provision_capabilities.output_oob_action >> 8;
    buf[7 + PARAMS_OFFSET_IN_PROVISIONING_PDU] = provision_capabilities.output_oob_action & 0xff;
    buf[8 + PARAMS_OFFSET_IN_PROVISIONING_PDU] = provision_capabilities.input_oob_size;
    buf[9 + PARAMS_OFFSET_IN_PROVISIONING_PDU] = provision_capabilities.input_oob_action >> 8;
    buf[10 + PARAMS_OFFSET_IN_PROVISIONING_PDU] = provision_capabilities.input_oob_action & 0xff;
}

static void capablities_tx(prov_fsm_tcb_t *fsm_tcb,prov_fsm_params_t *param)
{
    uint8_t length;
    uint8_t *buf = provisioning_pdu_build(Provisioning_Capabilities,&length);
    capablities_pdu_fill(buf);
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
    mesh_free(buf);
}

static void invite_rx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    LOG(LOG_LVL_INFO,"invite_rx\n");
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    memcpy(&tcb->invite.attation,param->pkt.data ,param->pkt.len);
    stop_current_beacon();
    user_unprov_dev_make_attention(param->pkt.data[0]);
}


static void capablities_timeout(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_tx_cancel(fsm_tcb);
    prov_fsm_set_stage_timeout(fsm_tcb);
}

static bool start_pdu_check(prov_server_fsm_tcb_t *tcb)
{
    if (tcb->select_capabilites.algorithm > PROV_ALGORITHM_FIPS_P256 ||
        tcb->select_capabilites.public_key > Using_OOB_Publickey ||
        tcb->select_capabilites.auth_method >  Input_OOB_Authentication)
    {
    	return false;
    }

    if (tcb->select_capabilites.public_key == Using_OOB_Publickey &&
        provision_capabilities.public_key_type != Using_OOB_Publickey)
    {
        return false;
    }

    if ((tcb->select_capabilites.auth_method == No_OOB_Authentication ||
         tcb->select_capabilites.auth_method == Static_OOB_Authentication) &&
        (tcb->select_capabilites.auth_action > 0 || tcb->select_capabilites.auth_size > 0))
    {
        return false;
    }

    if (tcb->select_capabilites.auth_method == Output_OOB_Authentication &&
       (tcb->select_capabilites.auth_action > OUTPUT_ALPHANUMERIC ||
        tcb->select_capabilites.auth_size < 1 || tcb->select_capabilites.auth_size > 8))
    {
        return false;
    }

    if (tcb->select_capabilites.auth_method == Input_OOB_Authentication &&
       (tcb->select_capabilites.auth_action >= INPUT_ALPHANUMERIC ||
        tcb->select_capabilites.auth_size < 1 || tcb->select_capabilites.auth_size > 8))
    {
        return false;
    }
    return true;
}

static void check_start_pdu(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    if(start_pdu_check(tcb))
    {
         prov_fsm_evt_handle(fsm_tcb,Start_Pdu_Check_Pass,param);
    }
    else
    {
         prov_fsm_evt_handle(fsm_tcb,Start_Pdu_Check_Fail,param);
    }

}

static void start_rx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    if(sizeof(provision_start_t) != param->pkt.len)
    {
        LOG(LOG_LVL_INFO,"receive invalid len start\n");
        return;
    }
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    memcpy(&tcb->select_capabilites,param->pkt.data ,param->pkt.len);
    LOG(LOG_LVL_INFO,"start_rx\n");
  
}


static void start_pdu_check_fail(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_failed_pdu_tx(fsm_tcb,Invalid_Format);
}

static void public_key_rx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    if(sizeof(public_key_t) != param->pkt.len)
    {
        LOG(LOG_LVL_INFO,"receive invalid len publickey\n");
        return;
    }
    memcpy(&tcb->provision_key.peer_public,param->pkt.data ,param->pkt.len);
}

static void public_key_tx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    uint8_t length;
    uint8_t *buf = provisioning_pdu_build(Provisioning_Public_Key,&length);
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    memcpy(buf+PARAMS_OFFSET_IN_PROVISIONING_PDU,&tcb->provision_key.local_public,sizeof(public_key_t));
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
    mesh_free(buf);

}

static void public_key_check(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_evt_handle(fsm_tcb,Public_Key_Check_Pass,NULL);
}

static void public_key_check_fail(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_failed_pdu_tx(fsm_tcb,Unexpected_PDU);
}

static void output_auth(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    Auth_Data_Type data_type = provision_get_auth_data_type(tcb->select_capabilites.auth_method,tcb->select_capabilites.auth_action);
    provision_generate_auth(tcb->provision_authentication.msg_for_cfm.detail.auth_value,tcb->select_capabilites.auth_size,data_type);
    unprov_output_auth_value(tcb->provision_authentication.msg_for_cfm.detail.auth_value);
}

static void unprov_dev_input_auth_value_done(void)
{
    prov_fsm_tcb_t *fsm_tcb = prov_serv_get_tcb();
    if(NULL == fsm_tcb)
    {
        LOG(LOG_LVL_INFO,"instance closed!!!!\n");
        return;
    }
    LOG(LOG_LVL_INFO,"unprov_dev_input_auth_value_done!!!!\n");
    prov_fsm_evt_handle(prov_serv_get_tcb(),User_Input_Complete,NULL);
}

static void notify_user_input_auth(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    LOG(LOG_LVL_INFO,"input auth!!!!\n");
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    unprov_input_auth_value(tcb->provision_authentication.msg_for_cfm.detail.auth_value,unprov_dev_input_auth_value_done);
}


static void public_key_tx_timeout(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    LOG(LOG_LVL_INFO,"public_key_tx_timeout\n");
    prov_fsm_tx_cancel(fsm_tcb);
    prov_fsm_set_stage_timeout(fsm_tcb);
}

static void copy_auth_info(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    if(tcb->select_capabilites.auth_method == Static_OOB_Authentication)
    {
        memcpy(tcb->provision_authentication.msg_for_cfm.detail.auth_value,tcb->static_auth_value,AUTHVALUE_LEN);
    }
}

static void notify_to_output_auth(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    output_auth(fsm_tcb,param);
}

static void prov_calc_confirmation_done(void* dummy)
{
    prov_fsm_tcb_t* fsm_tcb = (prov_fsm_tcb_t *)dummy;
    prov_fsm_params_t param;
    prov_fsm_evt_handle(fsm_tcb,Confirmation_Calc_Done,&param);
}

static void prov_calc_confirmation_cb(void* dummy, uint8_t result)
{
    prov_fsm_tcb_t* fsm_tcb = (prov_fsm_tcb_t *)dummy;
    mesh_queued_msg_send(prov_calc_confirmation_done,fsm_tcb);
    LOG(LOG_LVL_INFO,"prov_calc_confirmation_cb\n");
}

static void confirmation_input_assemble(prov_fsm_tcb_t *fsm_tcb,uint8_t * data)
{
     prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
     memcpy(data  +INVITE_PDU_OFFSET_IN_CONFIRMATION_INPUTS,&tcb->invite,INVITE_PDU_PARAMS_LEN);
     LOG(LOG_LVL_INFO,"invite data = ");
     log_hex_data((uint8_t  *)&tcb->invite, INVITE_PDU_PARAMS_LEN);
     capablities_pdu_fill(data + CAPABILITIES_PDU_OFFSET_INCONFIRMATION_INPUTS -1);
     memcpy(data + START_PDU_OFFSET_IN_CONFIRMATION_INPUTS,&tcb->select_capabilites,START_PDU_PARAMS_LEN);
     memcpy(data + PROVISIONER_PUBLIC_KEY_OFFSET_IN_CONFIRMATION_INPUTS,&tcb->provision_key.peer_public,PUBLIC_KEY_PDU_PARAMS_LEN);
     memcpy(data + DEVICE_PUBLIC_KEY_OFFSET_IN_CONFIRMATION_INPUTS,&tcb->provision_key.local_public,PUBLIC_KEY_PDU_PARAMS_LEN);
}

static void confirm_rx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    memcpy(tcb->confirmation.peer,param->pkt.data ,param->pkt.len);
    LOG(LOG_LVL_INFO,"confirm_rx\n");
}


static void input_complete_tx(prov_fsm_tcb_t *fsm_tcb)
{
    uint8_t length;
    uint8_t *buf = provisioning_pdu_build(Provisioning_Input_Complete,&length);
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
    mesh_free(buf);
}

static void user_auth_input(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
     LOG(LOG_LVL_INFO,"user_auth_input\n");
     input_complete_tx(fsm_tcb);
}

static void input_compete_tx_timeout(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_tx_cancel(fsm_tcb);
    prov_fsm_set_stage_timeout(fsm_tcb);
    LOG(LOG_LVL_INFO,"input_compete_tx_timeout\n");
}

static void calc_confirmation(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_confirmation_calc_t calc_info;
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    calc_info.dummy = fsm_tcb;
    calc_info.prov_callback = prov_calc_confirmation_cb;
    calc_info.prov_authentication = &tcb->provision_authentication;
    calc_info.prov_confirmation   = &tcb->confirmation;
    calc_info.prov_confirmation_input = confirmation_input;
    confirmation_input_assemble(fsm_tcb,calc_info.prov_confirmation_input);
    calc_info.prov_key  = &tcb->provision_key;
    calc_info.prov_random = &tcb->random;
    LOG(LOG_LVL_INFO,"random data = ");
    log_hex_data(tcb->random.field.random_device, RANDOM_PDU_PARAMS_LEN);
    memcpy(tcb->provision_authentication.msg_for_cfm.detail.random,tcb->random.field.random_device,RANDOM_PDU_PARAMS_LEN);
    provision_calc_confirmation_start(&calc_info);
    LOG(LOG_LVL_INFO,"calc_confirmation\n");
     
}

static void confirmation_tx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    uint8_t length;
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    uint8_t *buf = provisioning_pdu_build(Provisioning_Confirmation,&length);
    memcpy(buf+PARAMS_OFFSET_IN_PROVISIONING_PDU,tcb->confirmation.local,CONFIRMATION_PDU_PARAMS_LEN);
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
    LOG(LOG_LVL_INFO,"confirmation_tx\n");
    mesh_free(buf);

}

static void calc_confirmation_finish(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    confirmation_tx(fsm_tcb,param);
    LOG(LOG_LVL_INFO,"calc_confirmation_finish\n");
}

static void confirmation_tx_timeout(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_tx_cancel(fsm_tcb);
    prov_fsm_set_stage_timeout(fsm_tcb);
    LOG(LOG_LVL_INFO,"confirmation_tx_timeout\n");
}


static void random_rx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    if(param->pkt.len != RANDOM_PDU_PARAMS_LEN)
    {
         LOG(LOG_LVL_INFO,"random_rx invalid len\n");
         return;
    }
    LOG(LOG_LVL_INFO,"random_rx =");
    log_hex_data(param->pkt.data, RANDOM_PDU_PARAMS_LEN);
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    memcpy(tcb->random.field.random_provisioner,param->pkt.data,RANDOM_PDU_PARAMS_LEN);
    memcpy(tcb->provision_authentication.msg_for_cfm.detail.random,param->pkt.data,RANDOM_PDU_PARAMS_LEN);
    LOG(LOG_LVL_INFO,"random_rx\n");

}

static void confirmation_check_fail(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_failed_pdu_tx(fsm_tcb,Confirmation_Failed);
}

static void confirmation_check_finish(void * dummy)
{
    uint8_t check_result = 0;
    prov_fsm_params_t param;
    prov_fsm_tcb_t *fsm_tcb = (prov_fsm_tcb_t * )dummy;
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    LOG(LOG_LVL_INFO,"peer data = ");
    log_hex_data(tcb->confirmation.peer, CONFIRMATION_PDU_PARAMS_LEN);
    LOG(LOG_LVL_INFO,"calc data = ");
    log_hex_data(tcb->provision_authentication.confirmation_rslt, CONFIRMATION_PDU_PARAMS_LEN);
    check_result =  memcmp(tcb->confirmation.peer,tcb->provision_authentication.confirmation_rslt
                    ,CONFIRMATION_PDU_PARAMS_LEN)?false:true;
    LOG(LOG_LVL_INFO,check_result ? "confirmation_check:OK\n" : "confirmation_check:FAIL\n");
    if(check_result)
    {
        param.close_param.error = PROVISIOON_SUCCESS;
        prov_fsm_evt_handle(fsm_tcb,Confirmation_Check_Pass,&param);
    }
    else
    {
        param.close_param.error = PROVISIOON_FAIL;
        prov_fsm_evt_handle(fsm_tcb,Confirmation_Check_Fail,&param);
    }
}

static void confirmation_check_cb(void * dummy, uint8_t status)
{
    mesh_queued_msg_send(confirmation_check_finish,dummy);
}


static void confirmation_check(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_confirmation_check_t check_info;
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    check_info.check_callback = confirmation_check_cb;
    check_info.dummy = fsm_tcb;
    check_info.confirm_key = tcb->provision_authentication.confirmation_key;
    check_info.msg         = tcb->provision_authentication.msg_for_cfm.buf;
    check_info.length      = sizeof(tcb->provision_authentication.msg_for_cfm.buf);
    check_info.rst         = tcb->provision_authentication.confirmation_rslt;
    provision_check_confirmation_start(&check_info);
}

static void random_tx(prov_fsm_tcb_t *fsm_tcb)
{
    uint8_t length;
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    uint8_t *buf = provisioning_pdu_build(Provisioning_Random,&length);
    memcpy(buf+PARAMS_OFFSET_IN_PROVISIONING_PDU,tcb->random.field.random_device,RANDOM_PDU_PARAMS_LEN);
    LOG(LOG_LVL_INFO,"random tx = ");
    log_hex_data(tcb->random.field.random_device, RANDOM_PDU_PARAMS_LEN);
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
    mesh_free(buf);

}


static void confirmation_check_done(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    random_tx(fsm_tcb);
}


static void random_tx_timeout(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_tx_cancel(fsm_tcb);
    prov_fsm_set_stage_timeout(fsm_tcb);
    LOG(LOG_LVL_INFO,"random_tx_timeout\n");
}


static void session_calc_cb(void *dummy, uint8_t status)
{
    prov_fsm_params_t param;
    prov_fsm_tcb_t *fsm_tcb = (prov_fsm_tcb_t * )dummy;
    prov_fsm_evt_handle(fsm_tcb,Session_Key_Calc_Done,&param);
}


static void session_calc(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_session_key_calc_t session_calc_info;
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    session_calc_info.provisioning_salt_input = tcb->random.provisioning_salt_input;
    session_calc_info.salt_input_length       = sizeof(tcb->random.provisioning_salt_input);
    session_calc_info.provisioning_salt       = tcb->session_info.provisioning_salt;
    session_calc_info.ecdh_secret             = &tcb->provision_key.ecdh_secret;
    session_calc_info.session_key             = tcb->session_info.session_key;
    session_calc_info.session_nonce           = tcb->session_info.session_nonce;
    session_calc_info.session_callback        = session_calc_cb;
    session_calc_info.dummy                   = fsm_tcb;
    provision_session_key_calc_start(&session_calc_info);
}

static void data_decrypted_fail(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_failed_pdu_tx(fsm_tcb,Decryption_Failed);
}

static void unprov_dev_data_decrypted(ccm_cmac_buf_t *ccm_base,void *dummy,uint8_t status)
{
    prov_fsm_params_t param;
    prov_fsm_tcb_t *fsm_tcb = (prov_fsm_tcb_t *)g_tcb;
    
    if(status == DECRYPT_SUCCESSFUL)//success
    {
        prov_fsm_evt_handle(fsm_tcb,Decrypt_Success,&param);
    }else
    {
        prov_fsm_evt_handle(fsm_tcb,Decrypt_Fail,&param);
    }
}


static void decrypted_provisioning_data(prov_fsm_tcb_t *fsm_tcb)
{
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
    ccm_cmac_buf_t ccm_decrypt=
    {
        .param.ccm = {
            .key =   tcb->session_info.session_key,
            .nonce = tcb->session_info.session_nonce + 3,
            .msg = tcb->provision_encrypted_data.encrypted_data,
            .mic_length = PROVISIONING_DATA_MIC_LEN,
            .msg_length = ENCRYPTED_PROVISIONING_DATA_LEN,
            .additional_data = NULL,
            .additional_data_length = 0,
            .rslt = tcb->provision_encrypted_data.packed_data,
        },
        .op_type = CCM_DECRYPT,
    };
    ccm_cmac_start(&ccm_decrypt,unprov_dev_data_decrypted);
}

static void session_calc_finish(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    decrypted_provisioning_data(fsm_tcb);
}

static void prov_data_rx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
     prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);
     memcpy(tcb->provision_encrypted_data.encrypted_data,param->pkt.data,param->pkt.len);
}


static void complete_tx(prov_fsm_tcb_t *fsm_tcb)
{
    uint8_t length;
    uint8_t *buf = provisioning_pdu_build(Provisioning_Complete,&length);
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
    LOG(LOG_LVL_INFO,"complete_tx\n");
    mesh_free(buf);

}

static void decrypt_data_done(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    complete_tx(fsm_tcb);
}

static void complete_tx_timeout(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_tx_cancel(fsm_tcb);
    prov_fsm_set_stage_timeout(fsm_tcb);
    LOG(LOG_LVL_INFO,"complete_tx_timeout\n");
}

static void complete_tx_finish(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_params_t param_close ;
    param_close.close_param.error = PROVISIOON_SUCCESS;

    instance_to_save_data(fsm_tcb);
    prov_fsm_prepare_close_link(fsm_tcb,&param_close);
}

static void unexpect_pdu_check_fail(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_failed_pdu_tx(fsm_tcb,Unexpected_PDU);
}

void prov_server_provision_finish(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    LOG(LOG_LVL_INFO,"prov_server_provision_finish\n");
    fsm_tcb->used = 0;
    fsm_tcb->timeout_flag = 0;
    mesh_timer_stop(fsm_tcb->state_timeout_timer);
    if(NULL !=fsm_tcb->close_func)
    {
        fsm_tcb->close_func(fsm_tcb->cookie,0);
    }
    prov_server_free_instance(fsm_tcb);

}


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


static prov_fsm_table_t  capabalities_change_fsm_table[] = {
      {IDLE_STATUS,                        Provisioning_Invite_Rx,           WAIT_INVITE_ACK_STATUS               ,  invite_rx,                 },

      {WAIT_INVITE_ACK_STATUS,             Ack_Finish,                       CAPABILITIES_TX_STATUS               ,  capablities_tx     ,       },

      {CAPABILITIES_TX_STATUS,             Timeout,                          CAPABILITIES_TX_CANCEL_STATUS        ,  capablities_timeout,       },
      {CAPABILITIES_TX_STATUS,             Tx_Success,                       WAIT_START_STATUS                    ,  NULL,                      },
      {CAPABILITIES_TX_STATUS,             Tx_Fail,                          LINK_CLOSE_TX_STATUS                 ,  prov_fsm_tx_fail,          },
      {CAPABILITIES_TX_STATUS,             Provisioning_Fail_Rx,             CAPABILITIES_TX_CANCEL_STATUS        ,  capablities_timeout,       },

      {CAPABILITIES_TX_CANCEL_STATUS,      Tx_Timeout_Cancel,                LINK_CLOSE_TX_STATUS                 ,  prov_fsm_timeout_handler,  },

      {WAIT_START_STATUS,                  Timeout,                          LINK_CLOSE_TX_STATUS                 ,  prov_fsm_timeout_handler,  },
      {WAIT_START_STATUS,                  Provisioning_Start_Rx,            WAIT_START_ACK_STATUS                ,  start_rx,                  },
      {WAIT_START_STATUS,                  Provisioning_Capablities_Rx,      WAIT_RX_ACK_STATUS                   ,  NULL                  ,    },
      {WAIT_START_STATUS,                  Provisioning_Invite_Rx,           WAIT_RX_ACK_STATUS                   ,  NULL                  ,    },
      {WAIT_START_STATUS,                  Provisioning_Public_Key_Rx,       WAIT_RX_ACK_STATUS                   ,  NULL                  ,    },
      {WAIT_START_STATUS,                  Provisioning_Input_Complete_Rx,   WAIT_RX_ACK_STATUS                   ,  NULL                  ,    },
      {WAIT_START_STATUS,                  Provisioning_Confirmation_Rx,     WAIT_RX_ACK_STATUS                   ,  NULL                  ,    },
      {WAIT_START_STATUS,                  Provisioning_Random_Rx,           WAIT_RX_ACK_STATUS                   ,  NULL                  ,    },
      {WAIT_START_STATUS,                  Provisioning_Data_Rx,             WAIT_RX_ACK_STATUS                   ,  NULL                  ,    },
      {WAIT_START_STATUS,                  Provisioning_Complete_Rx,         WAIT_RX_ACK_STATUS                   ,  NULL                  ,    },

      {WAIT_START_ACK_STATUS,              Ack_Finish,                       START_CHECK_STATUS                   , check_start_pdu,            },

      {START_CHECK_STATUS,                 Start_Pdu_Check_Pass,             CAPABILITIES_EXCHANGE_FINISH_STATUS  , NULL,                       },
      {START_CHECK_STATUS,                 Start_Pdu_Check_Fail,             FAIL_PDU_TX_STATUS                   , start_pdu_check_fail,       },
};    
static uint8_t capabalities_change_fsm_table_size = ARRAY_TABLE_SIZE(capabalities_change_fsm_table);

static prov_fsm_table_t no_oob_fsm_table[] = {

      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Public_Key_Rx,      WAIT_PUBLIC_KEY_ACK_STATUS          ,  public_key_rx,            },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Capablities_Rx,     WAIT_RX_ACK_STATUS                  ,  NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Invite_Rx,          WAIT_RX_ACK_STATUS                  ,  NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Start_Rx,           WAIT_RX_ACK_STATUS                  ,  NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Input_Complete_Rx,  WAIT_RX_ACK_STATUS                  ,  NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Confirmation_Rx,    WAIT_RX_ACK_STATUS                  ,  NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Random_Rx,          WAIT_RX_ACK_STATUS                  ,  NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Data_Rx,            WAIT_RX_ACK_STATUS                  ,  NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Complete_Rx,        WAIT_RX_ACK_STATUS                  ,  NULL                  ,   },

      {WAIT_PUBLIC_KEY_ACK_STATUS,           Ack_Finish,                      PUBLIC_KEY_CHECK_STATUS             ,  public_key_check,         },
      {PUBLIC_KEY_CHECK_STATUS,              Public_Key_Check_Pass,           PUBLIC_KEY_TX_STATUS                ,  public_key_tx,            },
      {PUBLIC_KEY_CHECK_STATUS,              Public_Key_Check_Fail,           FAIL_PDU_TX_STATUS                  ,  public_key_check_fail,    },

      {PUBLIC_KEY_TX_STATUS,                Timeout,                          PUBLIC_KEY_TX_CANCEL_STATUS         ,  public_key_tx_timeout,    },
      {PUBLIC_KEY_TX_STATUS,                Tx_Success,                       WAIT_PEER_CONFIRMATION_STATUS       ,  copy_auth_info,           },
      {PUBLIC_KEY_TX_STATUS,                Tx_Fail,                          LINK_CLOSE_TX_STATUS                ,  prov_fsm_tx_fail,         },
      {PUBLIC_KEY_TX_STATUS,                Provisioning_Fail_Rx,             PUBLIC_KEY_TX_CANCEL_STATUS         ,  public_key_tx_timeout,    },

      {PUBLIC_KEY_TX_CANCEL_STATUS,         Tx_Timeout_Cancel,                LINK_CLOSE_TX_STATUS                ,  prov_fsm_timeout_handler, },
};
static uint8_t no_oob_fsm_table_size = ARRAY_TABLE_SIZE(no_oob_fsm_table);
static prov_fsm_table_t no_oob_out_auth_table[] = {

      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Public_Key_Rx,      WAIT_PUBLIC_KEY_ACK_STATUS          , public_key_rx,            },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Capablities_Rx,     WAIT_RX_ACK_STATUS                  , NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Invite_Rx,          WAIT_RX_ACK_STATUS                  , NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Start_Rx,           WAIT_RX_ACK_STATUS                  , NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Input_Complete_Rx,  WAIT_RX_ACK_STATUS                  , NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Confirmation_Rx,    WAIT_RX_ACK_STATUS                  , NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Random_Rx,          WAIT_RX_ACK_STATUS                  , NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Data_Rx,            WAIT_RX_ACK_STATUS                  , NULL                  ,   },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Complete_Rx,        WAIT_RX_ACK_STATUS                  , NULL                  ,   },

      {WAIT_PUBLIC_KEY_ACK_STATUS,           Ack_Finish,                      PUBLIC_KEY_TX_STATUS                , public_key_tx,            },
      {PUBLIC_KEY_TX_STATUS,                 Timeout,                         PUBLIC_KEY_TX_CANCEL_STATUS         , public_key_tx_timeout,    },
      {PUBLIC_KEY_TX_STATUS,                 Tx_Success,                      WAIT_PEER_CONFIRMATION_STATUS       , notify_to_output_auth,    },
      {PUBLIC_KEY_TX_STATUS,                 Tx_Fail,                         LINK_CLOSE_TX_STATUS                , prov_fsm_tx_fail,         },
      {PUBLIC_KEY_TX_STATUS,                 Provisioning_Fail_Rx,            PUBLIC_KEY_TX_CANCEL_STATUS         , public_key_tx_timeout,    },

      {PUBLIC_KEY_TX_CANCEL_STATUS,          Tx_Timeout_Cancel,               LINK_CLOSE_TX_STATUS                , prov_fsm_timeout_handler, },
};
static uint8_t no_oob_out_auth_table_size = ARRAY_TABLE_SIZE(no_oob_out_auth_table);
static prov_fsm_table_t no_oob_input_auth_table[] = {

      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Public_Key_Rx,     WAIT_PUBLIC_KEY_ACK_STATUS       , public_key_rx,               },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Capablities_Rx,    WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Invite_Rx,         WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Start_Rx,          WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Input_Complete_Rx, WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Confirmation_Rx,   WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Random_Rx,         WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Data_Rx,           WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Complete_Rx,       WAIT_RX_ACK_STATUS               , NULL                  ,      },

      {WAIT_PUBLIC_KEY_ACK_STATUS,           Ack_Finish,                     PUBLIC_KEY_TX_STATUS             , public_key_tx,               },

      {PUBLIC_KEY_TX_STATUS,                Timeout,                         PUBLIC_KEY_TX_CANCEL_STATUS      , public_key_tx_timeout,       },
      {PUBLIC_KEY_TX_STATUS,                Tx_Success,                      WAIT_AUTH_INFO_STATUS            , notify_user_input_auth,      },
      {PUBLIC_KEY_TX_STATUS,                Tx_Fail,                         LINK_CLOSE_TX_STATUS             , prov_fsm_tx_fail,            },
      {PUBLIC_KEY_TX_STATUS,                Provisioning_Fail_Rx,            PUBLIC_KEY_TX_CANCEL_STATUS      , public_key_tx_timeout,       },

      {PUBLIC_KEY_TX_CANCEL_STATUS,         Tx_Timeout_Cancel,               LINK_CLOSE_TX_STATUS             , prov_fsm_timeout_handler,    },

 //    {WAIT_AUTH_INFO_STATUS,              Timeout,                         LINK_CLOSE_TX_STATUS             , prov_fsm_prepare_close_link, },
      {WAIT_AUTH_INFO_STATUS,               User_Input_Complete,             INPUT_COMPELE_TX_STATUS          , user_auth_input,             },
      {WAIT_AUTH_INFO_STATUS,               Provisioning_Fail_Rx,            LINK_CLOSE_TX_STATUS             , prov_fsm_prepare_close_link, },

      {INPUT_COMPELE_TX_STATUS,             Timeout,                         INPUT_COMPELE_TX_CANCEL_STATUS   , input_compete_tx_timeout,    },
      {INPUT_COMPELE_TX_STATUS,             Tx_Success,                      WAIT_PEER_CONFIRMATION_STATUS    , NULL,                        },
      {INPUT_COMPELE_TX_STATUS,             Tx_Fail,                         LINK_CLOSE_TX_STATUS             , prov_fsm_tx_fail,            },
      {INPUT_COMPELE_TX_STATUS,             Provisioning_Fail_Rx,            INPUT_COMPELE_TX_CANCEL_STATUS   , input_compete_tx_timeout,    },

      {INPUT_COMPELE_TX_CANCEL_STATUS,      Tx_Timeout_Cancel,               LINK_CLOSE_TX_STATUS             , prov_fsm_timeout_handler,    },
};
static uint8_t no_oob_input_auth_table_size = ARRAY_TABLE_SIZE(no_oob_input_auth_table);
static prov_fsm_table_t   oob_publickey_no_auth_fsm_table[] = {

      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Public_Key_Rx,     WAIT_PUBLIC_KEY_ACK_STATUS        , public_key_rx,          },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Capablities_Rx,    WAIT_RX_ACK_STATUS                , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Invite_Rx,         WAIT_RX_ACK_STATUS                , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Start_Rx,          WAIT_RX_ACK_STATUS                , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Input_Complete_Rx, WAIT_RX_ACK_STATUS                , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Confirmation_Rx,   WAIT_RX_ACK_STATUS                , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Random_Rx,         WAIT_RX_ACK_STATUS                , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Data_Rx,           WAIT_RX_ACK_STATUS                , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Complete_Rx,       WAIT_RX_ACK_STATUS                , NULL                  , },

      {WAIT_PUBLIC_KEY_ACK_STATUS,           Ack_Finish,                     WAIT_PEER_CONFIRMATION_STATUS     , NULL,                   },

};
static uint8_t oob_publickey_no_auth_fsm_table_size = ARRAY_TABLE_SIZE(oob_publickey_no_auth_fsm_table);
static prov_fsm_table_t   oob_publickey_out_auth_fsm_table[] = {

      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Public_Key_Rx,     WAIT_PUBLIC_KEY_ACK_STATUS      , public_key_rx,          },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Capablities_Rx,    WAIT_RX_ACK_STATUS              , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Invite_Rx,         WAIT_RX_ACK_STATUS              , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Start_Rx,          WAIT_RX_ACK_STATUS              , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Input_Complete_Rx, WAIT_RX_ACK_STATUS              , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Confirmation_Rx,   WAIT_RX_ACK_STATUS              , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Random_Rx,         WAIT_RX_ACK_STATUS              , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Data_Rx,           WAIT_RX_ACK_STATUS              , NULL                  , },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Complete_Rx,       WAIT_RX_ACK_STATUS              , NULL                  , },

      {WAIT_PUBLIC_KEY_ACK_STATUS,           Ack_Finish,                     WAIT_PEER_CONFIRMATION_STATUS    ,output_auth,             },
};
static uint8_t oob_publickey_out_auth_fsm_table_size = ARRAY_TABLE_SIZE(oob_publickey_out_auth_fsm_table);

static prov_fsm_table_t   oob_publickey_input_auth_fsm_table[] = {

      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Public_Key_Rx,     WAIT_PUBLIC_KEY_ACK_STATUS       , public_key_rx             ,  },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Capablities_Rx,    WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Invite_Rx,         WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Start_Rx,          WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Input_Complete_Rx, WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Confirmation_Rx,   WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Random_Rx,         WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Data_Rx,           WAIT_RX_ACK_STATUS               , NULL                  ,      },
      {CAPABILITIES_EXCHANGE_FINISH_STATUS,  Provisioning_Complete_Rx,       WAIT_RX_ACK_STATUS               , NULL                  ,      },

      {WAIT_PUBLIC_KEY_ACK_STATUS,           Ack_Finish,                     WAIT_AUTH_INFO_STATUS            , notify_user_input_auth,      },

//    {WAIT_AUTH_INFO_STATUS,                Timeout,                        LINK_CLOSE_TX_STATUS             , prov_fsm_prepare_close_link, },
      {WAIT_AUTH_INFO_STATUS,                User_Input_Complete,            INPUT_COMPELE_TX_STATUS          , user_auth_input,             },
      {WAIT_AUTH_INFO_STATUS,                Provisioning_Fail_Rx,           LINK_CLOSE_TX_STATUS             , prov_fsm_prepare_close_link, },

      {INPUT_COMPELE_TX_STATUS,              Timeout,                        INPUT_COMPELE_TX_CANCEL_STATUS   , input_compete_tx_timeout,    },
      {INPUT_COMPELE_TX_STATUS,              Tx_Success,                     WAIT_PEER_CONFIRMATION_STATUS    , NULL,                        },
      {INPUT_COMPELE_TX_STATUS,              Tx_Fail,                        LINK_CLOSE_TX_STATUS             , prov_fsm_tx_fail,            },
      {INPUT_COMPELE_TX_STATUS,              Provisioning_Fail_Rx,           INPUT_COMPELE_TX_CANCEL_STATUS   , input_compete_tx_timeout,    },

      {INPUT_COMPELE_TX_CANCEL_STATUS,       Tx_Timeout_Cancel,              LINK_CLOSE_TX_STATUS             , prov_fsm_timeout_handler,    },
};
static uint8_t oob_publickey_input_auth_fsm_table_size = ARRAY_TABLE_SIZE(oob_publickey_input_auth_fsm_table);
     
static prov_fsm_table_t   auth_and_decrtpy_fsm_table[] = {

      {WAIT_PEER_CONFIRMATION_STATUS,      Timeout,                         LINK_CLOSE_TX_STATUS              , prov_fsm_timeout_handler,      },
      {WAIT_PEER_CONFIRMATION_STATUS,      Provisioning_Confirmation_Rx,    WAIT_CONFIRMATION_ACK_STATUS      , confirm_rx,                    },
      {WAIT_PEER_CONFIRMATION_STATUS,      Provisioning_Fail_Rx,            LINK_CLOSE_TX_STATUS              , prov_fsm_prepare_close_link,   },
      {WAIT_PEER_CONFIRMATION_STATUS,      Provisioning_Capablities_Rx,     WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_CONFIRMATION_STATUS,      Provisioning_Invite_Rx,          WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_CONFIRMATION_STATUS,      Provisioning_Start_Rx,           WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_CONFIRMATION_STATUS,      Provisioning_Public_Key_Rx,      WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_CONFIRMATION_STATUS,      Provisioning_Input_Complete_Rx,  WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_CONFIRMATION_STATUS,      Provisioning_Random_Rx,          WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_CONFIRMATION_STATUS,      Provisioning_Data_Rx,            WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_CONFIRMATION_STATUS,      Provisioning_Complete_Rx,        WAIT_RX_ACK_STATUS                , NULL                  ,        },

      {WAIT_CONFIRMATION_ACK_STATUS,       Ack_Finish,                      CONFIRM_CALC_STATUS               , calc_confirmation,             },

      {CONFIRM_CALC_STATUS,                Confirmation_Calc_Done,          CONFIRMATION_TX_STATUS            , calc_confirmation_finish,      },

      {CONFIRMATION_TX_STATUS,             Timeout,                         CONFIRMATION_TX_CANCEL_STATUS     , confirmation_tx_timeout,       },
      {CONFIRMATION_TX_STATUS,             Tx_Success,                      WAIT_PEER_RANDOM_STATUS           , NULL,                          },
      {CONFIRMATION_TX_STATUS,             Tx_Fail,                         LINK_CLOSE_TX_STATUS              , prov_fsm_tx_fail,              },
      {CONFIRMATION_TX_STATUS,             Provisioning_Fail_Rx,            CONFIRMATION_TX_CANCEL_STATUS     , confirmation_tx_timeout,       },

      {CONFIRMATION_TX_CANCEL_STATUS,      Tx_Timeout_Cancel,               LINK_CLOSE_TX_STATUS              , prov_fsm_timeout_handler,      },

      {WAIT_PEER_RANDOM_STATUS,            Timeout,                         LINK_CLOSE_TX_STATUS              , prov_fsm_timeout_handler,      },
      {WAIT_PEER_RANDOM_STATUS,            Provisioning_Random_Rx,          WAIT_RANDOM_ACK_STATUS            , random_rx,                     },
      {WAIT_PEER_RANDOM_STATUS,            Provisioning_Fail_Rx,            LINK_CLOSE_TX_STATUS              , prov_fsm_prepare_close_link,   },
      {WAIT_PEER_RANDOM_STATUS,            Provisioning_Capablities_Rx,     WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_RANDOM_STATUS,            Provisioning_Invite_Rx,          WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_RANDOM_STATUS,            Provisioning_Start_Rx,           WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_RANDOM_STATUS,            Provisioning_Public_Key_Rx,      WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_RANDOM_STATUS,            Provisioning_Input_Complete_Rx,  WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_RANDOM_STATUS,            Provisioning_Confirmation_Rx,    WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_RANDOM_STATUS,            Provisioning_Data_Rx,            WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_PEER_RANDOM_STATUS,            Provisioning_Complete_Rx,        WAIT_RX_ACK_STATUS                , NULL                  ,        },

      {WAIT_RANDOM_ACK_STATUS,              Ack_Finish,                     CONFIRMATION_CHECK_STATUS         , confirmation_check,            },

      {CONFIRMATION_CHECK_STATUS,          Confirmation_Check_Pass,         RANDOM_TX_STATUS                  , confirmation_check_done,       },
      {CONFIRMATION_CHECK_STATUS,          Confirmation_Check_Fail,         FAIL_PDU_TX_STATUS                , confirmation_check_fail,       },

      {RANDOM_TX_STATUS,                   Timeout,                         RANDOM_TX_CANCEL_STATUS           , random_tx_timeout,             },
      {RANDOM_TX_STATUS,                   Tx_Success,                      WAITING_PROVISION_DATA_STATUS     , NULL,                          },
      {RANDOM_TX_STATUS,                   Tx_Fail,                         LINK_CLOSE_TX_STATUS              , prov_fsm_tx_fail,              },
      {RANDOM_TX_STATUS,                   Provisioning_Fail_Rx,            RANDOM_TX_CANCEL_STATUS           , random_tx_timeout,             },

      {RANDOM_TX_CANCEL_STATUS,            Tx_Timeout_Cancel,               LINK_CLOSE_TX_STATUS              , prov_fsm_timeout_handler,      },

      {WAITING_PROVISION_DATA_STATUS,      Timeout,                         LINK_CLOSE_TX_STATUS              , prov_fsm_timeout_handler,      },
      {WAITING_PROVISION_DATA_STATUS,      Provisioning_Data_Rx,            WAIT_PROVISIONING_DATA_ACK_STATUS , prov_data_rx           ,       },
      {WAITING_PROVISION_DATA_STATUS,      Provisioning_Fail_Rx,            LINK_CLOSE_TX_STATUS              , prov_fsm_prepare_close_link,   },
      {WAITING_PROVISION_DATA_STATUS,      Provisioning_Capablities_Rx,     WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAITING_PROVISION_DATA_STATUS,      Provisioning_Invite_Rx,          WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAITING_PROVISION_DATA_STATUS,      Provisioning_Start_Rx,           WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAITING_PROVISION_DATA_STATUS,      Provisioning_Public_Key_Rx,      WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAITING_PROVISION_DATA_STATUS,      Provisioning_Input_Complete_Rx,  WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAITING_PROVISION_DATA_STATUS,      Provisioning_Confirmation_Rx,    WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAITING_PROVISION_DATA_STATUS,      Provisioning_Random_Rx,          WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAITING_PROVISION_DATA_STATUS,      Provisioning_Complete_Rx,        WAIT_RX_ACK_STATUS                , NULL                  ,        },

      {WAIT_PROVISIONING_DATA_ACK_STATUS,    Ack_Finish,                    CALC_SESSION_STATUS               , session_calc,                  },

      {CALC_SESSION_STATUS,                Timeout,                         CALC_SESSION_CANCEL_STATUS        , NULL,                          },
      {CALC_SESSION_STATUS,                Session_Key_Calc_Done,           DECRY_PROVISION_DATA_STATUS       , session_calc_finish,           },

      {CALC_SESSION_CANCEL_STATUS,         Session_Key_Calc_Done,           LINK_CLOSE_TX_STATUS              , prov_fsm_timeout_handler,      },

      {DECRY_PROVISION_DATA_STATUS,        Decrypt_Success,                 COMPLETE_TX_STATUS                , decrypt_data_done,             },
      {DECRY_PROVISION_DATA_STATUS,        Decrypt_Fail,                    FAIL_PDU_TX_STATUS                , data_decrypted_fail,           },

      {COMPLETE_TX_STATUS,                 Timeout,                         COMPLETE_TX_CANCEL_STATUS         , complete_tx_timeout,           },
      {COMPLETE_TX_STATUS,                 Tx_Success,                      PROVISION_DONE_STATUS             , complete_tx_finish,            },
      {COMPLETE_TX_STATUS,                 Tx_Fail,                         LINK_CLOSE_TX_STATUS              , prov_fsm_tx_fail,              },

      {COMPLETE_TX_CANCEL_STATUS,          Tx_Timeout_Cancel,               LINK_CLOSE_TX_STATUS              , prov_fsm_timeout_handler,      },
      {COMPLETE_TX_CANCEL_STATUS,          Tx_Normal_Cancel,                PROVISION_DONE_STATUS             , complete_tx_finish,            },

      {WAIT_ACK_CANCEL_STATUS,             Tx_Normal_Cancel,                IDLE_STATUS                       , prov_server_changeto_idle,     },
      {WAIT_TX_CANCEL_STATUS,              Tx_Normal_Cancel,                IDLE_STATUS                       , prov_server_changeto_idle,     },

      {WAIT_ASYNC_CALC_STATUS,             Confirmation_Calc_Done,          IDLE_STATUS                       , prov_server_changeto_idle,     },
      {WAIT_ASYNC_CALC_STATUS,             Confirmation_Check_Pass,         IDLE_STATUS                       , prov_server_changeto_idle,     },
      {WAIT_ASYNC_CALC_STATUS,             Confirmation_Check_Fail,         IDLE_STATUS                       , prov_server_changeto_idle,     },
      {WAIT_ASYNC_CALC_STATUS,             Session_Key_Calc_Done,           IDLE_STATUS                       , prov_server_changeto_idle,     },
      {WAIT_ASYNC_CALC_STATUS,             Decrypt_Success,                 IDLE_STATUS                       , prov_server_changeto_idle,     },
      {WAIT_ASYNC_CALC_STATUS,             Decrypt_Fail,                    IDLE_STATUS                       , prov_server_changeto_idle,     },

      {FAIL_PDU_TX_STATUS,                 Timeout,                         LINK_CLOSE_TX_STATUS              , prov_fsm_timeout_handler,      },
      {FAIL_PDU_TX_STATUS,                 Tx_Success,                      WAIT_CLOSE_LINK_STATUS            , NULL                 ,         },
      {FAIL_PDU_TX_STATUS,                 Tx_Fail,                         WAIT_CLOSE_LINK_STATUS            , NULL            ,              },

      {WAIT_CLOSE_LINK_STATUS,             Timeout,                         LINK_CLOSE_TX_STATUS              , prov_fsm_timeout_handler,      },
      {WAIT_CLOSE_LINK_STATUS,             Provisioning_Capablities_Rx,     WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_CLOSE_LINK_STATUS,             Provisioning_Invite_Rx,          WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_CLOSE_LINK_STATUS,             Provisioning_Start_Rx,           WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_CLOSE_LINK_STATUS,             Provisioning_Public_Key_Rx,      WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_CLOSE_LINK_STATUS,             Provisioning_Input_Complete_Rx,  WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_CLOSE_LINK_STATUS,             Provisioning_Confirmation_Rx,    WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_CLOSE_LINK_STATUS,             Provisioning_Random_Rx,          WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_CLOSE_LINK_STATUS,             Provisioning_Data_Rx,            WAIT_RX_ACK_STATUS                , NULL                  ,        },
      {WAIT_CLOSE_LINK_STATUS,             Provisioning_Complete_Rx,        WAIT_RX_ACK_STATUS                , NULL                  ,        },

      {WAIT_RX_ACK_STATUS,                 Ack_Finish,                      FAIL_PDU_TX_STATUS                , unexpect_pdu_check_fail,       },


      {LINK_CLOSE_TX_STATUS,               Link_Close_Tx_Finish,            IDLE_STATUS                       , prov_server_changeto_idle,     },

      {PROVISION_DONE_STATUS,              Link_Close_Tx_Finish,            IDLE_STATUS                       , prov_server_provision_finish,  },

};
static uint8_t auth_and_decrtpy_fsm_table_size = ARRAY_TABLE_SIZE(auth_and_decrtpy_fsm_table);

void prov_server_get_handler_table(prov_fsm_tcb_t *fsm_tcb, prov_fsm_table_t **ppFsm_table, uint8_t *p_table_size)
{
    prov_fsm_state_t  fsm_state = fsm_tcb->state;
    uint8_t choose_oob_publickey;
    uint8_t choose_oob_auth;
    prov_server_fsm_tcb_t *tcb = GET_SERVER_TCB_PTR(fsm_tcb);

    if(fsm_state < CAPABILITIES_EXCHANGE_FINISH_STATUS)
    {
        *ppFsm_table = capabalities_change_fsm_table;
        *p_table_size = capabalities_change_fsm_table_size;
    }
    else if(fsm_state >= WAIT_PEER_CONFIRMATION_STATUS)
    {
        *ppFsm_table = auth_and_decrtpy_fsm_table;
        *p_table_size = auth_and_decrtpy_fsm_table_size;
    }
    else
    {
        choose_oob_publickey = tcb->select_capabilites.public_key;
        choose_oob_auth      = tcb->select_capabilites.auth_method;
        if(choose_oob_publickey == Using_OOB_Publickey && (choose_oob_auth == No_OOB_Authentication|| choose_oob_auth == Static_OOB_Authentication))
        {
            *ppFsm_table = oob_publickey_no_auth_fsm_table;
            *p_table_size = oob_publickey_no_auth_fsm_table_size;
        }
        else  if(choose_oob_publickey == Using_OOB_Publickey && choose_oob_auth == Output_OOB_Authentication)
        {
            *ppFsm_table = oob_publickey_out_auth_fsm_table;
            *p_table_size = oob_publickey_out_auth_fsm_table_size;
        }
        else  if(choose_oob_publickey == Using_OOB_Publickey && choose_oob_auth == Input_OOB_Authentication)
        {
            *ppFsm_table = oob_publickey_input_auth_fsm_table;
            *p_table_size = oob_publickey_input_auth_fsm_table_size;
        }
        else  if(choose_oob_publickey == No_OOB_Publickey && (choose_oob_auth == No_OOB_Authentication || choose_oob_auth == Static_OOB_Authentication))
        {
            *ppFsm_table = no_oob_fsm_table;
            *p_table_size = no_oob_fsm_table_size;
        }
        else  if(choose_oob_publickey == No_OOB_Publickey && choose_oob_auth == Output_OOB_Authentication)
        {
            *ppFsm_table = no_oob_out_auth_table;
            *p_table_size = no_oob_out_auth_table_size;
        }
        else  if(choose_oob_publickey == No_OOB_Publickey && choose_oob_auth == Input_OOB_Authentication)
        {
            *ppFsm_table = no_oob_input_auth_table;
            *p_table_size = no_oob_input_auth_table_size;
        }
    }

}

void prov_server_update(mesh_prov_server_close_evt_t evt)
{
    static volatile mem_free_mask_t l_free_mask = MEM_FREE_MASK_INVALID;
    LOG(LOG_LVL_INFO,"prov_server_update %d \n", evt);

    taskENTER_CRITICAL();

    switch(evt)
    {
    case BLE_MESH_CLOSE_EVT_PROVISIONING_DONE :
    {
        l_free_mask |= MEM_FREE_MASK_PROVISION;
    }
    break;
    case BLE_MESH_CLOSE_EVT_KEY_DONE :
        l_free_mask |= MEM_FREE_MASK_KEY;
        break;
    default:
        l_free_mask = MEM_FREE_MASK_INVALID;
        break;
    }

    //check evt is all ready
    if((l_free_mask&MEM_FREE_MASK) == MEM_FREE_MASK)
    {
         taskEXIT_CRITICAL();
         mesh_free(g_tcb);
         g_tcb = NULL;
         LOG(LOG_LVL_INFO,"free_instance\n");
    }
    else
    {
        taskEXIT_CRITICAL();
    }

}


void prov_server_free_instance(prov_fsm_tcb_t *tcb)
{
     static uint8_t test_flag = 0;

     if(test_flag == 1)
     {
        BX_ASSERT(test_flag == 0);
     }
     test_flag = 1;
     mesh_timer_delete(tcb->state_timeout_timer);
     
    // mesh_free(tcb);
    // g_tcb = NULL;
     prov_server_update(BLE_MESH_CLOSE_EVT_PROVISIONING_DONE);
     LOG(LOG_LVL_INFO,"prov_server_free_instance\n");

}


prov_fsm_tcb_t * prov_serv_create_instance(void)
{
    prov_server_fsm_tcb_t * prov_fsm_instance = (prov_server_fsm_tcb_t *)mesh_alloc(sizeof(prov_server_fsm_tcb_t));
    BX_ASSERT(prov_fsm_instance);
    if(NULL == prov_fsm_instance)
    {
        return NULL;
    }
    memset(prov_fsm_instance,0,sizeof(prov_server_fsm_tcb_t));
    prov_fsm_instance->fsm_tcb.role = MESH_ROLE_UNPROV_DEVICE;
    g_tcb = prov_fsm_instance;
    return (prov_fsm_tcb_t *)prov_fsm_instance;
}



void * prov_serv_create_adv_protocol_instance(    void * cookie, prov_fsm_pkt_send pkt_send,prov_fsm_compare item_compare,prov_fsm_pkt_cancel cancel_tx)
{
    if(g_tcb == NULL)
    {
        return NULL;
    }
    prov_fsm_tcb_t *tcb = &g_tcb->fsm_tcb;
    if(tcb->used == 1)
    {
        LOG(LOG_LVL_INFO,"repeat create by adv !!\n");
        return NULL;
    }

    tcb->used = 1;
    tcb->type = PROV_SERV_ADV_TYPE;
    tcb->cookie = cookie;
    tcb->pkt_send = pkt_send;
    tcb->item_compare = item_compare;
    tcb->cancel_tx = cancel_tx;
    tcb->pre_close_func = (prov_fsm_prepare_link_close)provisioning_link_close_tx;
    tcb->close_func = (prov_fsm_link_close)provisioning_tx_pdu_buf_release;
    return (void *)g_tcb;

}

void * prov_serv_get_adv_protocol_instance(void * item)
{
    if(g_tcb == NULL)
        return NULL;

    prov_fsm_tcb_t *tcb = &g_tcb->fsm_tcb;
    if(tcb->item_compare == NULL)
    {
     return NULL;
    }
    if(!tcb->item_compare(tcb->cookie,item))
    {
     return tcb;
    }
    return NULL;
}

void * prov_serv_create_gatt_protocol_instance(    void * cookie, prov_fsm_pkt_send pkt_send,prov_fsm_compare item_compare,prov_fsm_pkt_cancel cancel_tx)
{
    if(g_tcb == NULL)
    {
        return NULL;
    }
    prov_fsm_tcb_t *tcb = &g_tcb->fsm_tcb;

    if(tcb->used == 1)
    {
        LOG(LOG_LVL_INFO,"repeat create by gatt !!\n");
        return NULL;
    }
    tcb->used = 1;
    tcb->type = PROV_SERV_GATT_TYPE;
    tcb->cookie = cookie;
    tcb->pkt_send = pkt_send;
    tcb->item_compare = item_compare;
    tcb->cancel_tx = cancel_tx;
    tcb->pre_close_func = provision_gatt_prepare_link_close;
    tcb->close_func = NULL;
    return g_tcb;
}

void prov_serv_free_gatt_protocol_instance(    void * cookie)
{
     if(g_tcb == NULL)
     {
         return;
     }
     prov_fsm_tcb_t *tcb = &g_tcb->fsm_tcb;

     if(!tcb->item_compare(tcb->cookie,cookie))
     {
        prov_fsm_timeout_handle(tcb);
     }
}

void prov_serv_close_protocol_instance(    void * item,uint8_t reason)
{
     if(g_tcb == NULL)
     {
         return;
     }
     prov_fsm_tcb_t *tcb = &g_tcb->fsm_tcb;
     prov_fsm_params_t param;
     param.close_param.error = (Provisioning_Close_Codes)reason;
     if(tcb->cookie == NULL)
     {
         return ;
     }
     if(!tcb->item_compare(tcb->cookie,item))
     {
         prov_fsm_evt_handle(tcb, Link_Close_Evt, &param);
     }
}


void prov_serv_gatt_close_protocol_instance(void * cookie)
{
     if(g_tcb == NULL)
     {
         return;
     }
     prov_fsm_tcb_t *tcb = &g_tcb->fsm_tcb;

     if(tcb->cookie == NULL || tcb->item_compare == NULL || tcb->type != PROV_SERV_GATT_TYPE)
     {
         return ;
     }
     if(!tcb->item_compare(tcb->cookie,cookie))
     {
         if(tcb->state == LINK_CLOSE_TX_STATUS)
         {

              prov_fsm_evt_handle(tcb, Link_Close_Tx_Finish, NULL);
         }
         else
         {
              prov_fsm_evt_handle(tcb, Link_Close_Evt, NULL);
         }
     }
}



void * prov_serv_get_gatt_protocol_instance(void * item)
{
     prov_fsm_tcb_t *tcb = &g_tcb->fsm_tcb;
     if(tcb->item_compare == NULL)
     {
         return NULL;
     }
     if(!tcb->item_compare(tcb->cookie,item))
     {
         return g_tcb;
     }
     return NULL;
}


