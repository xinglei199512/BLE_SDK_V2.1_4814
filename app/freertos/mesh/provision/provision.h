
#ifndef PROVISION_H_
#define PROVISION_H_
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <stdbool.h>
#include "provision_comm.h"

#define GENERIC_PROVISIONING_PDU_MAX_SIZE                      (BEARER_BUF_SIZE - 5)
#define MAX_DATA_SIZE_TRANSACTION_START_PDU                    (GENERIC_PROVISIONING_PDU_MAX_SIZE - 4)
#define MAX_DATA_SIZE_TRANSACTION_CONTINUATION_PDU             (GENERIC_PROVISIONING_PDU_MAX_SIZE - 1)
#define DATA_OFFSET_IN_PB_ADV                                  5
#define TRANSACTION_ACK_TIMEOUT                                30000
#define ESTABLISH_LINK_TIMEOUT                                 60000
#define RETRANSMISSION_TIMEOUT                                 3000
#define STATE_CHANGE_TIMEOUT                                   (TRANSACTION_ACK_TIMEOUT * 2)
#define MESH_RAND_DELAY_MIN                                    20 // unit:ms
#define MESH_RAND_DELAY_MAX                                    50

typedef void (*adv_callback)(void *tcb, Tx_Reason reason);

enum Generic_Provisioning_PDU_Type{
    Transaction_Start_PDU           = 0,
    Transaction_Ack_PDU             = 1,
    Transaction_Continuation_PDU    = 2,
    Provisioning_Bearer_Control_PDU = 3,
};

enum Bearer_Opcode_Type{
    Link_Open = 0,
    Link_Ack = 1,
    Link_Close = 2,
    Bearer_Opcode_Max,
};

enum ADV_STAGE_STATE {
    ADV_STATE_IDLE,
    ADV_SEND_LINK_OPEN,
    ADV_SEND_LINK_ACK,
    ADV_SEND_LINK_CLOSE,
    ADV_TRANS_SEND_PDU,
    ADV_TRANS_SEND_ACK,
    ADV_TRANS_ALREADY_SENDED_ACK,
    ADV_RECV_LINK_CLOSE,
};

typedef enum {
    PROVISION_PKT_IDLE           = 0,
    PROVISION_PKT_START          = 1,
    PROVISION_PKT_DELAY_LONGTIME = 2,
    PROVISION_PKT_CANCEL         = 3,
    PROVISION_PKT_ACK_TIMEOUT    = 4,
    PROVISION_PKT_COUNT_OVERFLOW = 5,
    PROVISION_PKT_DONE
}provision_tx_state_t;

typedef struct{
    uint16_t TotalLength;
    uint8_t SegN;
    uint8_t FCS;
}transaction_start_info_t;

typedef struct
{
    uint8_t *buf;
    transaction_start_info_t pdu_info;
    uint8_t block_ack;
    uint32_t link_id;
}provision_reassembly_env_t;

typedef struct{
    uint32_t Link_ID;
    uint8_t *data_buf;
    uint8_t data_length;
    uint8_t Transaction_Number;
}pb_adv_info_t;


typedef struct
{
    uint8_t pkt_state;
    uint8_t ack_pkt_state;
    uint8_t current_state;
    uint8_t ack_current_state;
    uint32_t link_id;
    struct {
        uint8_t peer;
        uint8_t local;
    }transaction_num;
    mesh_timer_t ack_Timer;
    mesh_timer_t establish_Timer;
    adv_callback adv_cb;
}provision_adv_env_t;

provision_adv_env_t* provision_env_search(uint32_t link_id);
provision_adv_env_t* provisioning_tx_pdu_buf_alloc(bool is_server);
void provisioning_tx_pdu_buf_release(provision_adv_env_t *env);
uint8_t provisioning_pdu_FCS_gen(uint8_t *pdu,uint8_t length);
void provision_pb_adv_rx(uint8_t *data,uint8_t len);
void provisioning_link_ack_tx(provision_adv_env_t *env);
void provisioning_link_open_tx(provision_adv_env_t *env, uint8_t *dev_uuid, adv_callback cb);
void provisioning_link_close_tx(provision_adv_env_t *env, uint8_t reason, adv_callback cb);
void pb_tx_on_adv_start_callback(mesh_adv_tx_t *adv);
#endif /* PROVISION_H_ */
