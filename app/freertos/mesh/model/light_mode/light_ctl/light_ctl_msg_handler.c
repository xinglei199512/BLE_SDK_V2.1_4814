
#include "light_ctl_common.h"
#include "light_ctl_temperature_server.h"
#include "light_ctl_setup_server.h"
#include "light_ctl_msg_handler.h"

#if (MESH_MODEL_LIGHT_CTL_SERVER || MESH_MODEL_LIGHT_CTL_TEMPERATURE_SERVER || MESH_MODEL_LIGHT_CTL_SETUP_SERVER || MESH_MODEL_LIGHT_CTL_CLIENT)
msg_handler_model_t light_CTL_model_msg[Light_CTL_Opcode_MAX]=
{
#if MESH_MODEL_LIGHT_CTL_SERVER
    [Light_CTL_Get] = {light_CTL_get_rx, LIGHT_CTL_SERVER_MODEL_ID},
    [Light_CTL_Set] = {light_CTL_set_rx, LIGHT_CTL_SERVER_MODEL_ID},
    [Light_CTL_Set_Unacknowledged] = {light_CTL_set_unacknowledged_rx, LIGHT_CTL_SERVER_MODEL_ID},

    [Light_CTL_Temperature_Range_Get] = {light_CTL_temperature_range_get_rx, LIGHT_CTL_SERVER_MODEL_ID},

    [Light_CTL_Default_Get] = {light_CTL_Default_get_rx, LIGHT_CTL_SERVER_MODEL_ID},
#endif /*MESH_MODEL_LIGHT_CTL_SERVER */
#if MESH_MODEL_LIGHT_CTL_TEMPERATURE_SERVER
    [Light_CTL_Temperature_Get] = {light_CTL_temperature_get_rx, LIGHT_CTL_TEMPERATURE_SERVER_MODEL_ID},
    [Light_CTL_Temperature_Set] = {light_CTL_temperature_set_rx, LIGHT_CTL_TEMPERATURE_SERVER_MODEL_ID},
    [Light_CTL_Temperature_Set_Unacknowledged] = {light_CTL_temperature_set_unacknowledged_rx, LIGHT_CTL_TEMPERATURE_SERVER_MODEL_ID},
#endif /* MESH_MODEL_LIGHT_CTL_TEMPERATURE_SERVER */
#if MESH_MODEL_LIGHT_CTL_SETUP_SERVER
    [Light_CTL_Temperature_Range_Set] = {light_CTL_temperature_range_set_rx, LIGHT_CTL_SETUP_SERVER_MODEL_ID},
    [Light_CTL_Temperature_Range_Set_Unacknowledged] = {light_CTL_temperature_range_set_unacknowledged_rx, LIGHT_CTL_SETUP_SERVER_MODEL_ID},
    [Light_CTL_Default_Set] = {light_CTL_default_set_rx, LIGHT_CTL_SETUP_SERVER_MODEL_ID},
    [Light_CTL_Default_Set_Unacknowledged] = {light_CTL_default_set_unacknowledged_rx, LIGHT_CTL_SETUP_SERVER_MODEL_ID},
#endif /*MESH_MODEL_LIGHT_CTL_SETUP_SERVER */
#if MESH_MODEL_LIGHT_CTL_CLIENT
    [Light_CTL_Temperature_Range_Status] = {light_CTL_temperature_range_status_rx, LIGHT_CTL_CLIENT_MODEL_ID},
    [Light_CTL_Status] = {light_CTL_status_rx, LIGHT_CTL_CLIENT_MODEL_ID},
    [Light_CTL_Temperature_Status] = {light_CTL_temperature_status_rx, LIGHT_CTL_CLIENT_MODEL_ID},
    [Light_CTL_Default_Status] = {light_CTL_Default_status_rx, LIGHT_CTL_CLIENT_MODEL_ID},
#endif /* MESH_MODEL_LIGHT_CTL_CLIENT */
};
#endif /*MESH_MODEL_LIGHT_CTL_SERVER || MESH_MODEL_LIGHT_CTL_TEMPERATURE_SERVER || MESH_MODEL_LIGHT_CTL_SETUP_SERVER || MESH_MODEL_LIGHT_CTL_CLIENT */
