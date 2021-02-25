
#include "generic_power_onoff_common.h"
#include "generic_power_onoff_msg_handler.h"

#if (MESH_MODEL_GENERIC_POWER_ONOFF_SERVER || MESH_MODEL_GENERIC_POWER_ONOFF_SETUP_SERVER || MESH_MODEL_GENERIC_POWER_ONOFF_CLIENT)
msg_handler_model_t generic_power_onoff_model_msg[Generic_OnPowerUp_Opcode_Max]=
{
#if MESH_MODEL_GENERIC_POWER_ONOFF_SERVER
    [Generic_OnPowerUp_Get] = {generic_onpowerup_get_rx, GENERIC_POWER_ONOFF_SERVER_MODEL_ID},
    [Generic_OnPowerUp_Set] = {generic_onpowerup_set_rx, GENERIC_POWER_ONOFF_SETUP_SERVER_MODEL_ID},
#endif /* MESH_MODEL_GENERIC_POWER_ONOFF_SERVER */
#if MESH_MODEL_GENERIC_POWER_ONOFF_SETUP_SERVER
    [Generic_OnPowerUp_Set_Unacknowledged] = {generic_onpowerup_set_unacknowledged_rx, GENERIC_POWER_ONOFF_SETUP_SERVER_MODEL_ID},
#endif /* MESH_MODEL_GENERIC_POWER_ONOFF_SETUP_SERVER */
#if MESH_MODEL_GENERIC_POWER_ONOFF_CLIENT
    [Generic_OnPowerUp_Status] = {generic_onpowerup_status_rx, GENERIC_POWER_CLIENT_SERVER_MODEL_ID},
#endif /* MESH_MODEL_GENERIC_POWER_ONOFF_CLIENT */
};
#endif /* MESH_MODEL_GENERIC_POWER_ONOFF_SERVER || MESH_MODEL_GENERIC_POWER_ONOFF_SETUP_SERVER || MESH_MODEL_GENERIC_POWER_ONOFF_CLIENT */
