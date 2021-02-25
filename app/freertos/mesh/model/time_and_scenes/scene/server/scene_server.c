/**
 ****************************************************************************************
 *
 * @file   scene_server.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-21 10:24
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "scene_server.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"
#include "node_setup.h"
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

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
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
static void time_scene_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"time_scene_status_tx_done\n");
}

void time_scene_status_publish(scene_server_t *server)
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
    LOG(3,"time_scene_status_publish current_scene:%x dst_addr:%x\n", server->msg_format->current_scene, dst_addr);
    scene_default_status_t msg;
    msg.current_scene = server->msg_format->current_scene;
    msg.status_code = server->msg_format->status_code;

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(TIME_AND_SCENE_ONE_OCTET_OPCODE_OFFSET, Scene_Status);
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = sizeof(scene_default_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,time_scene_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}
void scene_call_user_callback(scene_server_t *server, mesh_scene_model_evt_type_t type, uint16_t value, uint8_t repeat_flag)
{
    mesh_model_evt_t evt;
    evt.type.scene_type = type;
    evt.params.model_scene_set.target_value = value;
    evt.params.model_scene_set.repeat_flag = repeat_flag;
    if(server->cb)
        server->cb(&evt);
}

int8_t scene_search_scene_number(scene_server_t *server, uint16_t scene_number)
{
    int8_t i;
    for(i = 0; i < SCENE_NUMBER_MAX; i++) {
        LOG(3, "scene_search_scene_number %d %x %x\n", i, server->msg_format->scene_number[i], scene_number);
        if(server->msg_format->scene_number[i] == scene_number)
            return i;
    }
    return -1;
}

int8_t scene_get_scene_number_count(scene_server_t *server)
{
    int i, count = 0;
    for(i = 0; i < SCENE_NUMBER_MAX; i++) {
        if(server->msg_format->scene_number[i]) {
            count++;
        }
    }
    return count;
}

int8_t scene_store_scene_number(scene_server_t *server, uint16_t scene_number)
{
    int i;

    i = scene_search_scene_number(server, scene_number);
    if(i >= 0)
        return SCENE_NUMBER_MAX;

    for(i = 0; i < SCENE_NUMBER_MAX; i++) {
        if(server->msg_format->scene_number[i] == 0) {
            server->msg_format->scene_number[i] = scene_number;
            return i;
        }
    }
    return -1;
}

int8_t scene_delete_scene_number(scene_server_t *server, uint16_t scene_number)
{
    int i;
    for(i = 0; i < SCENE_NUMBER_MAX; i++) {
        if(server->msg_format->scene_number[i] == scene_number) {
            server->msg_format->scene_number[i] = 0;
            return i;
        }
    }
    return -1;
}

static void scene_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "%s\n", __func__);
}
static void send_scene_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scene_status_t msg1;
    scene_default_status_t msg2;
    scene_server_t *server = GET_SERVER_MODEL_PTR(scene_server_t, model);
    model_tx_msg_param_t tx_param;
    access_pdu_tx_t * ptr;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(TIME_AND_SCENE_ONE_OCTET_OPCODE_OFFSET, Scene_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;

    if(server->server_state == GENERIC_TRANS_PROCESS) {
        uint32_t delay_trans_expiry = 0;
        tx_param.pdu_length = sizeof(scene_status_t);

        msg1.status_code = server->msg_format->status_code;

        msg1.current_scene = server->msg_format->current_scene;
        //msg1.current_scene = 0;
        msg1.target_scene = server->msg_format->target_scene;

        delay_trans_expiry = generic_get_delay_trans_expiry(
                server->delay_trans_timer, server->delay_trans_timer->trans_time, server->delay_trans_timer->trans_timer_step);

        msg1.remaining_time = model_transition_time_encode(delay_trans_expiry);
        LOG(3, "send_scene_status remain_time:%x expiry:%x\n", msg1.remaining_time, delay_trans_expiry);
        ptr = access_model_pkt_build_fill(&tx_param,scene_status_tx_done,(uint8_t *)&msg1);
        BX_ASSERT(ptr);
        access_send(ptr);

    }else if(server->server_state == GENERIC_TRANS_IDALE) {
        tx_param.pdu_length = sizeof(scene_default_status_t);

        msg2.status_code = server->msg_format-> status_code;
        msg2.current_scene = server->msg_format->current_scene;
        ptr = access_model_pkt_build_fill(&tx_param,scene_status_tx_done,(uint8_t *)&msg2);
        BX_ASSERT(ptr);
        access_send(ptr);
    }

}

void scene_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    send_scene_status(elmt, model, pdu);
}

static void scene_recall_number_action(void *param)
{
    LOG(3,"scene_recall_number_action\n");
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    scene_server_t *server = (scene_server_t *)timer_param->inst;
    LOG(3,"scene_recall_number_action remain_tick_count:%d\n", server->delay_trans_timer->remain_tick_count);
    if(server->delay_trans_timer->remain_tick_count) {
    }else {
        LOG(3, "scene_recall_number_action scene_number:%x %x\n", 
                server->msg_format->target_scene, server->msg_format->current_scene);
        scene_call_user_callback(server, TIME_SCENE_EVT_RECALL, server->msg_format->target_scene, 0);

        if(server->delay_trans_timer->dt_timer_flag)
            server->server_state = GENERIC_TRANS_IDALE;
    }

}

static void scene_recall_number_action_then_publish(void *param)
{
    LOG(3,"scene_recall_number_action_then_publish\n");
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    scene_server_t *server = (scene_server_t *)timer_param->inst;

    scene_recall_number_action(param);
    if(server->delay_trans_timer->remain_tick_count == 0) {
        time_scene_status_publish(server);
        model_status_publish();
    }
}
int scene_recall_set_rx_handler(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu, uint8_t *publish_status)
{
    uint16_t payload_size;

    generic_valid_field_check_t msg_field;// store src ,dst ,tid
    uint8_t * access = access_get_pdu_payload(pdu);
    payload_size = get_access_pdu_rx_payload_size(pdu);
    msg_field.dst = access_get_pdu_dst_addr(pdu);
    msg_field.src = access_get_pdu_src_addr(pdu);

    scene_server_t *server = GET_SERVER_MODEL_PTR(scene_server_t, model);

    server->tid_queue.inst_param.inst = (void *)server;
    server->delay_trans_timer->inst = server->tid_queue.inst_param.inst;

    LOG(3,"scene_recall_set_rx_handler payload_size:%d dst:%x src:%x\n", payload_size, msg_field.dst, msg_field.src);

    if(payload_size == sizeof(scene_recall_t))
    {
        scene_recall_t *p_pdu = (scene_recall_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -1;
        }

        server->server_state = GENERIC_TRANS_PROCESS;

        if(scene_search_scene_number(server, p_pdu->scene_number) == -1 || p_pdu->scene_number == 0) {
            server->msg_format->status_code = 0x2;
        }else {
            server->msg_format->status_code = 0x0;

            server->msg_format->target_scene = p_pdu->scene_number;
            //*publish_status = (server->msg_format->target_scene != server->msg_format->current_scene);
            *publish_status = 1;
            server->msg_format->current_scene = 0;

            LOG(3,"scene_recall_set_rx_handler:\n");

            if(server->delay_trans_timer->Timer != NULL)
            {
                mesh_timer_stop(server->delay_trans_timer->Timer);
                mesh_timer_delete(server->delay_trans_timer->Timer);
                server->delay_trans_timer->Timer = NULL;
                server->delay_trans_timer->remain_tick_count = 0;
            }

            LOG(3,"tid:%x trans_time:%x delay:%x timer:%p\n", p_pdu->tid, p_pdu->trans_time, p_pdu->delay, server->delay_trans_timer->Timer);

            if(p_pdu->delay == 0 && p_pdu->trans_time == 0){//this block code is used to transition state
                server->delay_trans_timer->dt_timer_flag = USE_FOR_TRANS_TIME;
                scene_recall_number_action(server->delay_trans_timer);
            }else {
                server->delay_trans_timer->target_value = p_pdu->scene_number;
                server->delay_trans_timer->trans_time = p_pdu->trans_time;
                server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_SCENE_RECALL;

                generic_deal_delay_trans_func(server->delay_trans_timer, p_pdu->delay, p_pdu->trans_time, scene_recall_number_action_then_publish);
            }
        }
    }
    else if(payload_size == sizeof(scene_default_recall_t))
    {
        scene_default_recall_t *p_pdu = (scene_default_recall_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -2;
        }

        //this block code is used to  store valid message  within 6 seconds

        server->server_state = GENERIC_TRANS_PROCESS;
        if(scene_search_scene_number(server, p_pdu->scene_number) == -1 || p_pdu->scene_number == 0) {
            server->msg_format->status_code = 0x2;
        }else {
            server->msg_format->status_code = 0x0;
            server->msg_format->target_scene = p_pdu->scene_number;
            //*publish_status = (server->msg_format->target_scene != server->msg_format->current_scene);
            *publish_status = 1;
            server->msg_format->current_scene = 0;

            if(server->delay_trans_timer->Timer != NULL)
            {
                mesh_timer_stop(server->delay_trans_timer->Timer);
                mesh_timer_delete(server->delay_trans_timer->Timer);
                server->delay_trans_timer->Timer = NULL;
                server->delay_trans_timer->remain_tick_count = 0;
            }

            server->delay_trans_timer->target_value = p_pdu->scene_number;
            server->delay_trans_timer->trans_time = user_generic_default_transition_time_get(NULL);
            server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_SCENE_RECALL;
            if(server->delay_trans_timer->trans_time == 0) {
                server->delay_trans_timer->dt_timer_flag = USE_FOR_TRANS_TIME;
                scene_recall_number_action(server->delay_trans_timer);
            }else
                generic_deal_delay_trans_func(server->delay_trans_timer, 0, server->delay_trans_timer->trans_time, scene_recall_number_action_then_publish);
        }
    }

    return 0;
}
void scene_recall_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;
    scene_server_t *server = GET_SERVER_MODEL_PTR(scene_server_t, model);
    int ret = scene_recall_set_rx_handler(elmt, model, pdu, &publish_status);

    if(ret == 0)
        send_scene_status(elmt, model, pdu);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        time_scene_status_publish(server);
        model_status_publish();
    }
}
void scene_recall_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;
    scene_server_t *server = GET_SERVER_MODEL_PTR(scene_server_t, model);

    scene_recall_set_rx_handler(elmt, model, pdu, &publish_status);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        time_scene_status_publish(server);
        model_status_publish();
    }
}

static void scene_register_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "%s\n", __func__);
}
static void send_scene_register_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scene_register_status_t *msg;
    int scene_number_count = 0;
    int i = 0, j = 0;
    scene_server_t *server = GET_SERVER_MODEL_PTR(scene_server_t, model);

    scene_number_count = scene_get_scene_number_count(server);

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(TIME_AND_SCENE_TWO_OCTETS_OPCODE_OFFSET, Scene_Register_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(scene_register_status_t) + scene_number_count * 2;
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,scene_register_status_tx_done,(uint8_t *)&msg);
    
    msg = (scene_register_status_t *)access_model_tx_payload_ptr(ptr,tx_param.opcode);
    msg->status_code = server->msg_format->status_code;
    msg->current_scene = server->msg_format->current_scene;

    if(msg->current_scene > 0x1000)
            server->msg_format->current_scene = server->msg_format->scene_number[0];

    for(i = 0; i < SCENE_NUMBER_MAX; i++) {
        if(server->msg_format->scene_number[i])
            msg->scenes[j++] = server->msg_format->scene_number[i];
    }

    BX_ASSERT(ptr);
    access_send(ptr);
}

void scene_register_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scene_server_t *server = GET_SERVER_MODEL_PTR(scene_server_t, model);
    server->msg_format->status_code = 0;

    send_scene_register_status(elmt, model, pdu);
}

static void scene_store_rx_handle(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scene_server_t *server = GET_SERVER_MODEL_PTR(scene_server_t, model);
    uint8_t * access = access_get_pdu_payload(pdu);
    scence_store_t *p_pdu = (scence_store_t *)(access + 2);
    int8_t scene_index = 0;

    if(p_pdu->scene_number == 0) {
        server->msg_format->status_code = 0x2;
    }else {
        if((scene_index = scene_store_scene_number(server, p_pdu->scene_number)) >= 0) {

            server->msg_format->status_code = 0x0;
            server->msg_format->target_scene = p_pdu->scene_number;
            server->msg_format->current_scene = server->msg_format->scene_number[0];

            if(p_pdu->scene_number < server->msg_format->current_scene)
                server->msg_format->current_scene = p_pdu->scene_number;

            if(p_pdu->scene_number > 0x1000)
                server->msg_format->current_scene = p_pdu->scene_number;

            //if(scene_index != SCENE_NUMBER_MAX) 
                scene_call_user_callback(server, TIME_SCENE_EVT_STORE, (uint16_t)scene_index, scene_index == SCENE_NUMBER_MAX);
        }else {
            server->msg_format->status_code = 0x1;
        }
    }
}
void scene_store_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scene_store_rx_handle(elmt, model, pdu);
    send_scene_register_status(elmt, model, pdu);
}

void scene_store_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scene_store_rx_handle(elmt, model, pdu);
}

void scene_delete_rx_handler(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scene_server_t *server = GET_SERVER_MODEL_PTR(scene_server_t, model);
    uint8_t * access = access_get_pdu_payload(pdu);
    scene_delete_t *p_pdu = (scene_delete_t *)(access + 2);
    int8_t scene_index = 0;

    if((scene_index = scene_delete_scene_number(server, p_pdu->scene_number)) >= 0) {
        server->msg_format->status_code = 0x0;
        scene_call_user_callback(server, TIME_SCENE_EVT_DELETE, scene_index, 0);
        LOG(3, "scene_delete_rx_handler %x\n", scene_index);
    }else {
        server->msg_format->status_code = 0x2;
    }
}

void scene_delete_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scene_delete_rx_handler(elmt, model, pdu);

    send_scene_register_status(elmt, model, pdu);
}

void scene_delete_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scene_delete_rx_handler(elmt, model, pdu);
}
