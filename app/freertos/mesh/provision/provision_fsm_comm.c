/**
 ****************************************************************************************
 *
 * @file   provision_fsm_comm.c
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-27 16:55
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

#include "provision_fsm_comm.h"
#include "unprov_device_fsm.h"
#include "provisioner_fsm.h"
#include "provision_api.h"
#include "mesh_queued_msg.h"
#include "unprov_device_fsm.h"
#include "mesh_env.h" 
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#define  MESH_PROV_STAGE_TIMEOUT_MS  60000

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

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static void prov_fsm_tx_callback(prov_fsm_tcb_t *tcb, Tx_Reason reason);
static void prov_fsm_close_callback(prov_fsm_tcb_t *tcb, Tx_Reason reason);

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


static prov_fsm_evt_t  prov_fsm_pkt_to_evt[] = 
{
    [Provisioning_Invite]           =  Provisioning_Invite_Rx,
    [Provisioning_Capabilities]     =  Provisioning_Capablities_Rx,
    [Provisioning_Start]            =  Provisioning_Start_Rx,
    [Provisioning_Public_Key]       =  Provisioning_Public_Key_Rx,
    [Provisioning_Input_Complete]   =  Provisioning_Input_Complete_Rx,
    [Provisioning_Confirmation]     =  Provisioning_Confirmation_Rx,
    [Provisioning_Random]           =  Provisioning_Random_Rx,
    [Provisioning_Data]             =  Provisioning_Data_Rx,
    [Provisioning_Complete]         =  Provisioning_Complete_Rx,
    [Provisioning_Failed]           =  Provision_Fail,
};


static void dummy_fsm_handle (prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    LOG(LOG_LVL_INFO,"!!!!!!!!!ERROR   dummy_fsm_handle!!!!!!!!!!\n");
}

/*********

  provisioner send link close after receive complete pdu, but the ack of complete not recieve ,so 
  we meed to tolerate the scence!!!!!!!!!!!!!!!!!!!!!!!!
  not recommend to do !!!!!
**********/
static bool prov_is_complete_tx_status(prov_fsm_state_t state)
{
    switch(state)
    {
        case COMPLETE_TX_STATUS:
            return true;
        default:
            return false;
     }
}

static bool prov_is_complete_ack_status(prov_fsm_state_t state)
{
    switch(state)
    {
        case WAIT_COMPLETE_ACK_STATUS:
            return true;
        default:
            return false;
     }
}


static bool prov_is_tx_status(prov_fsm_state_t state)
{
    switch(state)
    {
        case INVITE_TX_STATUS:
        case CAPABILITIES_TX_STATUS:
        case START_TX_STATUS:
        case PUBLIC_KEY_TX_STATUS:
        case CONFIRMATION_TX_STATUS:
        case RANDOM_TX_STATUS:
        case INPUT_COMPELE_TX_STATUS:
        case PROVISIONING_DATA_TX_STATUS:
        case FAIL_PDU_TX_STATUS:
            return true;
         default:
             return false;
     }
}

static bool prov_is_ack_tx_status(prov_fsm_state_t state)
{
    switch(state)
    {
       case  WAIT_INVITE_ACK_STATUS:
        case WAIT_CAPABILITIES_ACK_STATUS:
        case WAIT_START_ACK_STATUS:
        case WAIT_PUBLIC_KEY_ACK_STATUS:
        case WAIT_CONFIRMATION_ACK_STATUS:
        case WAIT_INPUT_COMPELE_ACK_STATUS:
        case WAIT_RANDOM_ACK_STATUS:
        case WAIT_PROVISIONING_DATA_ACK_STATUS:
        case WAIT_COMPLETE_ACK_STATUS:
        case COMPLETE_TX_STATUS:
        case WAIT_RX_ACK_STATUS:
            return true;
         default:
             return false;
     }
}

static bool prov_is_async_calc_status(prov_fsm_state_t state)
{
    switch(state)
    {
        case CONFIRM_CALC_STATUS:
        case CONFIRMATION_CHECK_STATUS:
        case CALC_SESSION_STATUS:
        case CALC_SESSION_CANCEL_STATUS:
        case DECRY_PROVISION_DATA_STATUS:
        case ENCRYPT_PROVISIONING_DATA_STATUS:
        case ENCRYPT_PROVISIONING_CANCEL_STATUS:
        case UNPROV_DEVKEY_CALC_STATUS:
            return true;
         default:
             return false;
     }
}

static bool prov_is_normal_status(prov_fsm_state_t state)
{
    switch(state)
    {
        case WAIT_INPUT_START_STATUS:
        case WAIT_START_STATUS:
        case CAPABILITIES_EXCHANGE_FINISH_STATUS:
        case WAIT_PEER_PUBLIC_KEY_STATUS:
        case WAIT_OOB_PUBLICKEY_STATUS:
        case WAIT_AUTH_INFO_STATUS:
        case WAIT_PEER_CONFIRMATION_STATUS:
        case WAIT_PEER_RANDOM_STATUS:
        case WAITING_PROVISION_DATA_STATUS:
        case WAIT_PEER_COMPLETE_STATUS:        
        case WAIT_CLOSE_LINK_STATUS:
        case START_CHECK_STATUS:
        case IDLE_STATUS:
            return true;
         default:
             return false;
     }
}

static bool prov_is_prov_done_status(prov_fsm_state_t state)
{
    switch(state)
    {
        case PROVISION_DONE_STATUS:
            return true;
         default:
             return false;
     }
}

static bool prov_is_cancel_status(prov_fsm_state_t state)
{
    switch(state)
    {
        case CAPABILITIES_TX_CANCEL_STATUS:
        case START_TX_CANCEL_STATUS:
        case PUBLIC_KEY_TX_CANCEL_STATUS:
        case INPUT_COMPELE_TX_CANCEL_STATUS:
        case CONFIRMATION_TX_CANCEL_STATUS:
        case RANDOM_TX_CANCEL_STATUS:
        case ENCRYPT_PROVISIONING_CANCEL_STATUS:
        case PROVISIONING_DATA_TX_CANCEL_STATUS:
        case COMPLETE_TX_CANCEL_STATUS:
            return true;
         default:
             return false;
     }
}
void * prov_get_cookie(void * env)
{
     if(NULL == env)
     {
          return NULL;
     }
     prov_fsm_tcb_t *tcb = (prov_fsm_tcb_t *) env;
     return tcb->cookie;
}

void prov_fsm_tx_cancel(prov_fsm_tcb_t *tcb)
{
    LOG(LOG_LVL_INFO,"prov_serv_tx_cancel\n");
    tcb->cancel_tx(tcb->cookie,(void (*)(void *tcb, Tx_Reason reason))prov_fsm_tx_callback);
}

void prov_fsm_tx_pkt(prov_fsm_tcb_t *tcb, uint8_t *data, uint8_t length)
{
     tcb->pkt_send(tcb->cookie,data,length,(void (*)(void *tcb, Tx_Reason reason))prov_fsm_tx_callback);
}

void prov_fsm_prepare_close_link (prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
     if(fsm_tcb->pre_close_func)
     {
         fsm_tcb->pre_close_func(fsm_tcb->cookie,param->close_param.error,(void (*)(void *, Tx_Reason))prov_fsm_close_callback);
     }
//     fsm_tcb->state = WAIT_CLOSE_ACK_STATUS;
}

void prov_fsm_ack_tx_cancle (prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    LOG(LOG_LVL_INFO,"prov_fsm_ack_tx_cancle\n");
    prov_fsm_tx_cancel(fsm_tcb);
}



void prov_fsm_tx_fail (prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_fsm_prepare_close_link(fsm_tcb,param);
}

/*
static prov_fsm_state_t prov_fsm_get_current(prov_fsm_tcb_t *tcb)
{
    return tcb->state;
}
*/
void * prov_fsm_get_cookie(void * env)
{
     prov_fsm_tcb_t*  tcb  = (prov_fsm_tcb_t *)env;
     return tcb->cookie;
}

static prov_fsm_handler_t  prov_fsm_find_handler_in_table(prov_fsm_state_t state, prov_fsm_evt_t evt, prov_fsm_table_t *fsm_table, uint8_t table_size)
{
    uint8_t index = 0;
    for(index = 0; index < table_size; index++)
    {
        if(fsm_table[index].state == state && fsm_table[index].evt == evt)
        {
            return fsm_table[index].fsm_handler;
        }
    }
    return dummy_fsm_handle;
}

static prov_fsm_state_t  prov_fsm_getnext_status(prov_fsm_state_t state, prov_fsm_evt_t evt, prov_fsm_table_t *fsm_table, uint8_t table_size)
{
    uint8_t index = 0;
    for(index = 0; index < table_size; index++)
    {
        if(fsm_table[index].state == state && fsm_table[index].evt == evt)
        {
            return fsm_table[index].next_state;
        }
    }
    return INVALID_STATUS;
}


void prov_fsm_set_stage_timeout(prov_fsm_tcb_t *tcb)
{
     tcb->timeout_flag = true;
}

static void prov_fsm_reset_timer(prov_fsm_tcb_t *tcb)
{
     mesh_timer_reset(tcb->state_timeout_timer);
}

static void prov_fsm_set_nextstate(prov_fsm_tcb_t *tcb,prov_fsm_state_t next_state)
{
    tcb->state = next_state;
    prov_fsm_reset_timer(tcb);
    LOG(LOG_LVL_INFO,"set_nextstate = ");
    provision_print_state(next_state);

}


static void prov_fsm_get_handler_table(prov_fsm_tcb_t *tcb, prov_fsm_table_t **ppFsm_table, uint8_t *p_table_size)
{
     if(MESH_ROLE_UNPROV_DEVICE ==  tcb->role)
     {
    
          prov_server_get_handler_table(tcb,ppFsm_table,p_table_size);
     }
     else
     {
         #if MESH_PROVISION_CLIENT_SUPPORT
         prov_client_get_handler_table(tcb,ppFsm_table,p_table_size);
         #endif
     }
     return;
}

void  prov_fsm_evt_handle_link_close(prov_fsm_tcb_t *tcb, prov_fsm_params_t *param)
{
    LOG(LOG_LVL_INFO,"!!!!!!!prov_fsm_evt_handle_link_close!!!!!!!!\n");
    LOG(LOG_LVL_INFO,"current state = ");
    provision_print_state(tcb->state);

    if(prov_is_complete_tx_status(tcb->state))
    {
         prov_fsm_ack_tx_cancle(tcb,param);
         if(param->close_param.error == 0)
         {
             prov_fsm_set_nextstate(tcb,COMPLETE_TX_CANCEL_STATUS); // We succeeded!!!!
         }
         else
         {
             prov_fsm_set_nextstate(tcb,WAIT_TX_CANCEL_STATUS);
         }
    }
    else if(prov_is_complete_ack_status(tcb->state))
    {
         if(param->close_param.error == 0)
         {
             prov_fsm_set_nextstate(tcb,WAIT_ACK_CANCEL_STATUS); // We succeeded!!!!
         }
         else
         {
             prov_fsm_set_nextstate(tcb,WAIT_ASYNC_CALC_STATUS);
         }
    }
    else if(prov_is_tx_status(tcb->state) || prov_is_ack_tx_status(tcb->state))
    {
         prov_fsm_ack_tx_cancle(tcb,param);
         prov_fsm_set_nextstate(tcb,WAIT_TX_CANCEL_STATUS);
    }
    else if(prov_is_normal_status(tcb->state))
    {
        prov_server_changeto_idle(tcb,NULL);
    }
    else if(prov_is_async_calc_status(tcb->state))
    {
        prov_fsm_set_nextstate(tcb,WAIT_ASYNC_CALC_STATUS);
    } else if (prov_is_cancel_status(tcb->state))
    {
        prov_fsm_set_nextstate(tcb,WAIT_TX_CANCEL_STATUS);
    }else if (prov_is_prov_done_status(tcb->state))
    {
        prov_fsm_set_nextstate(tcb,IDLE_STATUS);
        prov_server_provision_finish(tcb,param);
    }
    else
    {
        BX_ASSERT(0);
    }
}

void prov_fsm_evt_handle(prov_fsm_tcb_t *tcb,prov_fsm_evt_t evt,prov_fsm_params_t *param)
{
    prov_fsm_state_t  fsm_state = tcb->state;
    prov_fsm_table_t *fsm_table = NULL;
    prov_fsm_handler_t  fsm_handler = NULL;
    prov_fsm_state_t  next_state = INVALID_STATUS;
    uint8_t table_size = 0;
    LOG(LOG_LVL_INFO,"evt_handle evt =!!");
    provision_print_evt(evt);
    if(evt == Link_Close_Evt )
    {
        prov_fsm_evt_handle_link_close(tcb,param);
        return;
    }
    prov_fsm_get_handler_table(tcb, &fsm_table,&table_size);
    fsm_handler = prov_fsm_find_handler_in_table(fsm_state,evt,fsm_table,table_size);
    next_state  = prov_fsm_getnext_status(fsm_state,evt,fsm_table,table_size);
    if(next_state != INVALID_STATUS)
    {
        prov_fsm_set_nextstate(tcb,next_state);
    }
    else
    {
        LOG(LOG_LVL_INFO,"invalid next state!!\n");
    }
    if(NULL != fsm_handler)
    {
        fsm_handler(tcb,param);
    }
    else
    {
        LOG(LOG_LVL_INFO,"invlaid fsm handle \n");
    }
}


static prov_fsm_evt_t prov_fsm_get_evt_by_pkt(Provisioning_PDU_Type pkt_type)
{
     if(pkt_type >= Provisioning_PDU_Type_Max)
     {
         return Invalid_Evt;
     }
     return prov_fsm_pkt_to_evt[pkt_type];
}

void provision_fsm_entry(prov_fsm_tcb_t * tcb, uint8_t * pkt, uint16_t length)
{
   Provisioning_PDU_Type  pkt_type = provision_pkt_analysis(pkt,length);
   prov_fsm_params_t param;
   prov_fsm_evt_t evt  = prov_fsm_get_evt_by_pkt(pkt_type);
   if(Invalid_Evt == evt)
   {
        LOG(LOG_LVL_INFO,"invalid pkttype %d!!\n",pkt_type);
        return ;
   }
   param.pkt.data = pkt + 1;
   param.pkt.len  = length - 1;
   prov_fsm_evt_handle(tcb,evt,&param);
}

void prov_fsm_timeout_handle(prov_fsm_tcb_t *tcb)
{
    prov_fsm_params_t param;
    memset(&param,0,sizeof(prov_fsm_params_t));
    param.close_param.error = PROVISIOON_TIMEOUT;
    tcb->timeout_flag = true;
    prov_fsm_evt_handle(tcb,Timeout,&param);
}

static void prov_fsm_timeout_callback(mesh_timer_t timer)
{
    //LOG(3, "prov_fsm_timeout_callback\n");
    prov_fsm_table_t *tcb = mesh_timer_get_associated_data(timer);
    mesh_queued_msg_send((void (*)(void *))prov_fsm_timeout_handle,(void *)tcb);
}


prov_fsm_tcb_t * prov_fsm_create_instance(mesh_provsion_role_t  role)
{
     prov_fsm_tcb_t *prov_fsm_instance = NULL;
     if(MESH_ROLE_UNPROV_DEVICE ==  role)
     {
         prov_fsm_instance = prov_serv_create_instance();
     }
     else
     {
      #if MESH_PROVISION_CLIENT_SUPPORT
         prov_fsm_instance =  prov_client_create_instance();
      #endif
     }
     prov_fsm_instance->state_timeout_timer = mesh_timer_create("prov_fsmTimer",pdMS_TO_TICKS(MESH_PROV_STAGE_TIMEOUT_MS),false,(void *)prov_fsm_instance,prov_fsm_timeout_callback);
     return prov_fsm_instance;
}


void prov_fsm_close_protocol_instance(    prov_fsm_tcb_t *tcb)
{
    if(tcb == NULL)
    {
        return;
    }
    if(NULL !=tcb->close_func)
    {
        tcb->close_func(tcb->cookie,0);
    }
    if(tcb->role == MESH_ROLE_UNPROV_DEVICE)
    {
        if(tcb->state == PROVISION_DONE_STATUS)
        {
            prov_server_free_instance(tcb);
        }
        else
        {
            tcb->used = 0;
            tcb->timeout_flag = 0;
            mesh_timer_stop(tcb->state_timeout_timer);
            tcb->state = IDLE_STATUS;
            tcb->type = 0;
            tcb->cookie = NULL;
            tcb->pkt_send = NULL;
            tcb->item_compare = NULL;
            tcb->cancel_tx = NULL;
            tcb->pre_close_func = NULL;
            tcb->close_func = NULL;
        }
    }
    else
    {
        #if MESH_PROVISION_CLIENT_SUPPORT
        prov_client_free_protocol_instance(tcb);
        #endif
    }
}

static void prov_fsm_close_callback(prov_fsm_tcb_t *tcb, Tx_Reason reason)
{
    BX_ASSERT(tcb);
    LOG(LOG_LVL_INFO,"prov_fsm_close_callback !!\n");
    prov_fsm_params_t param ;
    param.close_param.error =(Provisioning_Close_Codes)reason;
    //prov_fsm_close_protocol_instance(tcb);
    prov_fsm_evt_handle(tcb,Link_Close_Tx_Finish,&param);
}


static void prov_fsm_tx_callback(prov_fsm_tcb_t *tcb, Tx_Reason reason)
{
    prov_fsm_params_t param;
    prov_fsm_evt_t evt ;
    LOG(LOG_LVL_INFO,"prov_fsm_tx_callback !!\n");
    memset(&param,0,sizeof(prov_fsm_params_t));
    if(reason == Tx_Success_Finish)
    {
        evt = Tx_Success;
    }
    else if(reason == Tx_Fail_Finish)
    {
        param.close_param.error = PROVISIOON_FAIL;
        evt = Tx_Fail;
    }
    else if(reason == Tx_Cancel_Finish)
    {
        if(tcb->timeout_flag)
        {
             evt = Tx_Timeout_Cancel;
             param.close_param.error = PROVISIOON_TIMEOUT;
        }
        else
        {
             evt = Tx_Normal_Cancel;
        }
    }
    prov_fsm_evt_handle(tcb,evt,&param);
}

void prov_fsm_ack_finish(prov_fsm_tcb_t *tcb,void * item)
{
    prov_fsm_evt_handle(tcb,Ack_Finish,NULL);
}

void * prov_fsm_get_adv_protocol_instance(void * item)
{
     void * tcb = NULL;
     tcb = prov_serv_get_adv_protocol_instance(item);
     if(NULL == tcb)
     {
          #if MESH_PROVISION_CLIENT_SUPPORT
          tcb = prov_client_get_adv_protocol_instance(item);
          #endif
     }
     return tcb;
}

 void * prov_fsm_create_adv_protocol_instance(void * cookie, prov_fsm_pkt_send pkt_send,prov_fsm_compare item_compare,prov_fsm_pkt_cancel cancel_tx)
{
    return prov_serv_create_adv_protocol_instance( cookie,  pkt_send, item_compare, cancel_tx);
}


void * prov_fsm_create_adv_client_protocol_instance(void * cookie, prov_fsm_pkt_send pkt_send,prov_fsm_compare item_compare,prov_fsm_pkt_cancel cancel_tx)
{
    return prov_client_create_adv_protocol_instance( cookie,  pkt_send, item_compare, cancel_tx);
}



void  prov_fsm_adv_close_link(void * item,uint8_t reason)
{
    #if MESH_PROVISION_SERVER_SUPPORT
    prov_serv_close_protocol_instance(item,reason);
    #endif
    #if MESH_PROVISION_CLIENT_SUPPORT
    prov_client_close_protocol_instance(item,reason);
    #endif
}


void * prov_fsm_get_gatt_protocol_instance(void * item)
{
    return prov_serv_get_gatt_protocol_instance(item);
}

void * prov_fsm_create_gatt_protocol_instance(    void * cookie, prov_fsm_pkt_send pkt_send,prov_fsm_compare item_compare,prov_fsm_pkt_cancel cancel_tx)
{
    return prov_serv_create_gatt_protocol_instance( cookie,  pkt_send, item_compare, cancel_tx);
}

void  prov_fsm_gatt_close_link(void * item)
{
    prov_serv_gatt_close_protocol_instance(item);
}

