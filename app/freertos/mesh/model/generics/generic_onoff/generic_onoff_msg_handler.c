#include "generic_onoff_msg_handler.h"

#if (MESH_MODEL_GENERIC_ONOFF_SERVER || MESH_MODEL_GENERIC_ONOFF_CLIENT)
msg_handler_model_t generic_onoff_model_msg[Generic_OnOff_Opcode_Max]=
{
    #if (MESH_MODEL_GENERIC_ONOFF_SERVER)
    [Generic_OnOff_Get]                     = {generic_onoff_get_rx,                GENERIC_ONOFF_SERVER_MODEL_ID},
    [Generic_OnOff_Set]                     = {generic_onoff_set_rx,                GENERIC_ONOFF_SERVER_MODEL_ID},
    [Generic_OnOff_Set_Unacknowledged]      = {generic_onoff_set_unacknowledged_rx, GENERIC_ONOFF_SERVER_MODEL_ID},
    #endif /* MESH_MODEL_GENERIC_ONOFF_SERVER */
    #if (MESH_MODEL_GENERIC_ONOFF_CLIENT)
    [Generic_OnOff_Status]                  = {generic_onoff_status_rx,             GENERIC_ONOFF_CLIENT_MODEL_ID},
    #endif /*MESH_MODEL_GENERIC_ONOFF_CLIENT */
};
#endif /* MESH_MODEL_GENERIC_ONOFF_SERVER || MESH_MODEL_GENERIC_ONOFF_CLIENT */
