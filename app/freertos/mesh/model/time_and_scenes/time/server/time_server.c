/**
 ****************************************************************************************
 *
 * @file   time_server.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-17 11:32
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

#include "time_server.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"
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

int getWeekdayByYearday(int iY, int iM, int iD)
{
    int iWeekDay = -1;
    if (1 == iM || 2 == iM)
    {
        iM += 12;
        iY--;
    }
    iWeekDay = (iD + 2 * iM + 3 * (iM + 1) / 5 + iY + iY / 4 - iY / 100 + iY / 400) % 7;

    return iWeekDay;
}

void set_time_msg_format_to_time(time_server_t *server)
{
    int64_t L, T, B;
    int32_t D, H, C, X, Q, Z, V;
    int16_t E;
    int8_t F, A, J, K;

    T = server->msg_format->set.tai_seconds;

    if(T < server->msg_format->utc_set.tai_of_delta_change)
        E = server->msg_format->tai_utc_delta_current; 
    else
        E = server->msg_format->utc_set.tai_utc_delta_new; 

    if( T + 1 == T < server->msg_format->utc_set.tai_of_delta_change)
        F = 1;
    else
        F = 0;
    LOG(3, "set_time_msg_format_to_time E:%d F:%d T:%x %x\n", E, F, T, (T >> 32) & 0xffffffff);

    L = T - E - F;
    D = (int)(L / 86400);
    server->msg_format->day.hour = (int)(((L - D * 86400) / 3600) % 24);
    server->msg_format->day.min = (int)((L - D * 86400 - server->msg_format->day.hour * 3600) / 60 % 60);
    server->msg_format->day.sec = (int)((L - D * 86400 - server->msg_format->day.hour * 3600 - server->msg_format->day.min * 60 + F) % 60);

    B = D + 730119;
    Q = B % 146097;
    C = (int)(Q / 36524);
    H = Q % 36524;
    X = (int)((H % 1461) / 365);
    server->msg_format->day.year = (int)((B / 146097) * 400 
                                + C * 100
                                + (int)(H / 1461) * 4 
                                + X
                                + (!((C == 4) || (X == 4)) ? 1 : 0));

    Z = server->msg_format->day.year - 1;
    V = B - 365 * Z - (int)(Z / 4) + (int)(Z / 100) - (int)(Z / 400);

    if((server->msg_format->day.year % 4 == 0 && server->msg_format->day.year % 100 != 0) || server->msg_format->day.year % 400 == 0)
        A = 1;
    else 
        A = 2;

    if(V + A < 61)
        J = 0;
    else
        J = A;

    server->msg_format->day.mon = (int)(((V + J) * 12 + 373) / 367);

    if(server->msg_format->day.mon <= 2)
        K = 0;
    else 
        K = A;

    server->msg_format->day.mday = V + K + 1 - (int)((367 * server->msg_format->day.mon - 362) / 12);

#if 1
    tm_date stm;
    stm.year = server->msg_format->day.year;
    stm.mon = server->msg_format->day.mon;
    stm.mday = server->msg_format->day.mday;
    stm.hour = server->msg_format->day.hour;
    stm.min = server->msg_format->day.min;
    stm.sec = server->msg_format->day.sec;
    stm.ms = (uint8_t)((server->msg_format->set.subsecond / 256.0) * 999);

    LOG(3, "set_time_msg_format_to_time second:%d\n", stm.sec);
    LOG(3, "minute:%d\n", stm.min);
    LOG(3, "hour:%d\n", stm.hour);
    LOG(3, "mday:%d\n", stm.mday);
    LOG(3, "month:%d\n", stm.mon);
    LOG(3, "year:%d\n", stm.year);
    set_system_time(stm);
#endif

    LOG(3, "tai_seconds:%x %x\n", server->msg_format->set.tai_seconds, (server->msg_format->set.tai_seconds >> 32) & 0xffffffff);
    LOG(3, "second:%d %d\n", server->msg_format->day.sec, server->msg_format->set.subsecond);
    LOG(3, "minute:%d\n", server->msg_format->day.min);
    LOG(3, "hour:%d\n", server->msg_format->day.hour);
    LOG(3, "mday:%d\n", server->msg_format->day.mday);
    LOG(3, "month:%d\n", server->msg_format->day.mon);
    LOG(3, "year:%d\n", server->msg_format->day.year);
}

static void time_set_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "%s\n", __func__);
}

static void send_time_set_get_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    time_server_t *server = GET_SERVER_MODEL_PTR(time_server_t, model);
    time_status_t msg;
    tm_date stm;

    get_system_time(&stm);
#if 0
    server->msg_format->set.subsecond = stm.ms;
    msg.subsecond = (uint8_t)((stm.ms / 999.0) * 256); 
    msg.tai_seconds = server->msg_format->set.tai_seconds = time_set_date_to_tai_seconds(&stm);
#endif
    //msg.subsecond = (uint8_t)((server->msg_format->set.subsecond / 999.0) * 256); 
    msg.subsecond = server->msg_format->set.subsecond;
    msg.tai_seconds = server->msg_format->set.tai_seconds;
    LOG(3, "send_time_set_get_status:%d %x %x\n", stm.ms, msg.tai_seconds, ((msg.tai_seconds>>32) & 0xffffffff));
    LOG(3, "\n");

    msg.uncertainty = server->msg_format->set.uncertainty;
    msg.time_authority = server->msg_format->set.time_authority;
    msg.tai_utc_delta = server->msg_format->set.tai_utc_delta;
    msg.time_zone_offset = server->msg_format->set.time_zone_offset;

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(TIME_AND_SCENE_ONE_OCTET_OPCODE_OFFSET, Time_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(time_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,time_set_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}

void time_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t * access = access_get_pdu_payload(pdu);
    time_set_t *p_pdu = (time_set_t *)(access + 1);
    time_server_t *server = GET_SERVER_MODEL_PTR(time_server_t, model);
    mesh_model_evt_t evt;

    server->msg_format->set.tai_seconds = p_pdu->tai_seconds;
    //server->msg_format->set.subsecond = (uint8_t)((p_pdu->subsecond / 256.0) * 999);
    server->msg_format->set.subsecond = p_pdu->subsecond;
    server->msg_format->set.uncertainty = p_pdu->uncertainty;
    server->msg_format->set.time_authority = p_pdu->time_authority;
    server->msg_format->set.tai_utc_delta = p_pdu->tai_utc_delta;
    server->msg_format->set.time_zone_offset = p_pdu->time_zone_offset;

    evt.type.time_type = TIME_TIME_EVT_SET;
    LOG(3, "time_set_rx tai_seconds:%x %x\n", server->msg_format->set.tai_seconds, (server->msg_format->set.tai_seconds >> 32) & 0xffffffff);
    LOG(3, "time_set_rx subsecond:%x\n", server->msg_format->set.subsecond);
    LOG(3, "time_set_rx uncertainty:%x\n", server->msg_format->set.uncertainty);
    LOG(3, "time_set_rx time_authority:%x\n", server->msg_format->set.time_authority);
    LOG(3, "time_set_rx tai_utc_delta:%x\n", server->msg_format->set.tai_utc_delta);
    LOG(3, "time_set_rx time_zone_offset:%x\n", server->msg_format->set.time_zone_offset);

    if(server->cb)
        server->cb(&evt);

    send_time_set_get_status(elmt, model, pdu);
}

void time_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    mesh_model_evt_t evt;
    time_server_t *server = GET_SERVER_MODEL_PTR(time_server_t, model);
    evt.type.time_type = TIME_TIME_EVT_GET;
    if(server->cb)
        server->cb(&evt);

    send_time_set_get_status(elmt, model, pdu);
}

static void time_role_set_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "%s\n", __func__);
}
static void send_time_role_set_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    time_server_t *server = GET_SERVER_MODEL_PTR(time_server_t, model);
    time_role_status_t msg;
    msg.time_role = server->msg_format->role_set.time_role;
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(TIME_AND_SCENE_TWO_OCTETS_OPCODE_OFFSET, Time_Role_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(time_role_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,time_role_set_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}
    
void time_role_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    mesh_model_evt_t evt;
    time_server_t *server = GET_SERVER_MODEL_PTR(time_server_t, model);
    evt.type.time_type = TIME_TIME_ROLE_EVT_GET;
    if(server->cb)
        server->cb(&evt);
    send_time_role_set_status(elmt, model, pdu);
}

void time_role_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t * access = access_get_pdu_payload(pdu);
    time_role_set_t *p_pdu = (time_role_set_t *)(access + 2);
    time_server_t *server = GET_SERVER_MODEL_PTR(time_server_t, model);
    mesh_model_evt_t evt;

    if(p_pdu->time_role > 0x3)
        return;

    server->msg_format->role_set.time_role = p_pdu->time_role;

    evt.type.time_type = TIME_TIME_ROLE_EVT_SET;
    if(server->cb)
        server->cb(&evt);

    send_time_role_set_status(elmt, model, pdu);
}

static void time_zone_set_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "%s\n", __func__);
}
static void send_time_zone_set_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    time_server_t *server = GET_SERVER_MODEL_PTR(time_server_t, model);
    time_zone_status_t msg;
    msg.time_zone_offset_current = server->msg_format->zone_set.time_zone_offset_new;/* need to change */
    msg.time_zone_offset_new = server->msg_format->zone_set.time_zone_offset_new;
    msg.tai_of_zone_change = server->msg_format->zone_set.tai_of_zone_change;
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(TIME_AND_SCENE_TWO_OCTETS_OPCODE_OFFSET, Time_Zone_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(time_zone_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,time_zone_set_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}
void time_zone_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    mesh_model_evt_t evt;
    time_server_t *server = GET_SERVER_MODEL_PTR(time_server_t, model);
    evt.type.time_type = TIME_TIME_ZONE_EVT_GET;
    if(server->cb)
        server->cb(&evt);
    send_time_zone_set_status(elmt, model, pdu);
}
void time_zone_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t * access = access_get_pdu_payload(pdu);
    time_zone_set_t *p_pdu = (time_zone_set_t *)(access + 2);
    time_server_t *server = GET_SERVER_MODEL_PTR(time_server_t, model);
    mesh_model_evt_t evt;

    server->msg_format->zone_set.time_zone_offset_new = p_pdu->time_zone_offset_new;
    server->msg_format->zone_set.tai_of_zone_change = p_pdu->tai_of_zone_change;

    /* TODO zone_offset_current behavior*/
    if(server->msg_format->zone_set.time_zone_offset_new != server->msg_format->tai_zone_offset_current)
        server->msg_format->tai_zone_offset_current = server->msg_format->zone_set.time_zone_offset_new;

    evt.type.time_type = TIME_TIME_ZONE_EVT_GET;
    if(server->cb)
        server->cb(&evt);

    send_time_zone_set_status(elmt, model, pdu);
}

static void tai_zone_set_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "%s\n", __func__);
}

static void send_tai_utc_delta_set_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    time_server_t *server = GET_SERVER_MODEL_PTR(time_server_t, model);
    tai_utc_delta_status_t msg;

    msg.tai_utc_delta_current = server->msg_format->tai_utc_delta_current;/* need to change */
    msg.padding1 = 0;
    msg.tai_utc_delta_new = server->msg_format->utc_set.tai_utc_delta_new;
    msg.padding2 = 0;
    msg.tai_of_delta_change = server->msg_format->utc_set.tai_of_delta_change;

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(tai_utc_delta_status_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(TIME_AND_SCENE_TWO_OCTETS_OPCODE_OFFSET, TAI_UTC_Delta_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(time_zone_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,tai_zone_set_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}

void tai_utc_delta_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    send_tai_utc_delta_set_status(elmt, model, pdu);
}

void tai_utc_delta_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    
    uint8_t * access = access_get_pdu_payload(pdu);
    tai_utc_delta_set_t *p_pdu = (tai_utc_delta_set_t *)(access + 2);
    time_server_t *server = GET_SERVER_MODEL_PTR(time_server_t, model);

    server->msg_format->utc_set.tai_utc_delta_new = p_pdu->tai_utc_delta_new;
    server->msg_format->utc_set.tai_of_delta_change = p_pdu->tai_of_delta_change;

    /* TODO tai_utc_delta_current behavior*/
    if(server->msg_format->set.tai_seconds < server->msg_format->utc_set.tai_of_delta_change) 
        server->msg_format->tai_utc_delta_current = server->msg_format->utc_set.tai_utc_delta_new;

    send_tai_utc_delta_set_status(elmt, model, pdu);
}
