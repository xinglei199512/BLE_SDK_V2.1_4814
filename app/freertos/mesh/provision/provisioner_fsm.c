/**
 ****************************************************************************************
 *
 * @file   provisioner_fsm.c
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-16 18:38
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

#include "provisioner_fsm.h"
#include <co_math.h>
#include <co_endian.h>
#include "stack_mem_cfg.h"
#include "log.h"
#include "timer_wrapper.h"
#include "mesh_env.h"
#include "mesh_queued_msg.h"
#include "osapp_utils.h"
#include "provision_fsm_comm.h"
#include "provision_security.h"
#include "provision.h"
#include "provisioner_intf.h"
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
DEF_ARRAY_BUF(prov_client_buf, prov_client_tcb_buff_t, PROV_CLIENT_BUF_SIZE);
provisioner_t m_provisioner;
/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
prov_fsm_tcb_t * prov_client_find_dev(uint8_t  *dev_uuid);

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
provision_data_t * get_provision_resource(void)
{
    return &m_provisioner.provisioner_resources;
}

uint16_t  provisioner_alloc_addr(prov_client_fsm_tcb_t *tcb)
{
    provision_data_t *    resource = get_provision_resource();
    uint16_t addr = resource->unicast_addr;
    resource->unicast_addr += tcb->provision_capabilities.elements_num;
    return addr;
}
 void get_provision_data(prov_client_fsm_tcb_t *tcb,provision_data_t * provision_data)
{
    provision_data_t *    resource = get_provision_resource();
    memcpy(provision_data->network_key, resource->network_key, MESH_NETWORK_KEY_LENGTH);
    provision_data->key_index = resource->key_index;
    provision_data->flags = resource->flags;
    provision_data->current_iv_index = resource->current_iv_index;
    provision_data->unicast_addr = provisioner_alloc_addr(tcb);
    tcb->addr = provision_data->unicast_addr;
}
 
prov_fsm_tcb_t * client_search_dev_tcb(uint8_t* dev_uuid)
{
      return prov_client_find_dev(dev_uuid);
}

static void prov_client_tx_timeout(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    LOG(LOG_LVL_INFO,"prov_client_tx_timeout\n");
    prov_fsm_tx_cancel(fsm_tcb);
    prov_fsm_set_stage_timeout(fsm_tcb);
}
 
void prov_client_changeto_idle(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    LOG(LOG_LVL_INFO,"changeto_idle\n");
    prov_fsm_close_protocol_instance(fsm_tcb);

}

static void invite_tx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    
    prov_client_fsm_tcb_t *tcb =  GET_CLIENT_TCB_PTR(fsm_tcb);
    uint8_t length;
    uint8_t *buf = provisioning_pdu_build(Provisioning_Invite,&length);
    LOG(LOG_LVL_INFO,"invite_tx %d\n",tcb->attention_duration);
    buf[0 + PARAMS_OFFSET_IN_PROVISIONING_PDU] = tcb->attention_duration;
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
    mesh_free(buf);
}

static void capabilities_rx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_client_fsm_tcb_t *tcb =  GET_CLIENT_TCB_PTR(fsm_tcb);
    memcpy(tcb->confirmation_input  + CAPABILITIES_PDU_OFFSET_INCONFIRMATION_INPUTS,param->pkt.data,CAPABILITIES_PDU_PARAMS_LEN);
    tcb->provision_capabilities.elements_num = param->pkt.data[0];
    tcb->provision_capabilities.algorithms = (param->pkt.data[1] << 8) |param->pkt.data[2];
    tcb->provision_capabilities.public_key_type = param->pkt.data[3];
    tcb->provision_capabilities.static_oob_type = param->pkt.data[4];
    tcb->provision_capabilities.output_oob_size = param->pkt.data[5];
    tcb->provision_capabilities.output_oob_action = (param->pkt.data[6] << 8) |param->pkt.data[7];
    tcb->provision_capabilities.input_oob_size = param->pkt.data[8];
    tcb->provision_capabilities.input_oob_action = (param->pkt.data[9] << 8) |param->pkt.data[10];
    if(m_provisioner.evt)// not null
    {
        mesh_prov_evt_param_t param;
        //copy beacon data
        param.prov.param.p_dev_capabilities = &tcb->provision_capabilities;
        memcpy(param.prov.dev_uuid,tcb->dev_uuid,MESH_DEVICE_UUID_LENGTH);
        m_provisioner.evt(PROV_EVT_CAPABILITIES,param);
    }
}

static void wait_start(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    if(m_provisioner.evt)// not null
    {
        prov_client_fsm_tcb_t *tcb =  GET_CLIENT_TCB_PTR(fsm_tcb);
        mesh_prov_evt_param_t param;
        //copy beacon data
        param.prov.param.p_start_pdu = &tcb->start;
        memcpy(param.prov.dev_uuid,tcb->dev_uuid,MESH_DEVICE_UUID_LENGTH);
        m_provisioner.evt(PROV_EVT_REQUEST_START,param);
    }
}

void provisioner_input_start(prov_fsm_tcb_t *fsm_tcb, uint8_t * start_data)
{
    prov_client_fsm_tcb_t *tcb =  GET_CLIENT_TCB_PTR(fsm_tcb);
    memcpy(&tcb->start,start_data,sizeof(tcb->start));
}

static void start_tx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    LOG(LOG_LVL_INFO,"start_tx\n");
    prov_client_fsm_tcb_t *tcb =  GET_CLIENT_TCB_PTR(fsm_tcb);
    uint8_t length;
    uint8_t *buf = provisioning_pdu_build(Provisioning_Start,&length);
    memcpy(buf+PARAMS_OFFSET_IN_PROVISIONING_PDU,&tcb->start,START_PDU_PARAMS_LEN);
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
    mesh_free(buf);
}

static void public_key_tx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    uint8_t length;
    uint8_t *buf = provisioning_pdu_build(Provisioning_Public_Key,&length);
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    memcpy(buf+PARAMS_OFFSET_IN_PROVISIONING_PDU,&tcb->provision_key.local_public,sizeof(public_key_t));
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
    mesh_free(buf);
}

static void public_key_rx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    if(sizeof(public_key_t) != param->pkt.len)
    {
        LOG(LOG_LVL_INFO,"receive invalid len publickey\n");
        return;
    }
    memcpy(&tcb->provision_key.peer_public,param->pkt.data ,param->pkt.len);
}


static void confirmation_input_assemble(prov_fsm_tcb_t *fsm_tcb,uint8_t * data)
{
     prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
     memcpy(data  +INVITE_PDU_OFFSET_IN_CONFIRMATION_INPUTS,&tcb->attention_duration,INVITE_PDU_PARAMS_LEN);
//    memcpy(data  +CAPABILITIES_PDU_OFFSET_INCONFIRMATION_INPUTS,&tcb->provision_capabilities,CAPABILITIES_PDU_PARAMS_LEN);
     memcpy(data + START_PDU_OFFSET_IN_CONFIRMATION_INPUTS,&tcb->start,START_PDU_PARAMS_LEN);
     memcpy(data + DEVICE_PUBLIC_KEY_OFFSET_IN_CONFIRMATION_INPUTS,&tcb->provision_key.peer_public ,PUBLIC_KEY_PDU_PARAMS_LEN);
     memcpy(data + PROVISIONER_PUBLIC_KEY_OFFSET_IN_CONFIRMATION_INPUTS,&tcb->provision_key.local_public,PUBLIC_KEY_PDU_PARAMS_LEN);

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

static void client_calc_confirmation(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_confirmation_calc_t calc_info;
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    calc_info.dummy = fsm_tcb;
    calc_info.prov_callback = prov_calc_confirmation_cb;
    calc_info.prov_authentication = &tcb->provision_authentication;
    calc_info.prov_confirmation   = &tcb->confirmation;
    calc_info.prov_confirmation_input = tcb->confirmation_input;
    confirmation_input_assemble(fsm_tcb,calc_info.prov_confirmation_input);
    calc_info.prov_key  = &tcb->provision_key;
    calc_info.prov_random = &tcb->random;
    LOG(LOG_LVL_INFO,"random data = ");
    log_hex_data(tcb->random.field.random_provisioner, RANDOM_PDU_PARAMS_LEN);
    memcpy(tcb->provision_authentication.msg_for_cfm.detail.random,tcb->random.field.random_provisioner,RANDOM_PDU_PARAMS_LEN);
    provision_calc_confirmation_start(&calc_info);
    LOG(LOG_LVL_INFO,"calc_confirmation\n");
}

static void notify_admin_to_dispaly_auth(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    Auth_Data_Type data_type = provision_get_auth_data_type(tcb->start.auth_method,tcb->start.auth_action);
    provision_generate_auth(tcb->provision_authentication.msg_for_cfm.detail.auth_value,tcb->start.auth_size,data_type);
    if(m_provisioner.evt)// not null
    {
        mesh_prov_evt_param_t param;
        memcpy(param.prov.dev_uuid,tcb->dev_uuid,MESH_DEVICE_UUID_LENGTH);
        param.prov.param.p_output_val = tcb->provision_authentication.msg_for_cfm.detail.auth_value;
        m_provisioner.evt(PROV_EVT_AUTH_DISPLAY_NUMBER,param);
    }
}

void provisioner_input_oob_key(prov_fsm_tcb_t *fsm_tcb, uint8_t * key_data)
{
    prov_client_fsm_tcb_t *tcb =  GET_CLIENT_TCB_PTR(fsm_tcb);
    memcpy(&tcb->provision_key.peer_public,key_data,sizeof(tcb->provision_key.peer_public));
}


static void notity_to_input_oob_key(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    if(m_provisioner.evt)// not null
    {
        prov_client_fsm_tcb_t *tcb =  GET_CLIENT_TCB_PTR(fsm_tcb);
        mesh_prov_evt_param_t param;
        
        param.prov.param.p_public_keys = &tcb->provision_key.peer_public;
        memcpy(param.prov.dev_uuid,tcb->dev_uuid,MESH_DEVICE_UUID_LENGTH);
        m_provisioner.evt(PROV_EVT_READ_PEER_PUBLIC_KEY_OOB,param);
    }
}

void provisioner_input_auth(prov_fsm_tcb_t *fsm_tcb, uint8_t * auth_data)
{
    prov_client_fsm_tcb_t *tcb =  GET_CLIENT_TCB_PTR(fsm_tcb);
    memcpy(tcb->provision_authentication.msg_for_cfm.detail.auth_value,auth_data,AUTHVALUE_LEN);
}


static void notity_to_input_auth(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    if(m_provisioner.evt)// not null
    {
        prov_client_fsm_tcb_t *tcb =  GET_CLIENT_TCB_PTR(fsm_tcb);
        mesh_prov_evt_param_t param;
        
        param.prov.param.p_start_pdu = &tcb->start;
        memcpy(param.prov.dev_uuid,tcb->dev_uuid,MESH_DEVICE_UUID_LENGTH);
        param.prov.param.p_input_val = tcb->provision_authentication.msg_for_cfm.detail.auth_value;
        m_provisioner.evt(PROV_EVT_AUTH_INPUT_NUMBER,param);
    }
}

static void client_confirmation_tx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    uint8_t length;
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    uint8_t *buf = provisioning_pdu_build(Provisioning_Confirmation,&length);
    memcpy(buf+PARAMS_OFFSET_IN_PROVISIONING_PDU,tcb->confirmation.local,CONFIRMATION_PDU_PARAMS_LEN);
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
    LOG(LOG_LVL_INFO,"confirmation_tx\n");
    mesh_free(buf);
}


static prov_fsm_table_t  capabalities_change_fsm_table[] = {
      {IDLE_STATUS,                 Start_Provision,               INVITE_TX_STATUS             , invite_tx,                            },

      {INVITE_TX_STATUS,             Timeout,                      INVITE_TX_CANCEL_STATUS      , prov_client_tx_timeout,               },
      {INVITE_TX_STATUS,             Tx_Success,                   WAIT_CAPABILITIES_STATUS     , NULL,                                 },
      {INVITE_TX_STATUS,             Tx_Fail,                      LINK_CLOSE_TX_STATUS         , prov_fsm_tx_fail,                     },
      {INVITE_TX_STATUS,             Provisioning_Fail_Rx,         INVITE_TX_CANCEL_STATUS      , prov_client_tx_timeout,               },

      {INVITE_TX_CANCEL_STATUS,      Tx_Timeout_Cancel,            LINK_CLOSE_TX_STATUS         , prov_fsm_prepare_close_link,          },

      {WAIT_CAPABILITIES_STATUS,     Timeout,                      LINK_CLOSE_TX_STATUS         , prov_fsm_prepare_close_link,          },
      {WAIT_CAPABILITIES_STATUS,     Provisioning_Capablities_Rx,  WAIT_CAPABILITIES_ACK_STATUS , capabilities_rx,                      },

      {WAIT_CAPABILITIES_ACK_STATUS, Ack_Finish,                   WAIT_INPUT_START_STATUS      , wait_start,                           },

//      {WAIT_INPUT_START_STATUS,      Timeout,                    LINK_CLOSE_TX_STATUS         , prov_fsm_prepare_close_link,          },
      {WAIT_INPUT_START_STATUS,      User_Input_Complete,          START_TX_STATUS              , start_tx,                             },
};

static uint8_t capabalities_change_fsm_table_size = ARRAY_TABLE_SIZE(capabalities_change_fsm_table);

static prov_fsm_table_t no_oob_fsm_table[] = {

      {START_TX_STATUS,                 Timeout,                         START_TX_CANCEL_STATUS         , prov_client_tx_timeout,       },
      {START_TX_STATUS,                 Tx_Success,                      PUBLIC_KEY_TX_STATUS           , public_key_tx,                },
      {START_TX_STATUS,                 Tx_Fail,                         LINK_CLOSE_TX_STATUS           , prov_fsm_tx_fail,             },
      {START_TX_STATUS,                 Provisioning_Fail_Rx,            START_TX_CANCEL_STATUS         ,  prov_client_tx_timeout,      },

      {START_TX_CANCEL_STATUS,          Tx_Timeout_Cancel,               LINK_CLOSE_TX_STATUS           , prov_fsm_prepare_close_link,  },

      {PUBLIC_KEY_TX_STATUS,            Timeout,                         PUBLIC_KEY_TX_CANCEL_STATUS    , prov_client_tx_timeout,       },
      {PUBLIC_KEY_TX_STATUS,            Tx_Success,                      WAIT_PEER_PUBLIC_KEY_STATUS    , NULL,                         },
      {PUBLIC_KEY_TX_STATUS,            Tx_Fail,                         LINK_CLOSE_TX_STATUS           , prov_fsm_tx_fail,             },
      {PUBLIC_KEY_TX_STATUS,            Provisioning_Fail_Rx,            PUBLIC_KEY_TX_CANCEL_STATUS    , prov_client_tx_timeout,       },

      {PUBLIC_KEY_TX_CANCEL_STATUS,     Tx_Timeout_Cancel,               LINK_CLOSE_TX_STATUS           , prov_fsm_prepare_close_link,  },

      {WAIT_PEER_PUBLIC_KEY_STATUS,     Timeout,                         LINK_CLOSE_TX_STATUS           , prov_fsm_prepare_close_link,  },
      {WAIT_PEER_PUBLIC_KEY_STATUS,     Provisioning_Public_Key_Rx,      WAIT_PUBLIC_KEY_ACK_STATUS     , public_key_rx,                },
      {WAIT_PUBLIC_KEY_ACK_STATUS,      Ack_Finish,                      CONFIRM_CALC_STATUS            , client_calc_confirmation,     },
};
static uint8_t no_oob_fsm_table_size = ARRAY_TABLE_SIZE(no_oob_fsm_table);

static prov_fsm_table_t no_oob_out_auth_table[] ={

      {START_TX_STATUS,             Timeout,                             START_TX_CANCEL_STATUS        , prov_client_tx_timeout,        },
      {START_TX_STATUS,             Tx_Success,                          PUBLIC_KEY_TX_STATUS          , public_key_tx,                 },
      {START_TX_STATUS,             Tx_Fail,                             LINK_CLOSE_TX_STATUS          , prov_fsm_tx_fail,              },
      {START_TX_STATUS,             Provisioning_Fail_Rx,                START_TX_CANCEL_STATUS        , prov_client_tx_timeout,        },

      {START_TX_CANCEL_STATUS,      Tx_Timeout_Cancel,                   LINK_CLOSE_TX_STATUS          , prov_fsm_prepare_close_link,   },

      {PUBLIC_KEY_TX_STATUS,        Timeout,                             PUBLIC_KEY_TX_CANCEL_STATUS   , prov_client_tx_timeout,        },
      {PUBLIC_KEY_TX_STATUS,        Tx_Success,                          WAIT_PEER_PUBLIC_KEY_STATUS   , NULL,                          },
      {PUBLIC_KEY_TX_STATUS,        Tx_Fail,                             LINK_CLOSE_TX_STATUS          , prov_fsm_tx_fail,              },
      {PUBLIC_KEY_TX_STATUS,        Provisioning_Fail_Rx,                PUBLIC_KEY_TX_CANCEL_STATUS   , prov_client_tx_timeout,        },

      {PUBLIC_KEY_TX_CANCEL_STATUS, Tx_Timeout_Cancel,                   LINK_CLOSE_TX_STATUS          , prov_fsm_prepare_close_link,   },

      {WAIT_PEER_PUBLIC_KEY_STATUS, Timeout,                             LINK_CLOSE_TX_STATUS          , prov_fsm_prepare_close_link,   },
      {WAIT_PEER_PUBLIC_KEY_STATUS, Provisioning_Public_Key_Rx,          WAIT_PUBLIC_KEY_ACK_STATUS    , public_key_rx,                 },
      {WAIT_PUBLIC_KEY_ACK_STATUS,  Ack_Finish,                          WAIT_AUTH_INFO_STATUS         , notity_to_input_auth,          },


//      {WAIT_AUTH_INFO_STATUS,       Timeout,                           LINK_CLOSE_TX_STATUS          , prov_fsm_prepare_close_link,   },
      {WAIT_AUTH_INFO_STATUS,       User_Input_Complete,                 CONFIRM_CALC_STATUS           , client_calc_confirmation,      },
};
static uint8_t no_oob_out_auth_table_size = ARRAY_TABLE_SIZE(no_oob_out_auth_table);

static prov_fsm_table_t no_oob_input_auth_table[] = {

      {START_TX_STATUS,                Timeout,                         START_TX_CANCEL_STATUS         , prov_client_tx_timeout,        },
      {START_TX_STATUS,                Tx_Success,                      PUBLIC_KEY_TX_STATUS           , public_key_tx,                 },
      {START_TX_STATUS,                Tx_Fail,                         LINK_CLOSE_TX_STATUS           , prov_fsm_tx_fail,              }, 
      {START_TX_STATUS,                Provisioning_Fail_Rx,            START_TX_CANCEL_STATUS         , prov_client_tx_timeout,        },

      {START_TX_CANCEL_STATUS,         Tx_Timeout_Cancel,               LINK_CLOSE_TX_STATUS           , prov_fsm_prepare_close_link,   }, 

      {PUBLIC_KEY_TX_STATUS,           Timeout,                         PUBLIC_KEY_TX_CANCEL_STATUS    , prov_client_tx_timeout,        },
      {PUBLIC_KEY_TX_STATUS,           Tx_Success,                      WAIT_PEER_PUBLIC_KEY_STATUS    , NULL,                          },
      {PUBLIC_KEY_TX_STATUS,           Tx_Fail,                         LINK_CLOSE_TX_STATUS           , prov_fsm_tx_fail,              },
      {PUBLIC_KEY_TX_STATUS,           Provisioning_Fail_Rx,            PUBLIC_KEY_TX_CANCEL_STATUS    , prov_client_tx_timeout,        },

      {PUBLIC_KEY_TX_CANCEL_STATUS,    Tx_Timeout_Cancel,               LINK_CLOSE_TX_STATUS           , prov_fsm_prepare_close_link,   },

      {WAIT_PEER_PUBLIC_KEY_STATUS,    Timeout,                         LINK_CLOSE_TX_STATUS           , prov_fsm_prepare_close_link,   },
      {WAIT_PEER_PUBLIC_KEY_STATUS,    Provisioning_Public_Key_Rx,      WAIT_PUBLIC_KEY_ACK_STATUS     , public_key_rx,                 },
      {WAIT_PUBLIC_KEY_ACK_STATUS,     Ack_Finish,                      WAIT_INPUT_COMPELE_STATUS      , notify_admin_to_dispaly_auth,  },

//      {WAIT_INPUT_COMPELE_STATUS,      Timeout,                       LINK_CLOSE_TX_STATUS           , prov_fsm_prepare_close_link,   },
      {WAIT_INPUT_COMPELE_STATUS,      Provisioning_Input_Complete_Rx,  WAIT_INPUT_COMPELE_ACK_STATUS  , NULL,                          },

      {WAIT_INPUT_COMPELE_ACK_STATUS,  Ack_Finish,                      CONFIRM_CALC_STATUS            , client_calc_confirmation,      },
};
static uint8_t no_oob_input_auth_table_size = ARRAY_TABLE_SIZE(no_oob_input_auth_table);

static prov_fsm_table_t   oob_publickey_no_auth_fsm_table[] = {

      {START_TX_STATUS,             Timeout,                          START_TX_CANCEL_STATUS           , prov_client_tx_timeout,        }, 
      {START_TX_STATUS,             Tx_Success,                       WAIT_OOB_PUBLICKEY_STATUS        , notity_to_input_oob_key,       },
      {START_TX_STATUS,             Tx_Fail,                          LINK_CLOSE_TX_STATUS             , prov_fsm_tx_fail,              },
      {START_TX_STATUS,             Provisioning_Fail_Rx,             START_TX_CANCEL_STATUS           , prov_client_tx_timeout,        },

      {START_TX_CANCEL_STATUS,      Tx_Timeout_Cancel,                LINK_CLOSE_TX_STATUS             , prov_fsm_prepare_close_link,   },

//      {WAIT_OOB_PUBLICKEY_STATUS,   Timeout,                        LINK_CLOSE_TX_STATUS             , prov_fsm_prepare_close_link,   },
      {WAIT_OOB_PUBLICKEY_STATUS,   User_Input_Complete,              PUBLIC_KEY_TX_STATUS             , public_key_tx,                 },

      {PUBLIC_KEY_TX_STATUS,        Timeout,                          PUBLIC_KEY_TX_CANCEL_STATUS      , prov_client_tx_timeout,        },
      {PUBLIC_KEY_TX_STATUS,        Tx_Success,                       CONFIRM_CALC_STATUS              , client_calc_confirmation,      },
      {PUBLIC_KEY_TX_STATUS,        Tx_Fail,                          LINK_CLOSE_TX_STATUS             , prov_fsm_tx_fail,              },
      {PUBLIC_KEY_TX_STATUS,        Provisioning_Fail_Rx,             PUBLIC_KEY_TX_CANCEL_STATUS      , prov_client_tx_timeout,        },

      {PUBLIC_KEY_TX_CANCEL_STATUS, Tx_Timeout_Cancel,                LINK_CLOSE_TX_STATUS             , prov_fsm_prepare_close_link,   },
};
static uint8_t oob_publickey_no_auth_fsm_table_size = ARRAY_TABLE_SIZE(oob_publickey_no_auth_fsm_table);

static prov_fsm_table_t   oob_publickey_out_auth_fsm_table[] = {

      {START_TX_STATUS,             Timeout,                         START_TX_CANCEL_STATUS            , prov_client_tx_timeout,        },
      {START_TX_STATUS,             Tx_Success,                      WAIT_OOB_PUBLICKEY_STATUS         , notity_to_input_oob_key,       },
      {START_TX_STATUS,             Tx_Fail,                         LINK_CLOSE_TX_STATUS              , prov_fsm_tx_fail,              },
      {START_TX_STATUS,             Provisioning_Fail_Rx,            START_TX_CANCEL_STATUS            , prov_client_tx_timeout,        },

      {START_TX_CANCEL_STATUS,      Tx_Timeout_Cancel,               LINK_CLOSE_TX_STATUS              , prov_fsm_prepare_close_link,   },

//      {WAIT_OOB_PUBLICKEY_STATUS,   Timeout,                       LINK_CLOSE_TX_STATUS              , prov_fsm_prepare_close_link,   },
      {WAIT_OOB_PUBLICKEY_STATUS,   User_Input_Complete,             PUBLIC_KEY_TX_STATUS              , public_key_tx,                 },

      {PUBLIC_KEY_TX_STATUS,        Timeout,                         PUBLIC_KEY_TX_CANCEL_STATUS       , prov_client_tx_timeout,        },
      {PUBLIC_KEY_TX_STATUS,        Tx_Success,                      WAIT_AUTH_INFO_STATUS             , prov_fsm_prepare_close_link,   },
      {PUBLIC_KEY_TX_STATUS,        Tx_Fail,                         LINK_CLOSE_TX_STATUS              , prov_fsm_tx_fail,              },
      {PUBLIC_KEY_TX_STATUS,        Provisioning_Fail_Rx,            PUBLIC_KEY_TX_CANCEL_STATUS       ,  prov_client_tx_timeout,       },

//      {WAIT_AUTH_INFO_STATUS,       Timeout,                       LINK_CLOSE_TX_STATUS              , prov_fsm_prepare_close_link,   },
      {WAIT_AUTH_INFO_STATUS,       User_Input_Complete,             CONFIRM_CALC_STATUS               , client_calc_confirmation,      },
};
static uint8_t oob_publickey_out_auth_fsm_table_size = ARRAY_TABLE_SIZE(oob_publickey_out_auth_fsm_table);


static prov_fsm_table_t   oob_publickey_input_auth_fsm_table[] = {

      {START_TX_STATUS,             Timeout,                        START_TX_CANCEL_STATUS              ,  prov_client_tx_timeout,      },
      {START_TX_STATUS,             Tx_Success,                     WAIT_OOB_PUBLICKEY_STATUS           ,  notity_to_input_oob_key,     },
      {START_TX_STATUS,             Tx_Fail,                        LINK_CLOSE_TX_STATUS                ,  prov_fsm_tx_fail,            }, 
      {START_TX_STATUS,             Provisioning_Fail_Rx,           START_TX_CANCEL_STATUS              ,  prov_client_tx_timeout,      },

      {START_TX_CANCEL_STATUS,      Tx_Timeout_Cancel,              LINK_CLOSE_TX_STATUS                ,  prov_fsm_prepare_close_link, }, 
      {START_TX_CANCEL_STATUS,      Tx_Normal_Cancel,               WAIT_OOB_PUBLICKEY_STATUS           ,  notity_to_input_oob_key,     },

//      {WAIT_OOB_PUBLICKEY_STATUS,   Timeout,                      LINK_CLOSE_TX_STATUS                ,  prov_fsm_prepare_close_link, }, 
      {WAIT_OOB_PUBLICKEY_STATUS,   User_Input_Complete,            PUBLIC_KEY_TX_STATUS                ,  public_key_tx,               },

      {PUBLIC_KEY_TX_STATUS,        Timeout,                        PUBLIC_KEY_TX_CANCEL_STATUS         ,  prov_client_tx_timeout,      },
      {PUBLIC_KEY_TX_STATUS,        Tx_Success,                     WAIT_INPUT_COMPELE_STATUS           ,  notify_admin_to_dispaly_auth,},
      {PUBLIC_KEY_TX_STATUS,        Tx_Fail,                        LINK_CLOSE_TX_STATUS                ,  prov_fsm_tx_fail,            },
      {PUBLIC_KEY_TX_STATUS,        Provisioning_Fail_Rx,           PUBLIC_KEY_TX_CANCEL_STATUS         ,  prov_client_tx_timeout,      },

      {WAIT_INPUT_COMPELE_STATUS,   Timeout,                        LINK_CLOSE_TX_STATUS                ,  prov_fsm_prepare_close_link, },
      {WAIT_INPUT_COMPELE_STATUS,   Provisioning_Input_Complete_Rx, WAIT_INPUT_COMPELE_ACK_STATUS       ,  NULL,                        },
      {WAIT_INPUT_COMPELE_ACK_STATUS,Ack_Finish,                     CONFIRM_CALC_STATUS                ,  client_calc_confirmation,    },
};
static uint8_t oob_publickey_input_auth_fsm_table_size = ARRAY_TABLE_SIZE(oob_publickey_input_auth_fsm_table);

static void client_confirm_rx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    memcpy(tcb->confirmation.peer,param->pkt.data ,CONFIRMATION_PDU_PARAMS_LEN);
    LOG(LOG_LVL_INFO,"confirm_rx\n");
}

static void client_random_tx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    uint8_t length;
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    uint8_t *buf = provisioning_pdu_build(Provisioning_Random,&length);
    memcpy(buf+PARAMS_OFFSET_IN_PROVISIONING_PDU,tcb->random.field.random_provisioner,RANDOM_PDU_PARAMS_LEN);
    LOG(LOG_LVL_INFO,"random tx = ");
    log_hex_data(tcb->random.field.random_provisioner, RANDOM_PDU_PARAMS_LEN);
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
    mesh_free(buf);
}


static void client_random_rx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    if(param->pkt.len != RANDOM_PDU_PARAMS_LEN)
    {
         LOG(LOG_LVL_INFO,"random_rx invalid len\n");
         return;
    }
    LOG(LOG_LVL_INFO,"random_rx =");
    log_hex_data(param->pkt.data, RANDOM_PDU_PARAMS_LEN);
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    memcpy(tcb->random.field.random_device,param->pkt.data,RANDOM_PDU_PARAMS_LEN);
    memcpy(tcb->provision_authentication.msg_for_cfm.detail.random,param->pkt.data,RANDOM_PDU_PARAMS_LEN);
    LOG(LOG_LVL_INFO,"random_rx\n");
}

static void confirmation_check_finish(void * dummy)
{
    uint8_t check_result = 0;
    prov_fsm_params_t param;
    prov_fsm_tcb_t *fsm_tcb = (prov_fsm_tcb_t * )dummy;
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    LOG(LOG_LVL_INFO,"peer data = ");
    log_hex_data(tcb->confirmation.peer, CONFIRMATION_PDU_PARAMS_LEN);
    LOG(LOG_LVL_INFO,"calc data = ");
    log_hex_data(tcb->provision_authentication.confirmation_rslt, CONFIRMATION_PDU_PARAMS_LEN);
    check_result =  memcmp(tcb->confirmation.peer,tcb->provision_authentication.confirmation_rslt
                    ,CONFIRMATION_PDU_PARAMS_LEN)?false:true;
    LOG(LOG_LVL_INFO,check_result ? "confirmation_check:OK\n" : "confirmation_check:FAIL\n");
    if(check_result)
    {
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

static void confirm_check(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_confirmation_check_t check_info;
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    check_info.check_callback = confirmation_check_cb;
    check_info.dummy = fsm_tcb;
    check_info.confirm_key = tcb->provision_authentication.confirmation_key;
    check_info.msg         = tcb->provision_authentication.msg_for_cfm.buf;
    check_info.length      = sizeof(tcb->provision_authentication.msg_for_cfm.buf);
    check_info.rst         = tcb->provision_authentication.confirmation_rslt;
    provision_check_confirmation_start(&check_info);
}

static void session_calc_cb(void *dummy, uint8_t status)
{
    prov_fsm_params_t param;
    prov_fsm_tcb_t *fsm_tcb = (prov_fsm_tcb_t * )dummy;
    prov_fsm_evt_handle(fsm_tcb,Session_Key_Calc_Done,&param);
}

static void client_calc_session(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_session_key_calc_t session_calc_info;
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
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

static void client_confirmation_check_fail(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    prov_client_changeto_idle(fsm_tcb,param);
}

static void data_encrrpt_cb(void *dummy, uint8_t status)
{
    prov_fsm_params_t param;
    prov_fsm_tcb_t *fsm_tcb = (prov_fsm_tcb_t * )dummy;
    prov_fsm_evt_handle(fsm_tcb,Encrypt_Data_done,&param);
}

static void pack_provisioning_data(prov_client_fsm_tcb_t *tcb)
{
    uint8_t *buf = tcb->provision_encrypted_data.packed_data;
    uint8_t len = 0;
    provision_data_t provision_data ;
    get_provision_data(tcb,&provision_data);
    //Network Key
    len = sizeof(provision_data.network_key);
    memcpy(buf,provision_data.network_key,len);
    buf += len;
    //key_index - uint16
    len = sizeof(provision_data.key_index);
    buf[0] = (provision_data.key_index & 0xFF00) >> 8;
    buf[1] = (provision_data.key_index & 0x00FF) >> 0;
    buf += len;
    //flags
    len = sizeof(provision_data.flags);
    memcpy(buf,&provision_data.flags,len);
    buf += len;
    //current_iv_index - uint32
    len = sizeof(provision_data.current_iv_index);
    buf[0] = (provision_data.current_iv_index & 0xFF000000) >> 24;
    buf[1] = (provision_data.current_iv_index & 0x00FF0000) >> 16;
    buf[2] = (provision_data.current_iv_index & 0x0000FF00) >> 8;
    buf[3] = (provision_data.current_iv_index & 0x000000FF) >> 0;
    buf += len;
    //unicast_addr - uint16
    len = sizeof(provision_data.unicast_addr);
    buf[0] = (provision_data.unicast_addr & 0xFF00) >> 8;
    buf[1] = (provision_data.unicast_addr & 0x00FF) >> 0;
    buf += len;    
    LOG(LOG_LVL_INFO,"ProvisioningData = ");
    log_hex_data(tcb->provision_encrypted_data.packed_data,           ENCRYPTED_PROVISIONING_DATA_LEN);

}

static void encrypt_data(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    //calc provision data
    prov_encrypt_data_t enctry_info;
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    pack_provisioning_data(tcb);
    enctry_info.key = tcb->session_info.session_key;
    enctry_info.nonce       = tcb->session_info.session_nonce + 3;
    enctry_info.msg       = tcb->provision_encrypted_data.packed_data;
    enctry_info.msg_length              = ENCRYPTED_PROVISIONING_DATA_LEN;
    enctry_info.mic_length              = PROVISIONING_DATA_MIC_LEN;
    enctry_info.result                  = tcb->provision_encrypted_data.encrypted_data;
    enctry_info.encrypt_callback        = data_encrrpt_cb;
    enctry_info.dummy                   = (void *)fsm_tcb;
    provision_encrypt_data_start(&enctry_info);
}


static void prov_data_tx(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    uint8_t length;
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    uint8_t *buf = provisioning_pdu_build(Provisioning_Data,&length);
    memcpy(buf+PARAMS_OFFSET_IN_PROVISIONING_PDU,tcb->provision_encrypted_data.encrypted_data,DATA_PDU_PARAMS_LEN);
    prov_fsm_tx_pkt(fsm_tcb,buf,length);
}

static void prov_devkey_done(void* dummy)
{
    prov_fsm_tcb_t* fsm_tcb = (prov_fsm_tcb_t *)dummy;
    prov_fsm_params_t param;
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    user_provisioner_dev_key_gen_done(tcb->dev_uuid,tcb->devkey,tcb->addr);
    param.close_param.error = PROVISIOON_SUCCESS;
    prov_fsm_evt_handle(fsm_tcb,Devkey_Calc_done,&param);

}


static void prov_client_devkey_calc_finish(void* dummy, uint8_t result)
{
    prov_fsm_tcb_t* fsm_tcb = (prov_fsm_tcb_t *)dummy;
    mesh_queued_msg_send(prov_devkey_done,fsm_tcb);
    LOG(LOG_LVL_INFO,"prov_client_devkey_calc_finish\n");
}

static void user_provisioner_data_save(prov_fsm_tcb_t *fsm_tcb)
{
    prov_devkey_calc_t calc_info;
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    calc_info.dummy = fsm_tcb;
    calc_info.devkey_callback = prov_client_devkey_calc_finish;
    calc_info.ecdh = tcb->provision_key.ecdh_secret.x;
    calc_info.prov_salt   = tcb->session_info.provisioning_salt;
    calc_info.dev_key = tcb->devkey;
    provision_devkey_calc_start(&calc_info);
    LOG(LOG_LVL_INFO,"user_provisioner_data_save\n");
}


static void client_provison_done(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param)
{
    user_provisioner_data_save(fsm_tcb);
    
//    prov_fsm_prepare_close_link(fsm_tcb,param);
}

static prov_fsm_table_t   auth_and_decrtpy_fsm_table[] = {

      {CONFIRM_CALC_STATUS,                Confirmation_Calc_Done,        CONFIRMATION_TX_STATUS             , client_confirmation_tx,        },

      {CONFIRMATION_TX_STATUS,             Timeout,                       CONFIRMATION_TX_CANCEL_STATUS      , prov_client_tx_timeout,        },
      {CONFIRMATION_TX_STATUS,             Tx_Success,                    WAIT_PEER_CONFIRMATION_STATUS      , NULL,                          },
      {CONFIRMATION_TX_STATUS,             Tx_Fail,                       LINK_CLOSE_TX_STATUS               , prov_fsm_tx_fail,              },
      {CONFIRMATION_TX_STATUS,             Provision_Fail,                CONFIRMATION_TX_CANCEL_STATUS      , prov_client_tx_timeout,        },

      {CONFIRMATION_TX_CANCEL_STATUS,      Tx_Timeout_Cancel,             LINK_CLOSE_TX_STATUS               , prov_fsm_prepare_close_link,   },

      {WAIT_PEER_CONFIRMATION_STATUS,      Timeout,                       LINK_CLOSE_TX_STATUS               , prov_fsm_prepare_close_link,   },
      {WAIT_PEER_CONFIRMATION_STATUS,      Provisioning_Confirmation_Rx,  WAIT_CONFIRMATION_ACK_STATUS       , client_confirm_rx,             },
      {WAIT_CONFIRMATION_ACK_STATUS,       Ack_Finish,                    RANDOM_TX_STATUS                   ,  client_random_tx,             },

      {RANDOM_TX_STATUS,                   Timeout,                       RANDOM_TX_CANCEL_STATUS            , prov_client_tx_timeout,        },
      {RANDOM_TX_STATUS,                   Tx_Success,                    WAIT_PEER_RANDOM_STATUS            , NULL,                          },
      {RANDOM_TX_STATUS,                   Tx_Fail,                       LINK_CLOSE_TX_STATUS               , prov_fsm_tx_fail,              },
      {RANDOM_TX_STATUS,                   Provision_Fail,                RANDOM_TX_CANCEL_STATUS            , prov_client_tx_timeout,        },

      {RANDOM_TX_CANCEL_STATUS,            Tx_Timeout_Cancel,             LINK_CLOSE_TX_STATUS               , prov_fsm_prepare_close_link,   },

      {WAIT_PEER_RANDOM_STATUS,            Timeout,                       LINK_CLOSE_TX_STATUS               , prov_fsm_prepare_close_link,   },
      {WAIT_PEER_RANDOM_STATUS,            Provisioning_Random_Rx,        WAIT_RANDOM_ACK_STATUS             , client_random_rx,              },
      {WAIT_RANDOM_ACK_STATUS,             Ack_Finish,                    CONFIRMATION_CHECK_STATUS          , confirm_check,                 },

      {CONFIRMATION_CHECK_STATUS,          Confirmation_Check_Pass,        CALC_SESSION_STATUS               , client_calc_session,           },
      {CONFIRMATION_CHECK_STATUS,          Confirmation_Check_Fail,        LINK_CLOSE_TX_STATUS              , client_confirmation_check_fail,},

      {CALC_SESSION_STATUS,                Timeout,                       CALC_SESSION_CANCEL_STATUS         , NULL,                          },
      {CALC_SESSION_STATUS,                Session_Key_Calc_Done,         ENCRYPT_PROVISIONING_DATA_STATUS   , encrypt_data                ,  },

      {CALC_SESSION_CANCEL_STATUS,         Session_Key_Calc_Done,         LINK_CLOSE_TX_STATUS               , prov_fsm_prepare_close_link,   },

      {ENCRYPT_PROVISIONING_DATA_STATUS,   Timeout,                       ENCRYPT_PROVISIONING_CANCEL_STATUS , NULL,                          },
      {ENCRYPT_PROVISIONING_DATA_STATUS,   Encrypt_Data_done,             PROVISIONING_DATA_TX_STATUS        ,  prov_data_tx          ,       },

      {ENCRYPT_PROVISIONING_CANCEL_STATUS, Session_Key_Calc_Done,         LINK_CLOSE_TX_STATUS               , prov_fsm_prepare_close_link,   },

      {PROVISIONING_DATA_TX_STATUS,        Timeout,                       PROVISIONING_DATA_TX_CANCEL_STATUS , prov_client_tx_timeout,        },
      {PROVISIONING_DATA_TX_STATUS,        Tx_Success,                    WAIT_PEER_COMPLETE_STATUS          , NULL,                          },
      {PROVISIONING_DATA_TX_STATUS,        Tx_Fail,                       LINK_CLOSE_TX_STATUS               , prov_fsm_tx_fail,              },
      {PROVISIONING_DATA_TX_STATUS,        Provision_Fail,                PROVISIONING_DATA_TX_CANCEL_STATUS , prov_client_tx_timeout,        },

      {PROVISIONING_DATA_TX_CANCEL_STATUS, Tx_Timeout_Cancel,             LINK_CLOSE_TX_STATUS               , prov_fsm_prepare_close_link,   },

      {WAIT_PEER_COMPLETE_STATUS,          Timeout,                       LINK_CLOSE_TX_STATUS               , prov_fsm_prepare_close_link,   },
      {WAIT_PEER_COMPLETE_STATUS,          Provisioning_Complete_Rx,      WAIT_COMPLETE_ACK_STATUS           , NULL,                          },

      {WAIT_COMPLETE_ACK_STATUS,           Ack_Finish,                    UNPROV_DEVKEY_CALC_STATUS          , client_provison_done,          },

      {UNPROV_DEVKEY_CALC_STATUS,          Devkey_Calc_done,              PROVISION_DONE_STATUS              , prov_fsm_prepare_close_link,   },

      {WAIT_ACK_CANCEL_STATUS,             Ack_Finish,                    UNPROV_DEVKEY_CALC_STATUS          , client_provison_done,          },
      {WAIT_TX_CANCEL_STATUS,              Tx_Normal_Cancel,              IDLE_STATUS                        , prov_client_changeto_idle,     },

      {WAIT_ASYNC_CALC_STATUS,             Confirmation_Calc_Done,        IDLE_STATUS                        , prov_client_changeto_idle,     },
      {WAIT_ASYNC_CALC_STATUS,             Confirmation_Check_Pass,       IDLE_STATUS                        , prov_client_changeto_idle,     },
      {WAIT_ASYNC_CALC_STATUS,             Confirmation_Check_Fail,       IDLE_STATUS                        , prov_client_changeto_idle,     },
      {WAIT_ASYNC_CALC_STATUS,             Session_Key_Calc_Done,         IDLE_STATUS                        , prov_client_changeto_idle,     },
      {WAIT_ASYNC_CALC_STATUS,             Encrypt_Data_done,             IDLE_STATUS                        , prov_client_changeto_idle,     },
      {WAIT_ASYNC_CALC_STATUS,             Devkey_Calc_done,              IDLE_STATUS                        , prov_client_changeto_idle,     },
      {WAIT_ASYNC_CALC_STATUS,             Ack_Finish,                    IDLE_STATUS                        , prov_client_changeto_idle,     },

      {LINK_CLOSE_TX_STATUS,               Link_Close_Tx_Finish,          IDLE_STATUS                        , prov_client_changeto_idle,     },
      {PROVISION_DONE_STATUS,              Link_Close_Tx_Finish,          IDLE_STATUS                        , prov_client_changeto_idle,     },
      

};

static uint8_t auth_and_decrtpy_fsm_table_size = ARRAY_TABLE_SIZE(auth_and_decrtpy_fsm_table);

void prov_client_get_handler_table(prov_fsm_tcb_t *fsm_tcb, prov_fsm_table_t **ppFsm_table, uint8_t *p_table_size)
{
    prov_fsm_state_t  client_state = fsm_tcb->state;
    uint8_t choose_oob_publickey;
    uint8_t choose_oob_auth;
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    if(client_state < START_TX_STATUS)
    {
        *ppFsm_table = capabalities_change_fsm_table;
        *p_table_size = capabalities_change_fsm_table_size;
    }
    else if(client_state >= WAIT_PEER_CONFIRMATION_STATUS)
    {
        *ppFsm_table = auth_and_decrtpy_fsm_table;
        *p_table_size = auth_and_decrtpy_fsm_table_size;
    }
    else
    {
        choose_oob_publickey = tcb->start.public_key;
        choose_oob_auth      = tcb->start.auth_method;
        if(choose_oob_publickey == Using_OOB_Publickey && choose_oob_auth == No_OOB_Authentication)
        {
            *ppFsm_table = oob_publickey_no_auth_fsm_table;
            *p_table_size = oob_publickey_no_auth_fsm_table_size;
        }
        else  if(choose_oob_publickey == Using_OOB_Publickey && (choose_oob_auth == Output_OOB_Authentication|| choose_oob_auth == Static_OOB_Authentication))
        {
            *ppFsm_table = oob_publickey_out_auth_fsm_table;
            *p_table_size = oob_publickey_out_auth_fsm_table_size;
        }
        else  if(choose_oob_publickey == Using_OOB_Publickey && choose_oob_auth == Input_OOB_Authentication)
        {
            *ppFsm_table = oob_publickey_input_auth_fsm_table;
            *p_table_size = oob_publickey_input_auth_fsm_table_size;
        }
        else  if(choose_oob_publickey == No_OOB_Publickey && choose_oob_auth == No_OOB_Authentication )
        {
            *ppFsm_table = no_oob_fsm_table;
            *p_table_size = no_oob_fsm_table_size;
        }
        else  if(choose_oob_publickey == No_OOB_Publickey && (choose_oob_auth == Output_OOB_Authentication|| choose_oob_auth == Static_OOB_Authentication))
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

static prov_client_tcb_buff_t* prov_client_buf_alloc()
{
    return array_buf_alloc(&prov_client_buf);
}

static void prov_client_buf_release(prov_client_tcb_buff_t *ptr)
{
    array_buf_release(&prov_client_buf,ptr);
}


prov_fsm_tcb_t * prov_client_find_dev(uint8_t  *dev_uuid)
{
    /* network key index match*/
    prov_client_tcb_buff_t * tcb_buff;

    FOR_EACH_ALLOCATED_ITEM(prov_client_buf,tcb_buff,
       if(!memcmp(tcb_buff->tcb->dev_uuid,dev_uuid,MESH_DEVICE_UUID_LENGTH))
       {
           return &(tcb_buff->tcb->fsm_tcb);
       }
    );
    return NULL;
}


static prov_fsm_tcb_t * prov_client_find_handle(void *item)
{
    /* network key index match*/
    prov_client_tcb_buff_t * tcb_buff;

    FOR_EACH_ALLOCATED_ITEM(prov_client_buf,tcb_buff,
       if(!tcb_buff->tcb->fsm_tcb.item_compare(tcb_buff->tcb->fsm_tcb.cookie, item))
       {
           
           return &tcb_buff->tcb->fsm_tcb;
       }
    );

    return NULL;
}

static prov_client_tcb_buff_t * prov_client_find_buff(prov_fsm_tcb_t *ptr)
{
    /* network key index match*/
    prov_client_tcb_buff_t * tcb_buff;

    FOR_EACH_ALLOCATED_ITEM(prov_client_buf,tcb_buff,
       if(&tcb_buff->tcb->fsm_tcb == ptr)
       {
           
           return tcb_buff;
       }
    );

    return NULL;
}

void prov_client_free_protocol_instance(prov_fsm_tcb_t *tcb)
{
    prov_client_tcb_buff_t * buf_tcb = prov_client_find_buff(tcb);
    BX_ASSERT(buf_tcb);
    prov_client_buf_release(buf_tcb);
    mesh_timer_delete(tcb->state_timeout_timer);
    mesh_free(tcb);
}

prov_fsm_tcb_t * prov_client_create_instance(void)
{
    prov_client_tcb_buff_t * buf_tcb = prov_client_buf_alloc();

    prov_client_fsm_tcb_t * prov_fsm_instance = (prov_client_fsm_tcb_t *)mesh_alloc(sizeof(prov_client_fsm_tcb_t));
    BX_ASSERT(prov_fsm_instance);
    BX_ASSERT(buf_tcb);
    if(NULL == prov_fsm_instance || NULL == buf_tcb )
    {
        return NULL;
    }
    buf_tcb->tcb = prov_fsm_instance;
    memset(prov_fsm_instance,0,sizeof(prov_client_fsm_tcb_t));
    prov_fsm_instance->fsm_tcb.role = MESH_ROLE_PROVISIONER;
    return &prov_fsm_instance->fsm_tcb;
}

void init_client_param(prov_fsm_tcb_t * fsm_tcb)
{
    prov_client_fsm_tcb_t *tcb = GET_CLIENT_TCB_PTR(fsm_tcb);
    tools_random_generate(tcb->random.field.random_provisioner , RANDOM_PDU_PARAMS_LEN);
    tcb->attention_duration =  m_provisioner.val.attention_duration;
    memcpy(tcb->provision_key.private,  m_provisioner.provisioner_key.private,PRIVATE_KEY_LENGTH);
    memcpy(&tcb->provision_key.local_public,  &m_provisioner.provisioner_key.local_public,sizeof(m_provisioner.provisioner_key.local_public));
}

void * prov_client_create_adv_protocol_instance(    void * cookie, prov_fsm_pkt_send pkt_send,prov_fsm_compare item_compare,prov_fsm_pkt_cancel cancel_tx)
{
    prov_fsm_tcb_t * fsm_tcb = prov_fsm_create_instance(MESH_ROLE_PROVISIONER);
    BX_ASSERT(fsm_tcb);
    if(NULL == fsm_tcb)
    {
        return NULL;
    }
    init_client_param(fsm_tcb);
    fsm_tcb->cookie = cookie;
    fsm_tcb->pkt_send = pkt_send;
    fsm_tcb->item_compare = item_compare;
    fsm_tcb->cancel_tx = cancel_tx;
    fsm_tcb->pre_close_func = (prov_fsm_prepare_link_close)provisioning_link_close_tx;
    fsm_tcb->close_func = (prov_fsm_link_close)provisioning_tx_pdu_buf_release;
    return (void *)fsm_tcb;
}

void prov_client_start_provision(prov_fsm_tcb_t * fsm_tcb)
{
    BX_ASSERT(fsm_tcb);
    prov_fsm_evt_handle(fsm_tcb,Start_Provision,NULL);
}

void * prov_client_get_adv_protocol_instance(void * item)
{
     prov_fsm_tcb_t* tcb = prov_client_find_handle(item);
     return (void *)tcb;
}

void prov_client_close_protocol_instance(    void * item,uint8_t reason)
{
     prov_fsm_params_t param;
     prov_fsm_tcb_t* tcb = prov_client_find_handle(item);
     if(NULL == tcb)
     {
         return;
     }
     param.close_param.error = (Provisioning_Close_Codes)reason;
     prov_fsm_evt_handle(tcb, Link_Close_Evt, &param);
}



