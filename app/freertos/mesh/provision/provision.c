
#include "osapp_config.h"
#include "arch.h"
#include <string.h>
#include "co_endian.h"
#include "mesh_gatt.h"
#include "sdk_mesh_config.h"

#include "sdk_mesh_definitions.h"
#include "log.h"
#include "mesh_core_api.h"
#include "provision_api.h"
#include "provisioning_s.h"
#include "adv_bearer_tx.h"
#include "mesh_queued_msg.h"
#include "mesh_reset_database.h"
#include "stack_mem_cfg.h"
#include "provision.h"
#include "reassembly.h"
#include "mesh_env.h"
#include "unprov_device_fsm.h"
#include "osapp_utils.h"
#include "unprov_device_intf.h"

static const unsigned char  crctable[256] = {    //reversed, 8-bit, poly=0x07
    0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75,  0x0E, 0x9F, 0xED, 0x7C, 0x09, 0x98, 0xEA, 0x7B,
    0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A, 0xF8, 0x69,  0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67,
    0x38, 0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D,  0x36, 0xA7, 0xD5, 0x44, 0x31, 0xA0, 0xD2, 0x43,
    0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0, 0x51,  0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F,

    0x70, 0xE1, 0x93, 0x02, 0x77, 0xE6, 0x94, 0x05,  0x7E, 0xEF, 0x9D, 0x0C, 0x79, 0xE8, 0x9A, 0x0B,
    0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19,  0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17,
    0x48, 0xD9, 0xAB, 0x3A, 0x4F, 0xDE, 0xAC, 0x3D,  0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0, 0xA2, 0x33,
    0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21,  0x5A, 0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F,

    0xE0, 0x71, 0x03, 0x92, 0xE7, 0x76, 0x04, 0x95,  0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A, 0x9B,
    0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89,  0xF2, 0x63, 0x11, 0x80, 0xF5, 0x64, 0x16, 0x87,
    0xD8, 0x49, 0x3B, 0xAA, 0xDF, 0x4E, 0x3C, 0xAD,  0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
    0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1,  0xCA, 0x5B, 0x29, 0xB8, 0xCD, 0x5C, 0x2E, 0xBF,

    0x90, 0x01, 0x73, 0xE2, 0x97, 0x06, 0x74, 0xE5,  0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB,
    0x8C, 0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9,  0x82, 0x13, 0x61, 0xF0, 0x85, 0x14, 0x66, 0xF7,
    0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C, 0xDD,  0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3,
    0xB4, 0x25, 0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1,  0xBA, 0x2B, 0x59, 0xC8, 0xBD, 0x2C, 0x5E, 0xCF
};

static bool pb_adv_tx_continue(pb_adv_tx_t *ptr);
static void pb_adv_set_tx_state(provision_adv_env_t *env, provision_tx_state_t state);
static void pb_adv_set_ack_tx_state(provision_adv_env_t *env, provision_tx_state_t state);
static void pb_send(pb_adv_tx_t *ptr);
static pb_adv_tx_t *pb_adv_tx_buf_alloc(uint8_t pkt_type);

DEF_ARRAY_BUF(provision_reassembly_buf, provision_reassembly_env_t, PROVISION_REASSEMBLY_ENV_BUF_SIZE);
DEF_ARRAY_BUF(provision_tx_pdu_buf, provision_adv_env_t, PROVISION_TX_PDU_BUF_SIZE);
DEF_ARRAY_BUF(pb_adv_tx_buf,pb_adv_tx_t,PB_ADV_TX_BUF_SIZE);


static pb_adv_tx_t *get_pb_adv_data_buf(void)
{
    pb_adv_tx_t *adv_tx = pb_adv_tx_buf_alloc(PROVISION_BEARER_ADV_PKT);
    return adv_tx ;
}

static bool provisoning_judge_is_transaction_ack_pdu(pb_adv_tx_t *ptr)
{
    uint8_t type = *(uint8_t *)&ptr->adv.adv_data[DATA_OFFSET_IN_PB_ADV + DATA_OFFSET_IN_BEARER];
    //LOG(3, "provisoning_judge_is_transaction_ack_pdu type:%x ptr:%x\n", type, ptr);
    return ((type & 0x03) == 0x01);
}

static void pb_adv_tx_done_call_callback(uint32_t link_id, bool is_transaction_ack)
{
    provision_adv_env_t *env = provision_env_search(link_id);
    bool send_callback = false;
    bool ack_notice = false;
    uint8_t reason = Tx_Fail_Finish;

    if(env == NULL) {
        return;
    }

    LOG(3, "pb_adv_tx_done_call_callback link_id:%x callback:%p current_state:%x %x is_transaction_ack:%d\n",
            link_id, env->adv_cb, env->current_state, env->ack_current_state, is_transaction_ack);

    if(is_transaction_ack) {
        if(env->ack_current_state == ADV_TRANS_SEND_ACK) {
            ack_notice = true;
        }

        env->ack_current_state = ADV_STATE_IDLE;

        if(ack_notice) {
            void *tcb = prov_fsm_get_adv_protocol_instance(&link_id);
            prov_fsm_ack_finish(tcb, &link_id);
        }

        return;
    }else {
        if(env->current_state == ADV_SEND_LINK_CLOSE || env->current_state == ADV_SEND_LINK_OPEN) {
            send_callback = true;
            reason = Tx_Success_Finish;
        }

        env->current_state = ADV_STATE_IDLE;
    }

    if(env->pkt_state == PROVISION_PKT_CANCEL) {
        send_callback = true;
        reason = Tx_Cancel_Finish;
    }else if(env->pkt_state == PROVISION_PKT_ACK_TIMEOUT) {
        send_callback = true;
        reason = Tx_Fail_Finish;
    }else if(env->pkt_state == PROVISION_PKT_DONE) {
        send_callback = true;
        reason = Tx_Success_Finish;
    }

    if(send_callback) {
        void *tcb = prov_fsm_get_adv_protocol_instance(&link_id);
        if(env->adv_cb) {
            adv_callback adv_cb_temp = env->adv_cb;
            env->adv_cb = NULL;
            adv_cb_temp(tcb, (Tx_Reason)reason);
        }
    }
}

static void pb_adv_tx_buf_free(pb_adv_tx_t *ptr)
{
    uint32_t link_id = co_bswap32(*(uint32_t *)&ptr->adv.adv_data[DATA_OFFSET_IN_BEARER]);
    bool is_transaction_ack = provisoning_judge_is_transaction_ack_pdu(ptr);

    mesh_timer_delete(ptr->adv.timer);
    ptr->adv.timer = NULL;
    array_buf_release(&pb_adv_tx_buf, ptr);

    pb_adv_tx_done_call_callback(link_id, is_transaction_ack);
}

provision_adv_env_t* provisioning_tx_pdu_buf_alloc(bool is_server)
{
    provision_adv_env_t *env = array_buf_alloc(&provision_tx_pdu_buf);
    env->ack_Timer = NULL;
    env->establish_Timer = NULL;
    env->adv_cb = NULL;
    env->current_state = ADV_STATE_IDLE;
    env->ack_current_state = ADV_STATE_IDLE;
    pb_adv_set_tx_state(env, PROVISION_PKT_IDLE);
    pb_adv_set_ack_tx_state(env, PROVISION_PKT_IDLE);
    if(is_server) {
        env->transaction_num.peer = 0x0;
        env->transaction_num.local = 0x80;
    }else {
        env->transaction_num.peer = 0x80;
        env->transaction_num.local = 0x0;
    }
    return env;
}

void provisioning_tx_pdu_buf_release(provision_adv_env_t *env)
{
    LOG(3, "provisioning_tx_pdu_buf_release\n");
    if(env && env->ack_Timer) {
        mesh_timer_stop(env->ack_Timer);
        mesh_timer_delete(env->ack_Timer);
        env->ack_Timer = NULL;
    }
   if(env && env->establish_Timer) {
        mesh_timer_delete(env->establish_Timer);
        env->establish_Timer = NULL;
    }
    array_buf_release(&provision_tx_pdu_buf, env);
}

provision_reassembly_env_t* provision_reassembly_env_search(uint32_t link_id)
{
    provision_reassembly_env_t *ptr;
    FOR_EACH_ALLOCATED_ITEM(provision_reassembly_buf, ptr,
        if(ptr->link_id == link_id)
        {
            return ptr;
        }
    );
    return NULL;
}

provision_reassembly_env_t* provision_reassembly_env_alloc(void)
{
    provision_reassembly_env_t *ptr = array_buf_alloc(&provision_reassembly_buf);
    if(NULL == ptr)
    {
        return NULL;
    }
    return ptr;
    
}

bool provision_reassembly_env_release(provision_reassembly_env_t *ptr)
{
    if(array_buf_release(&provision_reassembly_buf, ptr))
    {
        return true;
    }else
    {
        return false;
    }
}


provision_adv_env_t* provision_env_search(uint32_t link_id)
{
    provision_adv_env_t *env = prov_get_cookie(prov_fsm_get_adv_protocol_instance(&link_id));

    return env;
}

static void pb_adv_set_ack_tx_state(provision_adv_env_t *env, provision_tx_state_t state)
{
    if(!env)
        return;
    //LOG(3, "pb_adv_set_ack_tx_state env:%p\n", env);
    env->ack_pkt_state = state;
}

static uint8_t pb_adv_get_ack_tx_state(provision_adv_env_t *env)
{
    if(!env)
        return 0xff;

    //LOG(3, "pb_adv_get_ack_tx_state env:%p state\n", env, env->ack_pkt_state);
    
    return env->ack_pkt_state;
}

static void pb_adv_set_tx_state(provision_adv_env_t *env, provision_tx_state_t state)
{
    if(!env)
        return;
    //LOG(3, "pb_adv_set_tx_state env:%p\n", env);
    env->pkt_state = state;
}

static uint8_t pb_adv_get_tx_state(provision_adv_env_t *env)
{
    if(!env)
        return 0xff;

    //LOG(3, "pb_adv_get_tx_state env:%p state\n", env, env->pkt_state);
    
    return env->pkt_state;
}

static void provisioning_close_current_connect(provision_adv_env_t *env, uint8_t reason)
{
    if(env && env->current_state != ADV_SEND_LINK_CLOSE) {
        env->current_state = ADV_RECV_LINK_CLOSE;
        prov_fsm_adv_close_link(&env->link_id, reason);

    }
}


static void provisioning_ack_timeoutCallback(mesh_timer_t thandle)
{
    //LOG(3, "provisioning_ack_timeoutCallback\n");
    provision_adv_env_t *env = (provision_adv_env_t *)pvTimerGetTimerID(thandle);
    void *tcb = NULL;

    if(env) {
        LOG(3, "provisioning_ack_timeoutCallback link_id:%x\n", env->link_id);
        tcb = prov_fsm_get_adv_protocol_instance(&env->link_id);
        if(tcb) {
            pb_adv_set_tx_state(env, PROVISION_PKT_ACK_TIMEOUT);
        }
    }
}


static void provisioning_establish_timeoutCallback(mesh_timer_t thandle)
{
    //LOG(3, "provisioning_establish_timeoutCallback\n");
    provision_adv_env_t *env = (provision_adv_env_t *)pvTimerGetTimerID(thandle);
    void *tcb = NULL;

    if(env) {
        LOG(3, "provisioning_establish_timeoutCallback link_id:%x\n", env->link_id);
        tcb = prov_fsm_get_adv_protocol_instance(&env->link_id);
        if(tcb) {
             provisioning_close_current_connect(env,PROVISIOON_TIMEOUT);
        }
    }
}

static void  provisioning_start_establish_timer(provision_adv_env_t *env)
{
      LOG(3, "provisioning_start_establish_timer link_id:%x\n", env->link_id);

   if(env->establish_Timer) {
        return;
    }else {
              LOG(3, "timer create\n");

        env->establish_Timer = mesh_timer_create(
                "establish_Timer", pdMS_TO_TICKS(ESTABLISH_LINK_TIMEOUT), pdFALSE, (void *)env, provisioning_establish_timeoutCallback);

        if(env->establish_Timer)
            mesh_timer_start(env->establish_Timer);
    }
}
static void  provisioning_stop_establish_timer(provision_adv_env_t *env)
{
   LOG(3, "provisioning_stop_establish_timer link_id:%x\n", env->link_id);

   if(env->establish_Timer) {
       mesh_timer_delete(env->establish_Timer);
       env->establish_Timer = NULL;
   }
}

uint8_t provisioning_pdu_FCS_gen(uint8_t *pdu, uint8_t length)
{
    /*Init*/
    unsigned char FCS=0xFF;
    unsigned char len = length;
    unsigned char *p = pdu;
    /*len is the number of bytes in the message, p points to message*/
    while (len--) {
        FCS=crctable[FCS^*p++];
    }
    /*Ones complement*/
    FCS=0xFF-FCS;
    return FCS;
}

void pb_adv_tx(pb_adv_info_t *info)
{
    pb_adv_tx_t *adv_tx = (pb_adv_tx_t *)info->data_buf;
    uint8_t * data_buf = adv_tx->adv.adv_data ;
    data_buf[0] = info->data_length + DATA_OFFSET_IN_PB_ADV + 1;
    data_buf[1] = MESH_PROVISIONING_AD_TYPE;
    info->Link_ID = co_bswap32(info->Link_ID);
    memcpy(data_buf + DATA_OFFSET_IN_BEARER,&info->Link_ID,sizeof(uint32_t));
    data_buf[4 + DATA_OFFSET_IN_BEARER] = info->Transaction_Number;
    adv_tx->adv.data_length = info->data_length + DATA_OFFSET_IN_PB_ADV + DATA_OFFSET_IN_BEARER;
    pb_send(adv_tx);
}

void provisioning_transaction_start_pdu_tx(provision_adv_env_t *env, transaction_start_info_t *pdu_info,uint8_t *data,uint8_t data_length)
{
    pb_adv_tx_t *tx = get_pb_adv_data_buf();
    uint8_t* buf = tx->adv.adv_data + DATA_OFFSET_IN_BEARER + DATA_OFFSET_IN_PB_ADV;
    buf[0] = pdu_info->SegN<<2;
    buf[1] = (uint8_t)(pdu_info->TotalLength >> 8);
    buf[2] = (uint8_t)(pdu_info->TotalLength & 0xff); 
    buf[3] = pdu_info->FCS;
    memcpy(buf + 4, data, data_length);

    pb_adv_info_t pb_adv = 
    {
        .Link_ID = env->link_id,
        .Transaction_Number = env->transaction_num.local,
        .data_buf = (uint8_t*)tx,
        .data_length = data_length + 4 ,
    };

    pb_adv_tx(&pb_adv);
}

void provisioning_transaction_continuation_pdu_tx(provision_adv_env_t *env, uint8_t SegIdx,uint8_t *data,uint8_t data_length)
{
    pb_adv_tx_t *tx = get_pb_adv_data_buf();
    uint8_t * buf = tx->adv.adv_data + DATA_OFFSET_IN_BEARER + DATA_OFFSET_IN_PB_ADV;
    buf[0] = (SegIdx<<2) | 0x2;
    memcpy(buf + 1,data,data_length);

    pb_adv_info_t pb_adv = {
        .Link_ID = env->link_id,
        .Transaction_Number = env->transaction_num.local,
        .data_buf = (uint8_t *)tx,
        .data_length = data_length + 1,
    };   
    pb_adv_tx(&pb_adv);
}

void provisioning_bearer_control_pdu_tx(provision_adv_env_t *env, uint8_t bearer_code,uint8_t *param,uint8_t param_len)
{
    pb_adv_tx_t * tx = get_pb_adv_data_buf();
    uint8_t * buf = tx->adv.adv_data + DATA_OFFSET_IN_BEARER + DATA_OFFSET_IN_PB_ADV;
    buf[0] = (bearer_code<<2) | 0x3;
    if(param_len)
    {
        memcpy(&buf[1],param,param_len);
    }
    
    pb_adv_info_t pb_adv = 
    {
        .Link_ID = env->link_id,
        .Transaction_Number = 0,
        .data_buf = (uint8_t *)tx,
        .data_length = param_len + 1,
    };
    pb_adv_tx(&pb_adv);
}

void provisioning_link_ack_tx(provision_adv_env_t *env)
{
    {
        provisioning_bearer_control_pdu_tx(env, Link_Ack, NULL, 0);
    }
}

void provisioning_link_open_tx(provision_adv_env_t *env, uint8_t *dev_uuid, adv_callback cb)
{
    tools_random_generate((uint8_t *)(&env->link_id), 4);
    env->current_state = ADV_SEND_LINK_OPEN;
    env->adv_cb = cb;
    provisioning_bearer_control_pdu_tx(env, Link_Open, dev_uuid, MESH_DEVICE_UUID_LENGTH);

    if(env->ack_Timer) {
        mesh_timer_change_period(env->ack_Timer, pdMS_TO_TICKS(TRANSACTION_ACK_TIMEOUT));
    }else {
        env->ack_Timer = mesh_timer_create(
                "act_Timer", pdMS_TO_TICKS(TRANSACTION_ACK_TIMEOUT), pdFALSE, (void *)env, provisioning_ack_timeoutCallback);

        if(env->ack_Timer)
            mesh_timer_start(env->ack_Timer);
    }
}

void provisioning_link_close_tx(provision_adv_env_t *env, uint8_t reason, adv_callback cb)
{
    env->adv_cb = cb;
    env->current_state = ADV_SEND_LINK_CLOSE;
    provisioning_bearer_control_pdu_tx(env, Link_Close, &reason, sizeof(uint8_t));
}

void provisioning_transation_adv_tx(void *instance, uint8_t *data, uint8_t length, adv_callback cb)
{
    uint8_t send_length;
    provision_adv_env_t *env = (provision_adv_env_t *)instance;
    env->adv_cb = cb;

    transaction_start_info_t start_pdu_info =
    {
        .TotalLength = length,
        .FCS = provisioning_pdu_FCS_gen(data,length),
    };

    if(length > MAX_DATA_SIZE_TRANSACTION_START_PDU){
        uint8_t remaining_length = length - MAX_DATA_SIZE_TRANSACTION_START_PDU;
        uint8_t quotient = remaining_length / MAX_DATA_SIZE_TRANSACTION_CONTINUATION_PDU;
        uint8_t SegIdx = 0;

        send_length = MAX_DATA_SIZE_TRANSACTION_START_PDU;
        start_pdu_info.SegN = remaining_length%MAX_DATA_SIZE_TRANSACTION_CONTINUATION_PDU?quotient + 1:quotient;
        provisioning_transaction_start_pdu_tx(env, &start_pdu_info, data, send_length);

        while(SegIdx++ != start_pdu_info.SegN) {
            data += send_length;

            send_length = (SegIdx == start_pdu_info.SegN) ? 
                length - MAX_DATA_SIZE_TRANSACTION_START_PDU - (start_pdu_info.SegN-1) * MAX_DATA_SIZE_TRANSACTION_CONTINUATION_PDU:
                MAX_DATA_SIZE_TRANSACTION_CONTINUATION_PDU; 

            LOG(3, "segidx%d segN:%d send_length:%d\n", SegIdx, start_pdu_info.SegN, send_length);

            provisioning_transaction_continuation_pdu_tx(env, SegIdx, data, send_length);
        }
    }else {
        send_length = length;
        start_pdu_info.SegN = 0;
        provisioning_transaction_start_pdu_tx(env, &start_pdu_info, data, send_length);
    }

    env->current_state = ADV_TRANS_SEND_PDU;
    env->transaction_num.local++;

    if(env->ack_Timer) {
        mesh_timer_change_period(env->ack_Timer, pdMS_TO_TICKS(TRANSACTION_ACK_TIMEOUT));
    }else {
        env->ack_Timer = mesh_timer_create(
                "act_Timer", pdMS_TO_TICKS(TRANSACTION_ACK_TIMEOUT), pdFALSE, (void *)env, provisioning_ack_timeoutCallback);

        if(env->ack_Timer)
            mesh_timer_start(env->ack_Timer);
    }
}

void provisioning_transaction_ack_pdu_tx(uint32_t link_id, uint8_t peer)
{
    pb_adv_tx_t * tx = pb_adv_tx_buf_alloc(PROVISION_BEARER_ADV_PKT);
    uint8_t * buf = tx->adv.adv_data + DATA_OFFSET_IN_BEARER + DATA_OFFSET_IN_PB_ADV;
    buf[0] = 0x1;
    pb_adv_info_t pb_adv = {
        .Link_ID = link_id,
        .Transaction_Number = peer,
        .data_buf = (uint8_t *)tx,
        .data_length = 1,
    };
    pb_adv_tx(&pb_adv);
}

static void provisioning_pdu_process(provision_adv_env_t *env, provision_reassembly_env_t *provision_reassembly, uint8_t trans_number)
{
    void *tcb = prov_fsm_get_adv_protocol_instance(&provision_reassembly->link_id); 
    LOG(3, "provisioning_pdu_process link_id:%x\n", provision_reassembly->link_id);

    //send ack
    env->ack_current_state = ADV_TRANS_ALREADY_SENDED_ACK;
    provisioning_transaction_ack_pdu_tx(provision_reassembly->link_id, env->transaction_num.peer);

    if(env->transaction_num.peer == trans_number) {
        env->ack_current_state = ADV_TRANS_SEND_ACK;
        env->transaction_num.peer++;
        provision_fsm_entry(tcb, provision_reassembly->buf, provision_reassembly->pdu_info.TotalLength);
    }
    provisioning_stop_establish_timer(env);
    mesh_free(provision_reassembly->buf);
    provision_reassembly_env_release(provision_reassembly);
}

static bool provision_pdu_integrity_check(uint8_t block_ack, uint8_t SegN)
{
    return (1<<(SegN+ 1)) - 1 ==  block_ack;
}

static void provisioning_transaction_start_pdu_rx(provision_adv_env_t *env, transaction_start_info_t *start_pdu_info, uint8_t trans_number, uint8_t data_length, uint8_t *data_buf)
{
    provision_reassembly_env_t *provision_reassembly  = NULL;
    if(start_pdu_info->SegN > 63)
    {
        return;
    }

    provision_reassembly = provision_reassembly_env_search(env->link_id);

    if(provision_reassembly == NULL) {
        provision_reassembly = provision_reassembly_env_alloc();
        //LOG(3, "provisioning_transaction_start_pdu_rx alloc\n");
        BX_ASSERT(provision_reassembly);

        provision_reassembly->buf = mesh_alloc(start_pdu_info->TotalLength);
        BX_ASSERT(provision_reassembly->buf);

        provision_reassembly->block_ack = 0;
        provision_reassembly->link_id = env->link_id;
    }else {
        //LOG(3, "%s:%d block_ack:%x segN:%x\n", __func__, __LINE__, provision_reassembly->block_ack, start_pdu_info->SegN);
        if(provision_reassembly->block_ack & (1 << 0))
            return;
    }

    provision_reassembly->pdu_info.SegN = start_pdu_info->SegN;
    provision_reassembly->pdu_info.FCS = start_pdu_info->FCS;
    provision_reassembly->pdu_info.TotalLength = start_pdu_info->TotalLength;
    provision_reassembly->block_ack |= (1 << 0);
    memcpy(provision_reassembly->buf, data_buf, data_length);

    //LOG(3, "%s:%d block_ack:%x segN:%x\n", __func__, __LINE__, provision_reassembly->block_ack, start_pdu_info->SegN);
    if(provision_pdu_integrity_check(provision_reassembly->block_ack, provision_reassembly->pdu_info.SegN))
    {
        provisioning_pdu_process(env, provision_reassembly, trans_number);
    }
}

static void provisioning_transaction_ack_pdu_rx(provision_adv_env_t *env, uint8_t trans_number)
{
    LOG(3, "provisioning_transaction_ack_pdu_rx transnumber:%x %x\n", 
            env->transaction_num.local, trans_number);
    //judge valid ack for myself
    if(env->transaction_num.local - trans_number == 1) {
        if(mesh_timer_active(env->ack_Timer)) {
            mesh_timer_stop(env->ack_Timer);
        }

        pb_adv_set_tx_state(env, PROVISION_PKT_DONE);
        
    }
}

static void provisioning_transaction_continuation_pdu_rx(provision_adv_env_t *env, uint8_t SegIdx, uint8_t trans_number, uint8_t data_length, uint8_t *data_buf)
{
    provision_reassembly_env_t *provision_reassembly = provision_reassembly_env_search(env->link_id);

    if(provision_reassembly == NULL) {
     //   LOG(3, "provisioning_transaction_continuation_pdu_rx alloc\n");
        provision_reassembly = provision_reassembly_env_alloc();
        BX_ASSERT(provision_reassembly);

        provision_reassembly->buf = mesh_alloc(MAX_DATA_SIZE_TRANSACTION_START_PDU + MAX_DATA_SIZE_TRANSACTION_CONTINUATION_PDU * 2);
        BX_ASSERT(provision_reassembly->buf);

        provision_reassembly->pdu_info.TotalLength = MAX_DATA_SIZE_TRANSACTION_START_PDU + MAX_DATA_SIZE_TRANSACTION_CONTINUATION_PDU * 2;
        provision_reassembly->pdu_info.SegN = 5;
        provision_reassembly->block_ack = 0;
        provision_reassembly->link_id = env->link_id;
    }

    if(provision_reassembly->block_ack & (1 << SegIdx))
        return;

    //LOG(3,"provisioning_transaction_continuation_pdu_rx block_ack:%x segN:%x\n", provision_reassembly->block_ack, provision_reassembly->pdu_info.SegN);
    memcpy(&provision_reassembly->buf[MAX_DATA_SIZE_TRANSACTION_START_PDU + MAX_DATA_SIZE_TRANSACTION_CONTINUATION_PDU * (SegIdx - 1)], data_buf, data_length);
    provision_reassembly->block_ack |= (1 << SegIdx);

    if(provision_pdu_integrity_check(provision_reassembly->block_ack, provision_reassembly->pdu_info.SegN))
    {
        provisioning_pdu_process(env, provision_reassembly, trans_number);
    }
}


static void provisioning_bearer_control_pdu_rx(provision_adv_env_t *env, uint8_t bearer_code, uint8_t *param, uint8_t param_len)
{
    LOG(3, "provisioning_bearer_control_pdu_rx:%d\n", bearer_code);
    switch(bearer_code) {
        case Link_Open:{
            env->current_state = ADV_SEND_LINK_ACK;
            provisioning_link_ack_tx(env);
            }
            break;
        case Link_Ack:{
            if(env->current_state == ADV_SEND_LINK_OPEN)
                pb_adv_set_tx_state(env, PROVISION_PKT_DONE);
            }
            break;
        case Link_Close:{
                provisioning_close_current_connect(env, *param);
            }
            break;
        default:
            LOG(3, "recv wrong pkt\n");
            break;

    }
}

void provisioning_pdu_adv_cancel_tx(void *cookie, adv_callback cb)
{
    provision_adv_env_t *env = (provision_adv_env_t *)cookie;
    env->adv_cb = cb;
    pb_adv_set_tx_state(env, PROVISION_PKT_CANCEL);
    pb_adv_set_ack_tx_state(env, PROVISION_PKT_CANCEL);
}

int32_t  provision_adv_compare(void * cookie, void *p_item)
{
    provision_adv_env_t *env = (provision_adv_env_t *)cookie;
    if(env->link_id == *(uint32_t*)p_item)
        return 0;
    else
        return -1;
}

void deal_provisioning_pdu_rx(pb_adv_info_t *pb_adv)
{
    uint8_t pdu_type = pb_adv->data_buf[0] & 0x3;
    uint8_t segment_index, bearer_code;

    segment_index = bearer_code = pb_adv->data_buf[0] >> 2;

    provision_adv_env_t *env = (provision_adv_env_t *)prov_get_cookie(prov_fsm_get_adv_protocol_instance(&pb_adv->Link_ID));
    //LOG(3,"%s: pdu_type=%d bearer_code:%x env:%p\n", __func__, pdu_type, bearer_code, env);

    if(env == NULL && (pdu_type == Provisioning_Bearer_Control_PDU) && bearer_code == Link_Open) {
        if(!get_is_provisioned()) {
            uint8_t *uuid = get_unprov_dev_uuid();
            if(memcmp(uuid, pb_adv->data_buf + 1, MESH_DEVICE_UUID_LENGTH) == 0) {
                env = provisioning_tx_pdu_buf_alloc(true);
                BX_ASSERT(env);
                LOG(3, "%s alloc env local:%x link_id:%x\n", __func__, env->transaction_num.local, pb_adv->Link_ID);
                env->link_id = pb_adv->Link_ID;
                if(prov_serv_create_adv_protocol_instance(env, provisioning_transation_adv_tx, provision_adv_compare, provisioning_pdu_adv_cancel_tx) == NULL) {
                    provisioning_tx_pdu_buf_release(env);
                  return;
                }
                provisioning_start_establish_timer(env);

            }
        }else {
            return;
        }
    }else if(env != NULL && (pdu_type == Provisioning_Bearer_Control_PDU) && (bearer_code == Link_Open)) {
        //TODO
    }
    if(env != NULL && pdu_type == Provisioning_Bearer_Control_PDU && bearer_code == Link_Close)
        provisioning_stop_establish_timer(env);

    
    if(env == NULL) {
        return;
    }
    LOG(3, "current_state:%x %x pdu_type:%x\n", env->current_state, env->ack_current_state, pdu_type);

    if(env->current_state == ADV_SEND_LINK_ACK && (pdu_type == Provisioning_Bearer_Control_PDU) && bearer_code == Link_Open) {
        return;
    }else if(env->current_state == ADV_TRANS_SEND_PDU && (pdu_type == Transaction_Start_PDU || pdu_type == Transaction_Continuation_PDU)) {
        if((env->transaction_num.peer - pb_adv->Transaction_Number) == 1) {
            pb_adv_set_tx_state(env, PROVISION_PKT_DELAY_LONGTIME);
        }else {
            provisioning_transaction_ack_pdu_rx(env, pb_adv->Transaction_Number);
            return;
        }
    }else if(env->current_state == ADV_RECV_LINK_CLOSE) {
        return;
    }

    if((env->ack_current_state == ADV_TRANS_SEND_ACK || env->ack_current_state == ADV_TRANS_ALREADY_SENDED_ACK) && pdu_type == Transaction_Ack_PDU) {
        pb_adv_set_ack_tx_state(env, PROVISION_PKT_DONE);
        return;
    }

    if((env->ack_current_state == ADV_TRANS_SEND_ACK || env->ack_current_state == ADV_TRANS_ALREADY_SENDED_ACK) && (pdu_type == Transaction_Start_PDU || pdu_type == Transaction_Continuation_PDU)) {
        if((env->transaction_num.peer - pb_adv->Transaction_Number) == 1) {
        }else {
            pb_adv_set_ack_tx_state(env, PROVISION_PKT_DONE);
        }
        return;
    }

    if(pdu_type == Transaction_Start_PDU || pdu_type == Transaction_Continuation_PDU)
    {
        //LOG(3, "trans_number:%x %x\n", env->transaction_num.peer, pb_adv->Transaction_Number);
        if(pb_adv->Transaction_Number == env->transaction_num.peer)
        {
            //env->transaction_num.peer++;
        }else if((env->transaction_num.peer - pb_adv->Transaction_Number) == 1) {
            env->ack_current_state = ADV_TRANS_ALREADY_SENDED_ACK;
            provisioning_transaction_ack_pdu_tx(env->link_id, pb_adv->Transaction_Number);
            return;
        }else
        {
            LOG(LOG_LVL_INFO,"Missing Transaction!!\n");
            return;
        }
    }

    switch(pdu_type) {
        case Transaction_Start_PDU: {
                uint8_t SegN = pb_adv->data_buf[0]>>2;
                uint16_t TotalLength = (pb_adv->data_buf[1]<<8) | pb_adv->data_buf[2];
                uint8_t FCS = pb_adv->data_buf[3];
                transaction_start_info_t start_pdu_info = 
                {
                    .TotalLength = TotalLength,
                    .SegN = SegN,
                    .FCS = FCS,
                };
                provisioning_transaction_start_pdu_rx(env, &start_pdu_info, pb_adv->Transaction_Number, pb_adv->data_length - 4, pb_adv->data_buf + 4);
            }
            break;
        case Transaction_Ack_PDU:
            provisioning_transaction_ack_pdu_rx(env, pb_adv->Transaction_Number);
            break;
        case Transaction_Continuation_PDU:
            provisioning_transaction_continuation_pdu_rx(env, segment_index, pb_adv->Transaction_Number, pb_adv->data_length - 1, pb_adv->data_buf + 1);
            break;
        case Provisioning_Bearer_Control_PDU:
            provisioning_bearer_control_pdu_rx(env, bearer_code, &pb_adv->data_buf[1], pb_adv->data_length - 1);
            break;
        default:
            LOG(LOG_LVL_ERROR,"Invalid provision pdu type\n");
            break;
    }
}

void provision_pb_adv_rx(uint8_t *data,uint8_t len)
{
    uint32_t Link_ID;
    memcpy(&Link_ID,data,sizeof(uint32_t));
    Link_ID = co_bswap32(Link_ID);
    uint8_t Transaction_Number = data[4];
    uint8_t *data_buf = data + DATA_OFFSET_IN_PB_ADV;
    uint8_t data_length = len - DATA_OFFSET_IN_PB_ADV;
    pb_adv_info_t rx_pb_adv = 
    {
        .Link_ID = Link_ID,
        .Transaction_Number = Transaction_Number,
        .data_buf = data_buf,
        .data_length = data_length,
    };

    deal_provisioning_pdu_rx(&rx_pb_adv);
}

void pb_pdu_adv_tx_start(pb_adv_tx_t *ptr)
{
    bearer_adv_tx_timer_start(&ptr->adv, 1);
}


uint32_t pb_get_adv_tx_link_id(pb_adv_tx_t *ptr)
{
    uint32_t link_id = *(uint32_t *)&ptr->adv.adv_data[DATA_OFFSET_IN_BEARER];
    //LOG(3, "pb_get_adv_tx_link_id:%x\n", *(uint32_t *)&ptr->adv.adv_data[DATA_OFFSET_IN_BEARER]);

    return co_bswap32(link_id);
}


static void pb_adv_tx_msg_send(pb_adv_tx_t *ptr) 
{
    uint32_t link_id = pb_get_adv_tx_link_id(ptr);
    bool is_transaction_ack = provisoning_judge_is_transaction_ack_pdu(ptr);
    //LOG(3, "pb_adv_tx_msg_send\n");
    provision_adv_env_t *env = provision_env_search(link_id);

    if(is_transaction_ack)
        pb_adv_set_ack_tx_state(env, PROVISION_PKT_START);
    else
        pb_adv_set_tx_state(env, PROVISION_PKT_START);

    bearer_adv_tx_msg_send(&ptr->adv , false);

    ptr->param.count++;
}

static void pb_adv_tx_timer_expire_handler(pb_adv_tx_t *ptr)
{
    bool adv_continue = pb_adv_tx_continue(ptr);

    //LOG(3,"pb_adv_tx_timer_expire_handler:%d ptr:%x\n", adv_continue, ptr);
    if(adv_continue)
    {
        pb_adv_tx_msg_send(ptr);
    }else
    {
        pb_adv_tx_buf_free(ptr);
    }
}

void pb_adv_tx_timer_callback(mesh_timer_t xTimer)
{
    //LOG(3,"pb_adv_tx_timer_callback\n");
    mesh_adv_tx_t *adv = pvTimerGetTimerID(xTimer);
    pb_adv_tx_t *ptr = CONTAINER_OF(adv, pb_adv_tx_t, adv);
    mesh_queued_msg_send((void (*)(void *))pb_adv_tx_timer_expire_handler, ptr);
}

static pb_adv_tx_t *pb_adv_tx_buf_alloc(uint8_t pkt_type)
{
    //LOG(3, "pb_adv_tx_buf_alloc\n");
    pb_adv_tx_t *ptr = array_buf_alloc(&pb_adv_tx_buf);
    bearer_adv_tx_timer_create("pb_tx", &ptr->adv, false, pb_adv_tx_timer_callback);
    ptr->adv.pkt_type = pkt_type;
    BX_ASSERT(NULL!=ptr);

    if(pkt_type != GATT_SERVICE_ADV_PKT)
    {
        mesh_core_params_t param_network_transmit;
        mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_NETWORK_TRANSMIT , &param_network_transmit);
        ptr->param.count = 0;
        ptr->param.repeats = param_network_transmit.network_transmit.count;
    }
    else
    {
        mesh_core_params_t param_proxy_services_transmit;
        mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_PROXY_SERVICES_TRANSMIT   , &param_proxy_services_transmit);
        ptr->param.count = 0;
        ptr->param.repeats = param_proxy_services_transmit.proxy_services_transmit.count;
    }
    return ptr;
}

pb_adv_tx_t *pb_adv_pkt_prepare(uint8_t *data,uint8_t length,uint8_t pkt_type)
{
    pb_adv_tx_t *adv_tx = pb_adv_tx_buf_alloc(pkt_type);
    adv_tx->adv.data_length = length;
    memcpy(adv_tx->adv.adv_data,data,length);
    adv_tx->adv.scan_rsp = NULL;
    adv_tx->adv.pkt_type = pkt_type;
    return adv_tx;    
}


static bool pb_adv_tx_continue(pb_adv_tx_t *ptr)
{
    bool adv_continue = false;
    uint32_t link_id = pb_get_adv_tx_link_id(ptr);
    provision_adv_env_t *env = provision_env_search(link_id);
    uint8_t pkt_state;
    bool is_transaction_ack = provisoning_judge_is_transaction_ack_pdu(ptr);

    if(is_transaction_ack) {
        pkt_state = pb_adv_get_ack_tx_state(env);
    }else {
        pkt_state = pb_adv_get_tx_state(env);
    }

    if(pkt_state == PROVISION_PKT_START)
    {
        adv_continue = true;
    }else if(pkt_state == PROVISION_PKT_DELAY_LONGTIME) {
        adv_continue = true;
    }else if(pkt_state >= PROVISION_PKT_CANCEL && pkt_state <= PROVISION_PKT_DONE) {
        adv_continue = false;
    }
    //LOG(3, "pb_adv_tx_continue pkt_state:%x adv_continue:%d is_transaction_ack:%d\n", pkt_state, adv_continue, is_transaction_ack);
    return adv_continue;
}

static void pb_send(pb_adv_tx_t *ptr)
{
    pb_adv_tx_msg_send(ptr);
}

void pb_tx_on_adv_start_callback(mesh_adv_tx_t *adv)
{
    pb_adv_tx_t *ptr = CONTAINER_OF(adv, pb_adv_tx_t, adv);
    bool adv_cnt_expire = ptr->param.count > ptr->param.repeats;
    bool adv_continue = false;
    uint16_t interval_ms = (rand() % (MESH_RAND_DELAY_MAX * 2)) + MESH_RAND_DELAY_MAX;
    uint32_t link_id = pb_get_adv_tx_link_id(ptr);
    provision_adv_env_t *env = provision_env_search(link_id);
    bool is_transaction_ack = provisoning_judge_is_transaction_ack_pdu(ptr);
    uint8_t pkt_state = pb_adv_get_tx_state(env);

    adv_continue = pb_adv_tx_continue(ptr);
    //LOG(3, "pb_tx_on_adv_start_callback %d %d current_state:%d %d linkid 0x%x ptr 0x%x\n", adv_cnt_expire, adv_continue, env->current_state, env->ack_current_state,link_id,ptr);

    if(adv_cnt_expire && adv_continue) {
        //LOG(3, "pb_tx_on_adv_start_callback is_transaction_ack:%d\n", is_transaction_ack);
        if(is_transaction_ack || (env->current_state > ADV_SEND_LINK_OPEN && env->current_state <= ADV_SEND_LINK_CLOSE)) {
            if(is_transaction_ack)
                pb_adv_set_ack_tx_state(env, PROVISION_PKT_COUNT_OVERFLOW);
            else
                pb_adv_set_tx_state(env, PROVISION_PKT_COUNT_OVERFLOW);

            pb_adv_tx_buf_free(ptr);
        }else {
            ptr->param.count = 0;
            bearer_adv_tx_timer_start(adv, interval_ms);
        }
    }else if(adv_continue == false) {
        pb_adv_tx_buf_free(ptr);
    }else {
        bearer_adv_tx_timer_start(adv, interval_ms);
    }
}

