
#include "tmall_model_common.h"
#include "tmall_model_msg_handler.h"

#if (MESH_MODEL_TMALL_MODEL_SERVER || MESH_MODEL_TMALL_MODEL_CLIENT)
msg_handler_model_t tmall_model_msg[Vendor_Message_MAX]=
{
#if MESH_MODEL_TMALL_MODEL_SERVER
    [Vendor_Message_Transparent_msg] = {tmall_message_transparent_msg_rx, TMALL_MODEL_SERVER_MODEL_ID},
    [Vendor_Message_Attr_Get] = {tmall_message_attr_get_rx, TMALL_MODEL_SERVER_MODEL_ID},
    [Vendor_Message_Attr_Set] = {tmall_message_attr_set_rx, TMALL_MODEL_SERVER_MODEL_ID},
    [Vendor_Message_Attr_Set_Unacknowledged] = {tmall_attr_set_unacknowledged_rx, TMALL_MODEL_SERVER_MODEL_ID},
    [Vendor_Message_Attr_Indication] = {tmall_message_attr_indication_rx, TMALL_MODEL_SERVER_MODEL_ID},
    [Vendor_Message_Attr_Confirmation] = {tmall_message_attr_confirmation_rx, TMALL_MODEL_SERVER_MODEL_ID},
    [Vendor_Message_Attr_Respond_Time] = {tmall_message_attr_respond_time, TMALL_MODEL_SERVER_MODEL_ID},
    [Vendor_Message_Transparernt_Ack] = {tmall_message_transparent_ack, TMALL_MODEL_SERVER_MODEL_ID},
#endif /* MESH_MODEL_TMALL_MODEL_SERVER */
#if MESH_MODEL_TMALL_MODEL_CLIENT
    [Vendor_Message_Attr_Status] = {tmall_message_attr_status_rx, TMALL_MODEL_CLIENT_MODEL_ID},
    [Vendor_Message_Attr_Time] = {tmall_message_attr_time_rx, TMALL_MODEL_CLIENT_MODEL_ID},
		
#endif /* MESH_MODEL_TMALL_MODEL_CLIENT */
};
#endif /* MESH_MODEL_TMALL_MODEL_SERVER || MESH_MODEL_TMALL_MODEL_CLIENT */
