/*
 * uart_debug_test.c
 *
 *  Created on: 2018-7-14
 *      Author: huichen
 */

#include "osapp_config.h"
#include "mesh_uart_config.h"
#ifdef OSAPP_UART_LOG_TEST
#include "mesh_uart_ctrl.h"
#ifdef MESH_UART_DEBUG_TEST_CMD
#include "uart_debug_test.h"
#include "config_client.h"
#include "config_server.h"
#include "sdk_mesh_definitions.h"
#include "upper_transport.h"
#include "node_setup.h"
#include "generic_onoff_client_api.h"
#include "generic_onoff_client.h"
#include "generic_onoff_server.h"
#include "generic_onoff_common.h"
#include "generic_level_client.h"
#include "generic_level_server.h"
#include "generic_level_common.h"
//#include "mesh_app_action.h"
#include "client_server_uart_test.h"
#include "plf.h"
#include "stdint.h"
#include "node_setup.h"
#include "string.h"
#include "mesh_iv_operation.h"
#include "mesh_iv_operation_ex.h"
#include "mesh_iv_update_test.h"
#include "mesh_kr_test.h"
#include "mesh_node_base.h"


//----------------------------------------------------------------------------------------------------------------
// -----------  define

// -----------  extern function

// -----------  static function

// -----------  static value

//----------------------------------------------------------------------------------------------------------------
/******USER FUNCTION******/

/*********CALLBACKS************/
void config_relay_get_tx_cb(access_pdu_tx_t * param_a,uint8_t param_b)
{
    LOG(3,"config_relay_get_tx_cb\n");
}
void config_relay_set_tx_cb(access_pdu_tx_t * param_a,uint8_t param_b)
{
    LOG(3,"config_relay_set_tx_cb\n");
}

void config_composition_data_get_tx_cb(access_pdu_tx_t * param_a,uint8_t param_b)
{
    LOG(3,"config_composition_data_get_tx_cb\n");
}




// tid auto add
static uint8_t alloc_generic_tid(void)
{
    static uint8_t client_generic_tid=0;

    client_generic_tid++;
    return client_generic_tid;
}

void client_bind_appkey_to_model(uint8_t elem_idx, uint32_t model_id, uint16_t appkey_idx)
{
    LOG(3, "client_bind_appkey_to_model elem_idx:%x model_id:%x appkey_idx:%x\n", elem_idx, model_id, appkey_idx);
    bind_appkey_to_model_by_element(elem_idx, model_id, appkey_idx);
}

static void level_bind_appkey_to_model_cb(access_pdu_tx_t * param_a,uint8_t param_b)
{
    LOG(3,"level_bind_appkey_to_model_cb\n");

}

void server_bind_appkey_to_model(uint16_t elmt_addr, uint8_t sig_model, uint16_t dst_addr, uint32_t model_id, uint16_t appkey_idx)
{
    config_model_app_bind_param_t bind;

    bind.elmt_addr = elmt_addr;
    bind.model_id = model_id;
    bind.sig_model = sig_model;
    bind.appkey_idx = appkey_idx;
    LOG(3,"server_bind_appkey_to_model idx:%x model_id:%x elmt_addr:%x sig_model:%x\n", appkey_idx, bind.model_id, bind.elmt_addr, bind.sig_model);
    config_model_app_bind_tx(&get_config_client()->model.base, &bind, dst_addr, level_bind_appkey_to_model_cb);
}


void mesh_debug_uart_test_rx_callback(uint8_t const *param,uint8_t len)
{
    uint8_t i=0;
    uint16_t dst_addr;
    //parse  msg
    memcpy((uint8_t *)&dst_addr, (param+2), 2);
    LOG(LOG_LVL_INFO,"mesh_debug_uart_test_rx_callback len:%d addr:%x\n", len, dst_addr);
    for(i=0;i<len;i++)
    {
        LOG(LOG_LVL_INFO,"0x%02X-",param[i]);
    }

//    //uint16_t uni_addr = param[1] << 8 | param[2];
      uint32_t signal = param[1] ;
//
//    //switch opcode
       switch(param[0])
       {
//       /* case RX_OP_CLIENT_CONFIG_RELAY_Get :
//        {
//            config_relay_get_tx(&config_client.model.base,uni_addr,config_relay_get_tx_cb);
//        }
//            break;
//        case RX_OP_CLIENT_CONFIG_RELAY_Set:
//        {
//            config_relay_param_t relay_para;
//            memcpy(&relay_para , &param[3] , 2);
//            config_relay_set_tx(&config_client.model.base,&relay_para,uni_addr,config_relay_set_tx_cb);
//        }
//        break;*/
       case  RX_OP_IV_UPDATE_TRANSACTION:
       {
          mesh_test_mode_transition_run((mesh_iv_update_signals_t)signal);
        //mesh_debug_uart_test_tx(param,len);
        }
      break;
      #ifdef MESH_TEST_SEC
      case 0xc1 :
       {
           mesh_iv_update_scene_cmd cmd = (mesh_iv_update_scene_cmd)signal;
           iv_update_ivrefresh_scene(cmd);
       }
       break;
       case 0xc2 :
       {
           mesh_kr_update_scene_cmd cmd = (mesh_kr_update_scene_cmd)signal;
           mesh_kr_update_scene_test(cmd);
       }
       break;
       #endif
       case 0xf1 :
       {
           /*ack */
           if(param[1] == 0) {
               generic_level_msg_default_t level_default;

               memcpy(&level_default, param + 2 + 2, sizeof(generic_level_msg_default_t));
               level_default.tid = alloc_generic_tid();//auto add
               LOG(LOG_LVL_INFO,"level:%x", level_default.level);
               user_client_set(&level_default, sizeof(generic_level_msg_default_t), dst_addr, 1);
           }else{
               generic_level_msg_set_t level_normol;

               memcpy(&level_normol, param + 2 + 2, sizeof(generic_level_msg_set_t));
               level_normol.tid = alloc_generic_tid();//auto add
               LOG(LOG_LVL_INFO,"level:%x", level_normol.level);
               user_client_set(&level_normol, sizeof(generic_level_msg_set_t), dst_addr, 1);
           }
       }
           break;
       case 0xf2 :
       {
           /* no ack */
           if(param[1] == 0) {
               generic_level_msg_default_t level_default;

               memcpy(&level_default, param + 2 + 2, sizeof(generic_level_msg_default_t));
               level_default.tid = alloc_generic_tid();//auto add
               LOG(LOG_LVL_INFO,"level:%x", level_default.level);
               user_client_set(&level_default, sizeof(generic_level_msg_default_t), dst_addr, 0);
           }else{
               generic_level_msg_set_t level_normol;

               memcpy(&level_normol, param + 2 + 2, sizeof(generic_level_msg_set_t));
               level_normol.tid = alloc_generic_tid();//auto add
               LOG(LOG_LVL_INFO,"level:%x", level_normol.level);
               user_client_set(&level_normol, sizeof(generic_level_msg_set_t), dst_addr, 0);
           }

           //send ack
           uart_log_provision_data_t ack;

           //1. send to pc client data init done msg
           ack.Opcode = TX_OP_CLIENT_MODLE_CTRL_MSG_NOACK_ACK;
           ack.len = 0;
           client_server_uart_msg_send((uart_log_provision_data_t *)&ack);
       }
       break;
       case 0xf3:
       {
           uint16_t elmt_addr;
           uint8_t sig_model;
           uint32_t model_id;
           uint16_t appkey_idx;
           memcpy(&elmt_addr, param + 2 + 2, 2);
           memcpy(&sig_model, param + 2 + 2 + 2, 1);
           memcpy(&model_id, param + 2 + 2 + 2 + 1, 4);
           memcpy(&appkey_idx, param + 2 + 2 + 2 + 1 + 4, 2);
           server_bind_appkey_to_model(elmt_addr, sig_model, dst_addr, model_id, appkey_idx);
       }
            break;
       case 0xf4:
       {
           uint8_t elem_idx;
           uint32_t model_id;
           uint16_t appkey_idx;
           memcpy(&elem_idx, param + 2 + 2, 1);
           memcpy(&model_id, param + 2 + 2 + 1, 4);
           memcpy(&appkey_idx, param + 2 + 2 + 1 + 4, 2);
           client_bind_appkey_to_model(elem_idx, model_id, appkey_idx);
       }
            break;
    }
    //mesh_debug_uart_test_tx(param,len);
}


void mesh_debug_uart_test_tx(const void *param,uint16_t len)
{
    uart_log_tx_env_t debug;

    if(param)
    {
        debug.cmd = UART_PKTCMD_DEBUG_TX;
        debug.tx_len = len;
        if(debug.tx_len)
        {
            debug.pdata = (uint8_t *)pvPortMalloc(debug.tx_len);
            BX_ASSERT(debug.pdata!=NULL);
            memcpy(debug.pdata,(uint8_t *)(param),debug.tx_len);
        }

        uart_log_send_cmd(debug.cmd,debug.pdata,debug.tx_len);
        if(debug.tx_len) vPortFree(debug.pdata);
    }

    LOG(LOG_LVL_INFO,"mesh_debug_uart_test_tx !\n");
}
//----------------------------------------------------------------------------------------------------------------
#endif /* MESH_UART_DEBUG_TEST_CMD */
#else
void mesh_debug_uart_test_tx(const void *param,uint16_t len)
{
	
}

#endif /* OSAPP_UART_LOG_TEST */

