#ifndef TMALL_MODEL_MSG_HANDLER__H
#define TMALL_MODEL_MSG_HANDLER__H
#include "tmall_model_client.h"
#include "tmall_model_server.h"
#include "access_rx_process.h"
#include "model_msg.h"

#define TMALL_MODEL_SERVER_MODEL_ID 0x000001a8
#define TMALL_MODEL_CLIENT_MODEL_ID 0x000101a8
#define TMALL_MODEL_OPCODE_COMPANY_ID 0x01A8
#define TMALL_MODEL_OPCODE_COMPANY_ID_1 0xa801
#define TMALL_MODEL_OPCODE_COMPANY_ID_SHIFT 16
#define TMALL_MODEL_OPCODE_OFFSET 0x0d
enum Vendor_Massage_Opcode
{	  
	  Vendor_Message_Transparernt_Ack,       
	  Vendor_Message_Transparernt_Indication,
    Vendor_Message_Transparent_msg,
    Vendor_Message_Attr_Get,
    Vendor_Message_Attr_Set,
    Vendor_Message_Attr_Set_Unacknowledged,
    Vendor_Message_Attr_Status,
    Vendor_Message_Attr_Indication,
    Vendor_Message_Attr_Confirmation,
    Vendor_Message_Attr_Time=0xf,          //0xDE
	  Vendor_Message_Attr_Respond_Time=0x10 ,//0xDF
    Vendor_Message_MAX,
};

extern msg_handler_model_t tmall_model_msg[Vendor_Message_MAX];
#endif /* TMALL_MODEL_MSG_HANDLER__H */
