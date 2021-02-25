#ifndef TC_MODEL_MSG_HANDLER__H
#define TC_MODEL_MSG_HANDLER__H
#include "tc_model_client.h"
#include "tc_model_server.h"
#include "access_rx_process.h"
#include "model_msg.h"

#define TC_MODEL_SERVER_MODEL_ID 0x0000a2b2
#define TC_MODEL_CLIENT_MODEL_ID 0x0001a2b2
#define TC_MODEL_OPCODE_COMPANY_ID 0xa2b2
#define TC_MODEL_OPCODE_COMPANY_ID_SHIFT 16
#define TC_MODEL_OPCODE_OFFSET 0x1f
enum TC_Massage_Opcode
{
    TC_Message_Transparent_msg,
    TC_Message_Attr_Get,
    TC_Message_Attr_Set,
    TC_Message_Attr_Set_Unacknowledged,
    TC_Message_Attr_Status,
    TC_Message_Attr_Get_Value,
    TC_Message_Attr_Indication,
    TC_Message_Attr_Confirmation,
    TC_Message_MAX,
};

extern msg_handler_model_t tc_model_msg[TC_Message_MAX];
#endif /* TC_MODEL_MSG_HANDLER__H */
