
#include "tc_model_common.h"
#include "tc_model_msg_handler.h"

#if (MESH_MODEL_TC_MODEL_SERVER || MESH_MODEL_TC_MODEL_CLIENT)
msg_handler_model_t tc_model_msg[TC_Message_MAX]=
{
#if MESH_MODEL_TC_MODEL_SERVER
    [TC_Message_Transparent_msg] = {tc_message_transparent_msg_rx, TC_MODEL_SERVER_MODEL_ID},
    [TC_Message_Attr_Get] = {tc_message_attr_get_rx, TC_MODEL_SERVER_MODEL_ID},
    [TC_Message_Attr_Set] = {tc_message_attr_set_rx, TC_MODEL_SERVER_MODEL_ID},
    [TC_Message_Attr_Set_Unacknowledged] = {tc_attr_set_unacknowledged_rx, TC_MODEL_SERVER_MODEL_ID},
    [TC_Message_Attr_Indication] = {tc_message_attr_indication_rx, TC_MODEL_SERVER_MODEL_ID},
    [TC_Message_Attr_Confirmation] = {tc_message_attr_confirmation_rx, TC_MODEL_SERVER_MODEL_ID},
#endif /* MESH_MODEL_TC_MODEL_SERVER */
#if MESH_MODEL_TC_MODEL_CLIENT
    [TC_Message_Attr_Status] = {tc_message_attr_status_rx, TC_MODEL_CLIENT_MODEL_ID},
    [TC_Message_Attr_Get_Value]={tc_message_attr_get_value,TC_MODEL_CLIENT_MODEL_ID},
#endif /* MESH_MODEL_TC_MODEL_CLIENT */
};
#endif /* MESH_MODEL_TC_MODEL_SERVER || MESH_MODEL_TC_MODEL_CLIENT */
