#include "osapp_config.h"
#include "health_server.h"
#include "mesh_model.h"
#include "osapp_config.h"
#include "generic_common.h"
#include "model_common.h"
#include "model_servers_events_api.h"
#include "app_keys_dm.h"
#include "access_tx_process.h"
#include "foundation_msg_handler.h"

static uint8_t fault[10];

static void health_call_user_callback(health_server_t *server, uint16_t value)
{
    mesh_model_evt_t evt;
    evt.params.model_value_set.target_value = value;
    if(server->cb)
        server->cb(&evt);
}

int health_get_fault_array_size(uint8_t *fault_array)
{
    int i = 0;

    while(fault_array[i] && i < HEALTH_FAULT_NUM_MAX)
        i++;

    return i;
}
static void health_insert_fault_to_fault_array(uint8_t *fault_array, uint8_t fault)
{
    int i = 0;

    if(fault == 0)
        return;

    while(fault_array[i]) {
        if(fault_array[i] == fault)
            return;
        else
            i++;
    }

    fault_array[i % HEALTH_FAULT_NUM_MAX] = fault;
}

static void health_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "health_status_tx_done\n");
}

static void health_send_status(uint16_t src_addr, uint16_t dst_addr, void *msg, uint16_t msg_len, uint16_t global_idx, uint16_t opcode, void(*cb)(void *, uint8_t))
{
    model_tx_msg_param_t model_tx_msg_param;
    access_pdu_tx_t *access_pdu_tx_var;

    LOG(3, "health_send_status addr:%x %x\n", src_addr, dst_addr);
    memset(&model_tx_msg_param,0,sizeof(model_tx_msg_param_t));
    model_tx_msg_param.dst_addr.addr = dst_addr;
    model_tx_msg_param.src_addr = src_addr;
    model_tx_msg_param.opcode = opcode;
    model_tx_msg_param.pdu_length = msg_len;
    model_tx_msg_param.seg = true;
    model_tx_msg_param.akf = 1;
    dm_appkey_index_to_appkey_handle(global_idx ,&model_tx_msg_param.key.app_key);

    access_pdu_tx_var = access_model_pkt_build_fill(&model_tx_msg_param, cb,(uint8_t *)msg);
    if(NULL == access_pdu_tx_var)
    {
        return ;
    }
    access_send(access_pdu_tx_var);
}

static void health_attention_timer_cb(mesh_timer_t thandle)
{
    health_server_t *server = (health_server_t *)mesh_timer_get_associated_data(thandle);

    if(server->attention_Timer) {
        mesh_timer_stop(server->attention_Timer);
        mesh_timer_delete(server->attention_Timer);
        server->attention_Timer = NULL;
    }

    health_call_user_callback(server, 0);
}

static void health_attention_set(health_server_t *server, uint8_t attention_value)
{
    if(attention_value == 0) {
        if(server->attention_Timer) {
            mesh_timer_stop(server->attention_Timer);
            mesh_timer_delete(server->attention_Timer);
            server->attention_Timer = NULL;
        }
    }else {
        if(server->attention_Timer) {
            mesh_timer_change_period(server->attention_Timer, pdS_TO_TICKS(attention_value));
        }else {
            server->attention_Timer = mesh_timer_create(
                    "attention", pdS_TO_TICKS(attention_value), pdFALSE, (void *)server, health_attention_timer_cb);
            if(server->attention_Timer)
                mesh_timer_start(server->attention_Timer);
        }
        health_call_user_callback(server, attention_value);
    }

}

static void health_pushlish_timer_cb(mesh_timer_t thandle)
{
    health_server_t *server = (health_server_t *)mesh_timer_get_associated_data(thandle);
    health_period_publish(server);
}

static uint32_t health_calculate_pushlish_period(uint8_t period_divisor)
{
    uint32_t seconds = 2;

    while(period_divisor) {
        period_divisor -= 1;
        seconds *= 2;
    }

    return seconds;
}
static void health_current_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "health_current_status_tx_done\n");
}
static void send_current_status(health_server_t *server)
{
    int fault_size = 0;
    health_current_status_t *msg; 
    int opcode;
    uint16_t dst_addr = 0;
    uint16_t appkey_global_idx = server->model.base.publish->appkey_idx;

    memcpy(server->current_faults_array, fault, sizeof(fault));
    fault_size = health_get_fault_array_size(server->current_faults_array);
    msg = (health_fault_status_t *)mesh_alloc(fault_size + sizeof(health_current_status_t));
    msg->test_id = server->test_id;
    msg->company_id = server->company_id;

    if(fault_size) {
        memcpy(msg->fault_array, server->current_faults_array, fault_size); 
    }

    opcode = ONE_OCTET_OPCODE_GEN(FOUNDATION_MODELS_ONE_OCTET_OPCODE_OFFSET, Health_Current_Status);
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

    LOG(3, "send_current_status addr:%x %x\n", server->model.base.elmt->uni_addr, dst_addr);

    health_send_status(server->model.base.elmt->uni_addr, dst_addr, (void *)msg, sizeof(health_current_status_t) + fault_size, appkey_global_idx, opcode, health_current_status_tx_done);

    mesh_free(msg);
}

void health_period_publish(health_server_t *server)
{
    uint32_t seconds = health_calculate_pushlish_period(server->fast_period_divisor);
    LOG(3, "health_period_publish:%x %x\n", seconds, server->fast_period_divisor);
    if(server->publish_Timer) {
        mesh_timer_change_period(server->publish_Timer, pdS_TO_TICKS(seconds));
    }else {
        server->publish_Timer = mesh_timer_create(
                "publish", pdS_TO_TICKS(seconds), pdFALSE, (void *)server, health_pushlish_timer_cb);
        if(server->publish_Timer)
            mesh_timer_start(server->publish_Timer);
    }

    send_current_status(server);
}

static void send_fault_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    int fault_size = 0;
    health_fault_status_t *msg; 
    int opcode;
    health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);

    memcpy(server->current_faults_array, fault, sizeof(fault));
    fault_size = health_get_fault_array_size(server->current_faults_array);
    msg = (health_fault_status_t *)mesh_alloc(fault_size + sizeof(health_fault_status_t));
    msg->test_id = server->test_id;
    msg->company_id = server->company_id;
    LOG(3, "send_fault_status msg len %d %d\n", sizeof(health_fault_status_t), sizeof(health_fault_status_t) + fault_size);
    if(fault_size) {
        memcpy(msg->fault_array, server->current_faults_array, fault_size); 
    }
    opcode = ONE_OCTET_OPCODE_GEN(FOUNDATION_MODELS_ONE_OCTET_OPCODE_OFFSET, Health_Fault_Status);

    health_send_status(elmt->uni_addr, access_get_pdu_src_addr(pdu), (void *)msg, sizeof(health_fault_status_t) + fault_size, access_get_pdu_appkey_global_index(pdu), opcode, health_status_tx_done);

    mesh_free(msg);
}

void health_fault_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    health_fault_get_t *p_pdu = (health_fault_get_t *)(access_get_pdu_payload(pdu) + 2);
    health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);
    LOG(3, "%s company_id:%x %x\n", __func__, server->company_id, p_pdu->company_id);
    if(p_pdu->company_id != server->company_id)
    {
        return;
    }
    send_fault_status(elmt, model, pdu);
}

void health_fault_clear_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    health_fault_clear_t *p_pdu = (health_fault_clear_t *)(access_get_pdu_payload(pdu) + 2);
    health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);
    LOG(3, "%s company_id:%x %x\n", __func__, server->company_id, p_pdu->company_id);
    if(p_pdu->company_id != server->company_id)
    {
        return;
    }
    
    memset(server->current_faults_array, 0, HEALTH_FAULT_NUM_MAX);
    memset(fault, 0, 10);

    send_fault_status(elmt, model, pdu);
}

void health_fault_clear_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    health_fault_clear_t *p_pdu = (health_fault_clear_t *)(access_get_pdu_payload(pdu) + 2);
    health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);
    LOG(3, "%s company_id:%x %x\n", __func__, server->company_id, p_pdu->company_id);
    if(p_pdu->company_id != server->company_id)
    {
        return;
    }

    memset(server->current_faults_array, 0, HEALTH_FAULT_NUM_MAX);
    memset(fault, 0, 10);
}

void health_fault_test_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    health_fault_test_t *p_pdu = (health_fault_test_t *)(access_get_pdu_payload(pdu) + 2);
    health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);

    if(p_pdu->test_id != server->test_id)
        return;
    server->test_id = p_pdu->test_id;
    LOG(3, "%s company_id:%x %x\n", __func__, server->company_id, p_pdu->company_id);

    if(p_pdu->company_id != server->company_id)
    {
        health_insert_fault_to_fault_array(server->current_faults_array, HEALTH_SELT_TEST_ERROR);
        health_insert_fault_to_fault_array(fault, HEALTH_SELT_TEST_ERROR);
        return;
    }

    send_fault_status(elmt, model, pdu);
}

void health_fault_test_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    health_fault_test_t *p_pdu = (health_fault_test_t *)(access_get_pdu_payload(pdu) + 2);
    health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);

    if(p_pdu->test_id != server->test_id)
        return;
    server->test_id = p_pdu->test_id;
    LOG(3, "%s company_id:%x %x\n", __func__, server->company_id, p_pdu->company_id);

    if(p_pdu->company_id != server->company_id)
    {
        health_insert_fault_to_fault_array(server->current_faults_array, 0x1A);
        health_insert_fault_to_fault_array(fault, HEALTH_SELT_TEST_ERROR);
        return;
    }
}


static void send_period_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    health_period_status_t msg;
    uint32_t opcode;
    health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);
    msg.fast_period_divisor = server->fast_period_divisor;
    opcode = TWO_OCTETS_OPCODE_GEN(FOUNDATION_MODELS_TWO_OCTETS_OPCODE_OFFSET, Health_Period_Status);
    health_send_status(elmt->uni_addr, access_get_pdu_src_addr(pdu), (void *)&msg, sizeof(health_period_status_t), access_get_pdu_appkey_global_index(pdu), opcode, health_status_tx_done);
}

void health_fault_period_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
   // health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);
    
    send_period_status(elmt, model, pdu);
}


void health_fault_period_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    health_period_set_t *p_pdu = (health_period_set_t *)(access_get_pdu_payload(pdu) + 2);
    health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);
    if(p_pdu->fast_period_divisor <= FAST_PERIOD_DIVISOR_MAX)
    {
        server->fast_period_divisor = p_pdu->fast_period_divisor;
    }else {
        return;
    }

    send_period_status(elmt, model, pdu);

    health_period_publish(server);
}

void health_fault_period_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    health_period_set_t *p_pdu = (health_period_set_t *)(access_get_pdu_payload(pdu) + 2);
    health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);
    if(p_pdu->fast_period_divisor <= FAST_PERIOD_DIVISOR_MAX)
    {
        server->fast_period_divisor = p_pdu->fast_period_divisor;
    }else {
        return;
    }

    health_period_publish(server);
}

static void send_attention_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    health_attention_status_t msg;
    health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);
    uint32_t opcode;

    if(server->attention_Timer) {
        msg.attention = TICKS_TO_pdS(mesh_timer_get_remain_time(server->attention_Timer));
    }else {
        msg.attention = 0;
    }
    msg.attention = server->attention;
    opcode = TWO_OCTETS_OPCODE_GEN(FOUNDATION_MODELS_TWO_OCTETS_OPCODE_OFFSET, Health_Attention_Status);
    health_send_status(elmt->uni_addr, access_get_pdu_src_addr(pdu), (void *)&msg, sizeof(health_attention_status_t), access_get_pdu_appkey_global_index(pdu), opcode, health_status_tx_done);
}

void health_fault_attention_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
   // health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);
    send_attention_status(elmt, model, pdu); 
}

void health_fault_attention_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    health_attention_set_t *p_pdu = (health_attention_set_t *)(access_get_pdu_payload(pdu) + 2);
    health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);
    server->attention = p_pdu->attention;
    //attention = p_pdu->attention;
    health_attention_set(server, server->attention);
    send_attention_status(elmt, model, pdu);
}


void health_fault_attention_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    health_attention_set_t *p_pdu = (health_attention_set_t *)(access_get_pdu_payload(pdu) + 2);
    health_server_t *server = GET_SERVER_MODEL_PTR(health_server_t, model);
    server->attention = p_pdu->attention;
    //attention = p_pdu->attention;
    health_attention_set(server, server->attention);
}

void send_health_attention_get(uint16_t src_addr, uint16_t dst_addr)
{
    uint32_t opcode;
    opcode = TWO_OCTETS_OPCODE_GEN(FOUNDATION_MODELS_TWO_OCTETS_OPCODE_OFFSET, Health_Attention_Get);
    LOG(3, "send_health_attention_get: %x %x %x\n", src_addr, dst_addr);
    health_send_status(src_addr, dst_addr, NULL, 0, 0, opcode, health_status_tx_done);
}
void send_health_period_get(uint16_t src_addr, uint16_t dst_addr)
{
    uint32_t opcode;
    opcode = TWO_OCTETS_OPCODE_GEN(FOUNDATION_MODELS_TWO_OCTETS_OPCODE_OFFSET, Health_Period_Get);
    LOG(3, "send_health_period_get: %x %x %x\n", src_addr, dst_addr);
    health_send_status(src_addr, dst_addr, NULL, 0, 0, opcode, health_status_tx_done);
}
void send_health_fault_get(uint16_t src_addr, uint16_t dst_addr, uint16_t company_id)
{
    uint32_t opcode;
    health_fault_get_t msg;
    msg.company_id = company_id;
    opcode = TWO_OCTETS_OPCODE_GEN(FOUNDATION_MODELS_TWO_OCTETS_OPCODE_OFFSET, Health_Fault_Get);
    LOG(3, "send_health_fault_get: %x %x %x\n", src_addr, dst_addr, company_id);
    health_send_status(src_addr, dst_addr, (void *)&msg, sizeof(health_fault_get_t), 0, opcode, health_status_tx_done);
}

void health_deal_from_uart_data(uint8_t *data)
{
    LOG(3, "health_deal_from_uart_data:%x\n", data[0]);
    switch(data[0]) {
        case 0:
            health_insert_fault_to_fault_array(fault, data[1]);
            health_insert_fault_to_fault_array(fault, data[2]);
            health_insert_fault_to_fault_array(fault, data[3]);
            break;
        case 1:
            send_health_fault_get((data[1] << 8) | data[2], (data[3] << 8) | data[4], (data[5] << 8) | data[6]);
            break;
        case 2:
            send_health_period_get((data[1] << 8) | data[2], (data[3] << 8) | data[4]);
            break;
        case 3:
            send_health_attention_get((data[1] << 8) | data[2], (data[3] << 8) | data[4]);
            break;
        case 4:
            break;
        default:
            break;
    };
}

