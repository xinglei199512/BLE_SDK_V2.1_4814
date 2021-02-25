#include "friendship.h"
#include "control_tx_process.h"
#include "co_endian.h"
#include "upper_pdu.h"


#define FRIEND_CLEAR_PDU_LENGTH 4


void friend_clear_tx(uint16_t dst_addr,uint8_t ttl,security_credentials_t *net_security,uint16_t lpn_addr,uint16_t lpn_counter,void (*callback)(void *,uint8_t))
{
    control_msg_tx_param_t param= {
        .expected_tx_time = NULL,
        .dst_addr = dst_addr,
        .length = FRIEND_CLEAR_PDU_LENGTH,
        .netkey_credentials = net_security,
        .opcode = FRIEND_CLEAR,
        .ttl = ttl,
        .high_priority = false,
    };
    control_pdu_tx_t *pdu = control_unseg_msg_build(&param,callback);
    BX_ASSERT(pdu);
    friend_clear_payload_t *ptr = (void*)pdu->payload;
    ptr->lpn_addr_be = co_bswap16(lpn_addr);
    ptr->lpn_counter_be = co_bswap16(lpn_counter);
    control_send(pdu);
}

