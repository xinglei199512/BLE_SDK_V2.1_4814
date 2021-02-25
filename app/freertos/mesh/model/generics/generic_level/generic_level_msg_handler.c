
#include "generic_level_common.h"

#if (MESH_MODEL_GENERIC_LEVEL_SERVER || MESH_MODEL_GENERIC_LEVEL_CLIENT)
msg_handler_model_t generic_level_model_msg[Generic_Level_Opcode_Max]=
{
#if MESH_MODEL_GENERIC_LEVEL_SERVER
    [Generic_Level_Get] = {generic_level_get_rx, GENERIC_LEVEL_SERVER_MODEL_ID},
    [Generic_Level_Set] = {generic_level_set_rx, GENERIC_LEVEL_SERVER_MODEL_ID},
    [Generic_Level_Set_Unacknowledged] = {generic_level_set_unacknowledged_rx, GENERIC_LEVEL_SERVER_MODEL_ID},
    [Generic_Delta_Set] = {generic_delta_set_rx, GENERIC_LEVEL_SERVER_MODEL_ID},
    [Generic_Delta_Set_Unacknowledged] = {generic_delta_set_unacknowledged_rx, GENERIC_LEVEL_SERVER_MODEL_ID},
    [Generic_Move_Set] = {generic_move_set_rx, GENERIC_LEVEL_SERVER_MODEL_ID},
    [Generic_Move_Set_Unacknowledged] = {generic_move_set_unacknowledged_rx, GENERIC_LEVEL_SERVER_MODEL_ID},
#endif /*MESH_MODEL_GENERIC_LEVEL_SERVER */
#if MESH_MODEL_GENERIC_LEVEL_CLIENT
    [Generic_Level_Status] = {generic_level_status_rx, GENERIC_LEVEL_CLIENT_MODEL_ID},
#endif /* MESH_MODEL_GENERIC_LEVEL_CLIENT */
};
#endif /* MESH_MODEL_GENERIC_LEVEL_SERVER || MESH_MODEL_GENERIC_LEVEL_CLIENT */
