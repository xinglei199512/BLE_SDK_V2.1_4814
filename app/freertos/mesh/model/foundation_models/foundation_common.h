#ifndef FOUNDATION_COMMON_H_
#define FOUNDATION_COMMON_H_
#include "access_rx_process.h"

enum Foundation_Models_Status_Code
{
    Config_Success = 0,
    Invalid_Address,
    Invalid_Model,
    Invalid_AppKey_Index,
    Invalid_NetKey_Index,
    Insufficient_Resources,
    Key_Index_Already_Stored,
    Invalid_Publish_Parameters,
    Not_a_Subscribe_Model,
    Storage_Failure,
    Feature_Not_Supported,
    Cannot_Update,
    Cannot_Remove,
    Cannot_Bind,
    Temporarily_Unable_to_Change_State,
    Cannot_Set,
    Unspecified_Error,
    Invalid_Binding,
    
};

void fd_model_two_octets_status_tx(uint16_t netkey_idx,uint16_t dst_addr,ble_txrx_time_t *rx_time,void (*cb)(void *,uint8_t),
    uint8_t *payload,uint8_t length,uint16_t opcode_offset);
    
void fd_model_two_octets_status_response_tx(access_pdu_rx_t *rx_pdu,void (*cb)(void *,uint8_t),
    uint8_t *payload,uint8_t length,uint16_t opcode_offset);

    
#endif


