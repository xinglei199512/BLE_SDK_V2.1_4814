#include "osapp_config.h"
//#include "access.h"
#include "tmall_model_server.h"
#include "tmall_model_common.h"
#include "tmall_model_server.h"
#include "generic_common.h"
#include "mesh_model.h"
#include "sdk_mesh_definitions.h"
//#include "access.h"
#include "app_keys_dm.h"
#include "access_tx_process.h"
bool indication_success=0;
custom_tmall_server_t *tmall_server = NULL;
bool  success_respond=0;
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

static uint8_t simple_tmall_tid(void)
{
    static uint8_t simple_tmall_tid=0;
    simple_tmall_tid++;
    return simple_tmall_tid;
}

static void tmall_message_attr_indication_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"tmall_message_attr_indication_tx_done\n");
}

void set_tmall_server(custom_tmall_server_t *server)
{
    tmall_server = server;
}

void tmall_msg_attr_indication_tx(void *msg, uint8_t len)
{
    custom_tmall_msg_attr_indication_t *send_pkt;
    access_pdu_tx_t *access_pdu_tx_var;
    model_tx_msg_param_t model_tx_msg_param;

    if(tmall_server == NULL) {
        LOG(3, "tmall_msg_attr_indication_tx tmall_server is NULL\n");
        return;
    }

    LOG(3, "tmall_msg_attr_indication_tx tmall_server msg:%p len%d\n", msg, len);

    memset(&model_tx_msg_param, 0, sizeof(model_tx_msg_param_t));
    model_tx_msg_param.dst_addr.addr = 0xf000;
    model_tx_msg_param.src_addr = mesh_node_get_primary_element_addr();
    model_tx_msg_param.opcode = THREE_OCTETS_OPCODE_GEN(TMALL_MODEL_OPCODE_OFFSET, TMALL_MODEL_OPCODE_COMPANY_ID, TMALL_MODEL_OPCODE_COMPANY_ID_SHIFT, Vendor_Message_Attr_Indication);
    model_tx_msg_param.ttl = NULL;
    model_tx_msg_param.pdu_length = len + sizeof(custom_tmall_msg_attr_indication_t);
    model_tx_msg_param.seg = true;

    dm_appkey_index_to_appkey_handle(0, &model_tx_msg_param.key.app_key);

    access_pdu_tx_var = access_model_msg_build(&model_tx_msg_param, tmall_message_attr_indication_tx_done);

    if(NULL == access_pdu_tx_var)
    {
        return ;
    }

    access_pdu_tx_var->src[0] = (model_tx_msg_param.opcode >> 16) & 0xFF;
    access_pdu_tx_var->src[1] = (model_tx_msg_param.opcode >> 8) & 0xFF;
    access_pdu_tx_var->src[2] = (model_tx_msg_param.opcode >> 0) & 0xFF;

    send_pkt = (custom_tmall_msg_attr_indication_t *)(access_pdu_tx_var->src + 3);
    send_pkt->tid = simple_tmall_tid();
    memcpy(send_pkt->value, (uint8_t *)msg, len);

    show_buf("tmall_msg_attr_indication_tx", access_pdu_tx_var->src, model_tx_msg_param.pdu_length + 3);

    access_send(access_pdu_tx_var);
}

static void tmall_server_action(custom_tmall_server_t *server)
{
    mesh_model_evt_t evt;

     //TODO
    /* Assigning evt parameters */

    if(server->cb)
        server->cb(&evt);
}

void tmall_attr_set_rx_handler(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint16_t payload_size;

    generic_valid_field_check_t msg_field;// store src ,dst ,tid
    uint8_t * access = access_get_pdu_payload(pdu);
    custom_tmall_msg_set_t *p_pdu = (custom_tmall_msg_set_t *)(access + 3);

    payload_size = get_access_pdu_rx_payload_size(pdu);
    msg_field.dst = access_get_pdu_dst_addr(pdu);
    msg_field.src = access_get_pdu_src_addr(pdu);

    custom_tmall_server_t *server = GET_SERVER_MODEL_PTR(custom_tmall_server_t, model);

    show_buf("tmall_attr_set_rx_handler", access, access_get_pdu_payload_length(pdu));

    msg_field.tid = p_pdu->tid;

    server->tid_queue.inst_param.inst = (void *)server;

    LOG(3,"tmall_set_rx_handler payload_size:%d dst:%x src:%x tid:%x\n", 
            payload_size, msg_field.dst, msg_field.src, msg_field.tid);

    if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
        LOG(3, "recv same tid Within 6 seconds\n");
        return;
    }

    server->msg_format[0].attr_type = p_pdu->attr_type;
    server->msg_format[0].value[0] = p_pdu->value[0];
    server->msg_format[0].value[1] = p_pdu->value[1];
    server->msg_format[0].value[2] = p_pdu->value[2];

    /* TODO */
    /* Assigning server parameters */

    tmall_server_action(server);
}

static void tmall_message_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"tmall_message_status_tx_done\n");
}

static void tmall_message_response_status_tx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    custom_tmall_msg_status_t msg;
    custom_tmall_server_t *server = GET_SERVER_MODEL_PTR(custom_tmall_server_t, model);

    msg.tid = simple_tmall_tid();
    msg.value_type = server->msg_format[0].attr_type;
    msg.value[0] = server->msg_format[0].value[0];
    msg.value[1] = server->msg_format[0].value[1];
    msg.value[2] = server->msg_format[0].value[2];

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = THREE_OCTETS_OPCODE_GEN(TMALL_MODEL_OPCODE_OFFSET, TMALL_MODEL_OPCODE_COMPANY_ID, TMALL_MODEL_OPCODE_COMPANY_ID_SHIFT, Vendor_Message_Attr_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(custom_tmall_msg_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,tmall_message_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

void tmall_message_transparent_msg_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tmall_message_transparent_msg_rx\n");
}

void tmall_message_attr_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tmall_message_attr_get_rx\n");

    tmall_message_response_status_tx(elmt, model, pdu);
}

void tmall_message_attr_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tmall_message_attr_set_rx\n");
    tmall_attr_set_rx_handler(elmt, model, pdu);

    tmall_message_response_status_tx(elmt, model, pdu);
}

void tmall_attr_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tmall_attr_set_unacknowledged_rx\n");
    tmall_attr_set_rx_handler(elmt, model, pdu);

}

static void tmall_message_attr_confirmation_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"tmall_message_attr_confirmation_tx_done\n");
}
static void tmall_message_response_attr_confirmation_tx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    custom_tmall_msg_attr_confirmation_t msg;
    custom_tmall_server_t *server = GET_SERVER_MODEL_PTR(custom_tmall_server_t, model);

    msg.tid = simple_tmall_tid();

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = THREE_OCTETS_OPCODE_GEN(TMALL_MODEL_OPCODE_OFFSET, TMALL_MODEL_OPCODE_COMPANY_ID, TMALL_MODEL_OPCODE_COMPANY_ID_SHIFT, Vendor_Message_Attr_Confirmation);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(custom_tmall_msg_attr_confirmation_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,tmall_message_attr_confirmation_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

void tmall_message_attr_indication_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tmall_message_attr_indication_rx\n");
    tmall_message_response_attr_confirmation_tx(elmt, model, pdu);
}

void tmall_message_attr_confirmation_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tmall_message_attr_confirmation_rx\n");
	   indication_success=1;
	
}
void tmall_message_attr_time_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tmall_message_attr_time_rx\n");
}
//Vendor_Message_Transparernt_Ack
void tmall_message_transparent_ack(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tmall_message_attr_respond_time\n");

}
void tmall_message_attr_respond_time(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tmall_message_attr_respond_time\n");

    success_respond=1;	
	  uint8_t tid;
	  uint16_t attr_type;
	  uint8_t time[4];	
	  uint8_t * access = access_get_pdu_payload(pdu);
	  memcpy(&tid,access + 3,1);
    memcpy(&attr_type,access + 4,2);
	  for(int i=0;i<4;i++)
	{
    memcpy(&time[i],access +6+i,1);
		LOG(3,"time[%d]:%x\n",i,time[i]);
	}

}