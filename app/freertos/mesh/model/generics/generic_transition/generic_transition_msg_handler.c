
#include "generic_transition_common.h"
#include "generic_transition_msg_handler.h"

#if (MESH_MODEL_GENERIC_TRANSITION_SERVER || MESH_MODEL_GENERIC_TRANSITION_CLIENT)
msg_handler_model_t generic_default_transition_time_model_msg[Generic_Default_Transition_Time_Opcode_Max]=
{
#if MESH_MODEL_GENERIC_TRANSITION_SERVER
    [Generic_Default_Transition_Time_Get] = {generic_default_transition_time_get_rx, GENERIC_DEFAULT_TRANSITION_TIME_SERVER_MODEL_ID},
    [Generic_Default_Transition_Time_Set] = {generic_default_transition_time_set_rx, GENERIC_DEFAULT_TRANSITION_TIME_SERVER_MODEL_ID},
    [Generic_Default_Transition_Time_Set_Unacknowledged] = {generic_default_transition_time_set_unacknowledged_rx, GENERIC_DEFAULT_TRANSITION_TIME_SERVER_MODEL_ID},
#endif /* MESH_MODEL_GENERIC_TRANSITION_SERVER */
#if MESH_MODEL_GENERIC_TRANSITION_CLIENT
    [Generic_Default_Transition_Time_Status] = {generic_default_transition_time_status_rx, GENERIC_DEFAULT_TRANSTION_TIME_CLIENT_MODEL_ID},
#endif /* MESH_MODEL_GENERIC_TRANSITION_CLIENT */
};
#endif /* MESH_MODEL_GENERIC_TRANSITION_SERVER || MESH_MODEL_GENERIC_TRANSITION_CLIENT */
