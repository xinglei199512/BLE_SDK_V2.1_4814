#include "osapp_config.h"
#include "generic_transition_server.h"
#include "generic_transition_common.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"

static uint8_t trans_time = 0x01;
static void generic_default_transition_time_publish_status_tx_done(void *pdu,uint8_t status)
{
      LOG(3,"generic_default_transition_time_publish_status_tx_done status:%d\n", status);
}
static void generic_default_transition_time_status_tx_done(void *pdu,uint8_t status)
{
      LOG(3,"generic_default_transition_time_status_tx_done status:%d\n", status);
}

void generic_transition_status_publish(generic_transition_server_t *server)
{
    uint16_t dst_addr = 0;
    uint16_t appkey_global_idx = server->model.base.publish->appkey_idx;

    if(server->model.base.publish == NULL)
    {
        return;
    }

    if(server->model.base.publish->addr.is_virt)
    {
        dst_addr = server->model.base.publish->addr.addr.virt->virt_addr;
    }
    else
    {
        dst_addr = server->model.base.publish->addr.addr.addr;
    }
    if(IS_UNASSIGNED_ADDR(dst_addr))
    {
        return;
    }
    LOG(3,"generic_transition_status_publish:%x dst_addr:%x\n", server->trans_time, dst_addr);
    generic_transition_msg_t msg;
    msg.transition = server->trans_time;
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(GENERIC_DEFAULT_TRANSITION_TIME_OPCODE_OFFSET,Generic_Default_Transition_Time_Status);
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = sizeof(generic_transition_msg_t);;
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_default_transition_time_publish_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

static void send_generic_default_transition_time_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    generic_transition_msg_t msg;
    
    generic_transition_server_t *server = GET_SERVER_MODEL_PTR(generic_transition_server_t,model);
    LOG(3, "send_generic_default_transition_time_status trans_time:%x length:%d\n", server->trans_time, sizeof(generic_transition_msg_t));
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(GENERIC_DEFAULT_TRANSITION_TIME_OPCODE_OFFSET, Generic_Default_Transition_Time_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(generic_transition_msg_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_default_transition_time_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

void generic_default_transition_time_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    send_generic_default_transition_time_status(elmt, model, pdu);
}

void generic_default_transition_time_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    generic_transition_server_t *server = GET_SERVER_MODEL_PTR(generic_transition_server_t, model);
    uint8_t * access = access_get_pdu_payload(pdu);
    uint8_t publish_status = 0;

    trans_time = *(uint8_t *)(access + 2);

    if(trans_time & 0x80) {
        trans_time = server->trans_time;
        return;
    }

    if(trans_time != server->trans_time)
        publish_status = 1;

    server->trans_time = trans_time;
    LOG(3, "generic_default_transition_time_set_rx trans_time:%x publish_status:%d\n", server->trans_time, publish_status);

    send_generic_default_transition_time_status(elmt, model, pdu);

    if(publish_status)
        generic_transition_status_publish(server);
}

void generic_default_transition_time_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    generic_transition_server_t *server = GET_SERVER_MODEL_PTR(generic_transition_server_t,model);
    uint8_t * access = access_get_pdu_payload(pdu);
    uint8_t publish_status = 0;

    trans_time = *(uint8_t *)(access + 2);

    if(trans_time & 0x80) {
        trans_time = server->trans_time;
        return;
    }

    if(trans_time != server->trans_time)
        publish_status = 1;

    server->trans_time = trans_time;
    LOG(3, "generic_default_transition_time_set_unacknowledged_rx trans_time:%x publish_status:%d\n", server->trans_time, publish_status);

    if(publish_status)
        generic_transition_status_publish(server);
}

uint8_t user_generic_default_transition_time_get(generic_transition_server_t *server)
{
    if(server)
        trans_time = server->trans_time;

    LOG(3, "user_generic_default_transition_time_get:%x\n", trans_time);

    return trans_time;
}

