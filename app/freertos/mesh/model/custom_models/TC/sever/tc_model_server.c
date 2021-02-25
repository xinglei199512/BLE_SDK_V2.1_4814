#include "osapp_config.h"
//#include "access.h"
#include "tc_model_server.h"
#include "tc_model_common.h"

#include "generic_common.h"
#include "mesh_model.h"
#include "sdk_mesh_definitions.h"
//#include "access.h"
#include "app_keys_dm.h"
#include "access_tx_process.h"

custom_tc_server_t *tc_server = NULL;

void show_buf(const char *name, uint8_t *buf, int len)
{
    int i = 0;

    LOG(3, "%s: ", name);
    for(i = 0; i < len; i++) {
        if(i % 16 == 0 && i != 0)
            LOG(3, "\n");
        LOG(3, "0x%x ", buf[i]);
    }
    LOG(3, "\n");
}

static uint8_t simple_tc_tid(void)
{
    static uint8_t simple_tc_tid=0;
    simple_tc_tid++;
    return simple_tc_tid;
}

static void tc_message_attr_indication_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"tc_message_attr_indication_tx_done\n");
}

void set_tc_server(custom_tc_server_t *server)
{
    tc_server = server;
}

void tc_msg_attr_indication_tx(void *msg, uint8_t len)
{

    custom_tc_msg_attr_indication_t *send_pkt;
    access_pdu_tx_t *access_pdu_tx_var;
    model_tx_msg_param_t model_tx_msg_param;

    if(tc_server == NULL) {
        LOG(3, "tc_msg_attr_indication_tx tc_server is NULL\n");
        return;
    }

    LOG(3, "tc_msg_attr_indication_tx tc_server msg:%p len%d\n", msg, len);

    memset(&model_tx_msg_param, 0, sizeof(model_tx_msg_param_t));
    model_tx_msg_param.dst_addr.addr = 0xf000;
    model_tx_msg_param.src_addr = mesh_node_get_primary_element_addr();
    model_tx_msg_param.opcode = THREE_OCTETS_OPCODE_GEN(TC_MODEL_OPCODE_OFFSET, TC_MODEL_OPCODE_COMPANY_ID, TC_MODEL_OPCODE_COMPANY_ID_SHIFT, TC_Message_Attr_Indication);
    model_tx_msg_param.ttl = NULL;
    model_tx_msg_param.pdu_length = len + sizeof(custom_tc_msg_attr_indication_t);
    model_tx_msg_param.seg = true;

    dm_appkey_index_to_appkey_handle(0, &model_tx_msg_param.key.app_key);

    access_pdu_tx_var = access_model_msg_build(&model_tx_msg_param, tc_message_attr_indication_tx_done);

    if(NULL == access_pdu_tx_var)
    {
        return ;
    }

    access_pdu_tx_var->src[0] = (model_tx_msg_param.opcode >> 16) & 0xFF;
    access_pdu_tx_var->src[1] = (model_tx_msg_param.opcode >> 8) & 0xFF;
    access_pdu_tx_var->src[2] = (model_tx_msg_param.opcode >> 0) & 0xFF;

    send_pkt = (custom_tc_msg_attr_indication_t *)(access_pdu_tx_var->src + 3);
    send_pkt->tid = simple_tc_tid();
    memcpy(send_pkt->value, (uint8_t *)msg, len);

    show_buf("tc_msg_attr_indication_tx", access_pdu_tx_var->src, model_tx_msg_param.pdu_length + 3);

    access_send(access_pdu_tx_var);
}

static void tc_server_action(custom_tc_server_t *server)
{
    LOG(3, " tc_server_action \n");

    mesh_model_evt_t evt;

    if(server->cb)
        server->cb(&evt);
}

void tc_attr_set_rx_handler(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    generic_valid_field_check_t msg_field;// store src ,dst ,tid
    uint8_t * access = access_get_pdu_payload(pdu);
    custom_tc_msg_set_t *p_pdu = (custom_tc_msg_set_t *)(access + 3);

    msg_field.dst = access_get_pdu_dst_addr(pdu);
    msg_field.src = access_get_pdu_src_addr(pdu);
	  LOG(3, " msg_field.dst :0X%x    msg_field.src:0x%x  \n",msg_field.dst,msg_field.src);
    custom_tc_server_t *server = GET_SERVER_MODEL_PTR(custom_tc_server_t, model);

    show_buf("tc_attr_set_rx_handler", access, access_get_pdu_payload_length(pdu));
    uint32_t length=access_get_pdu_payload_length(pdu);
	  LOG(3, " length %d\n",length);
    msg_field.tid = 10;
  
    server->tid_queue.inst_param.inst = (void *)server;
    if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
        LOG(3, "recv same tid Within 6 seconds\n");
        return;
    }

   for(int i=0;i<length;i++)
    server->msg_format[0].value[i] = p_pdu->value[i];
		server->msg_format[0].length=length;
    tc_server_action(server);
}

static void tc_message_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"tc_message_status_tx_done\n");
}

static void tc_message_response_status_tx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
	
    LOG(3,"tc_message_response_status_tx\n");

    custom_tc_msg_status_t msg;
    custom_tc_server_t *server = GET_SERVER_MODEL_PTR(custom_tc_server_t, model);  
	  uint32_t length=server->msg_format[0].length;
    LOG(3,"length=%d\n",length);
   for(int i=0;i<SET_VALUE_NUM;i++)
    {
    msg.value[i] = server->msg_format[0].value[i];
    }
    model_tx_msg_param_t tx_params;
    memset(&tx_params,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_params.key.app_key);
    tx_params.opcode = THREE_OCTETS_OPCODE_GEN(TC_MODEL_OPCODE_OFFSET, TC_MODEL_OPCODE_COMPANY_ID, TC_MODEL_OPCODE_COMPANY_ID_SHIFT, TC_Message_Attr_Status);
    tx_params.dst_addr.addr = access_get_pdu_src_addr(pdu);
//		tx_params.dst_addr.addr=0x6;
    tx_params.pdu_length = sizeof(custom_tc_msg_status_t);
    tx_params.src_addr = server->model.base.elmt->uni_addr;
    tx_params.akf = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_params.rx_time = &rx_time;
	  LOG(3, " tx_params.dst_addr.addr :0X%x    tx_params.src_addr:0x%x  \n",tx_params.dst_addr.addr,tx_params.src_addr);
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_params,tc_message_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);


}

void tc_message_transparent_msg_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tc_message_transparent_msg_rx\n");
}

void tc_message_attr_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tc_message_attr_get_rx\n");

      custom_tc_msg_get_value_t msg;
      custom_tc_server_t *server = GET_SERVER_MODEL_PTR(custom_tc_server_t, model);      
    for(int i=0;i<200;i++)
        {
         msg.value[i] =200-i;
        }
      model_tx_msg_param_t tx_param;
      memset(&tx_param,0,sizeof(model_tx_msg_param_t));
      dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
      tx_param.opcode = THREE_OCTETS_OPCODE_GEN(TC_MODEL_OPCODE_OFFSET, TC_MODEL_OPCODE_COMPANY_ID, TC_MODEL_OPCODE_COMPANY_ID_SHIFT, TC_Message_Attr_Get_Value);
      tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
      tx_param.pdu_length = sizeof(custom_tc_msg_get_value_t);
      tx_param.src_addr = server->model.base.elmt->uni_addr;
      tx_param.akf = 1;
      ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
      tx_param.rx_time = &rx_time;
      access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,tc_message_status_tx_done,(uint8_t *)&msg);
      BX_ASSERT(ptr);
      access_send(ptr);

}

void tc_message_attr_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tc_message_attr_set_rx\n");
    tc_attr_set_rx_handler(elmt, model, pdu);

    tc_message_response_status_tx(elmt, model, pdu);
}

void tc_attr_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tc_attr_set_unacknowledged_rx\n");
    tc_attr_set_rx_handler(elmt, model, pdu);

}

static void tc_message_attr_confirmation_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"tc_message_attr_confirmation_tx_done\n");
}
static void tc_message_response_attr_confirmation_tx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3,"tc_message_response_attr_confirmation_tx\n");
	
    custom_tc_msg_attr_confirmation_t msg;
    custom_tc_server_t *server = GET_SERVER_MODEL_PTR(custom_tc_server_t, model);

    msg.tid = simple_tc_tid();

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = THREE_OCTETS_OPCODE_GEN(TC_MODEL_OPCODE_OFFSET, TC_MODEL_OPCODE_COMPANY_ID, TC_MODEL_OPCODE_COMPANY_ID_SHIFT, TC_Message_Attr_Confirmation);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(custom_tc_msg_attr_confirmation_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,tc_message_attr_confirmation_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

void tc_message_attr_indication_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tc_message_attr_indication_rx\n");
    tc_message_response_attr_confirmation_tx(elmt, model, pdu);
}

void tc_message_attr_confirmation_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tc_message_attr_confirmation_rx\n");
}

